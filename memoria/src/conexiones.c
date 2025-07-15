#include "sockets.h"
#include "utils/libs/logger.h"
#include "utils/libs/config.h"
#include "utils/structs.h"
#include "conexiones.h"
#include "utils/paquete.c"
#include <math.h>
#include <pthread.h>

void *atender_cliente(void *socket_cliente_void)
{
    int socket_cliente = (intptr_t)socket_cliente_void;

    t_modulo modulo_origen;
    recv(socket_cliente, &modulo_origen, sizeof(t_modulo), 0);

    if (modulo_origen == MODULO_KERNEL)
    {
        log_info(logger, "## Kernel Conectado - FD del socket: %i", socket_cliente);

        operarKernel(socket_cliente); // Atendemos la operacion de Kernel
        close(socket_cliente);        // Kernel = conexión efimera (DUDAS, pero hay que probar)
    }
    else if (modulo_origen == MODULO_CPU_DISPATCH)
    {
        int id_cpu;
        recv(socket_cliente, &id_cpu, sizeof(int), 0);
        log_debug(logger, "Se conecto una CPU. ID: %d", id_cpu);

        operarCPU(socket_cliente);
    }
    else
    {
        log_warning(logger, "Se conecto un modulo desconocido.");
        close(socket_cliente);
    }

    return NULL;
}

void establecerConexiones()
{
    char* puerto_string = string_itoa(configMEMORIA.puerto_escucha);
    int socket_servidor = iniciarServidor(puerto_string, logger, "MEMORIA");
    free(puerto_string);

    log_debug(logger, "Servidor MEMORIA escuchando conexiones.");

    while (1)
    {
        int socket_cliente = esperarCliente(socket_servidor, logger);

        pthread_t hilo_cliente;
        pthread_create(&hilo_cliente, NULL, atender_cliente, (void *)(intptr_t)socket_cliente);
        pthread_detach(hilo_cliente);
    }
}

void operarKernel(int socket_cliente)
{
    log_debug(logger, "Manejando operacion del Kernel...");

    t_opcode codigo_operacion;
    int recibidos = recv(socket_cliente, &codigo_operacion, sizeof(t_opcode), MSG_WAITALL);

    if (recibidos <= 0)
    {
        log_error(logger, "Error al recibir codigo de operacion desde Kernel.");
        return;
    }

    switch (codigo_operacion)
    {

    case INICIAR_PROCESO:
        log_debug(logger, "Kernel solicita INICIAR_PROCESO");
        // Recibir paquete con PID, tamaño y archivo
        t_paquete *paquete = recibir_paquete(socket_cliente);

        int offset = 0;
        int pid;
        memcpy(&pid, paquete->buffer->stream + offset, sizeof(int));
        offset += sizeof(int);

        int tamanio;
        memcpy(&tamanio, paquete->buffer->stream + offset, sizeof(int));
        offset += sizeof(int);

        uint32_t tam_string;
        memcpy(&tam_string, paquete->buffer->stream + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        char *archivo = malloc(tam_string);
        memcpy(archivo, paquete->buffer->stream + offset, tam_string);

        log_debug(logger, "Datos recibidos del Kernel: PID=%d, TAM=%d, ARCHIVO=%s", pid, tamanio, archivo);

        uint32_t paginas_necesarias = (uint32_t)ceil((double)tamanio / configMEMORIA.tam_pagina);
        log_debug(logger, "El proceso %d requiere %d páginas", pid, paginas_necesarias);

        if (hay_espacio_para(paginas_necesarias))
        {
            crear_estructuras_para_proceso(pid, archivo, tamanio, paginas_necesarias); // crea la jerarquica sin asignar marcos por ahora

            int respuesta_ok = RESPUESTA_OK;
            send(socket_cliente, &respuesta_ok, sizeof(int), 0);
            log_info(logger, "## PID: %i - Proceso Creado - Tamaño: %i", pid, tamanio);
            log_debug(logger, "Proceso %d aceptado. Estructuras creadas en memoria.", pid);
        }
        else
        {
            int respuesta_error = RESPUESTA_ERROR;
            send(socket_cliente, &respuesta_error, sizeof(int), 0);
            log_warning(logger, "No hay espacio suficiente para el proceso PID %d", pid);
        }

        free(archivo);
        eliminar_paquete(paquete);
        break;

    case DUMP_MEMORY:
    {
        log_debug(logger, "Se recibio solicitud de DUMP_MEMORY");
        log_info(logger, "## PID: %i - Memory Dump solicitado", pid);
        t_paquete *paquete = recibir_paquete(socket_cliente);
        int pid;
        memcpy(&pid, paquete->buffer->stream, sizeof(int));
        eliminar_paquete(paquete);

        t_proceso_en_memoria *proceso = buscar_proceso_por_pid(pid);
        if (!proceso)
        {
            log_warning(logger, "PID %d no encontrado para DUMP", pid);
            int error = RESPUESTA_ERROR;
            send(socket_cliente, &error, sizeof(int), 0);
            break;
        }

        log_info(logger, "## PID: %d - Inicio de Dump", pid);
        bool ok = generar_dump(proceso);
        // CREAR ARCHIVO y aca verificar si estuvo okey o hubo error

        int respuesta = ok ? RESPUESTA_OK : RESPUESTA_ERROR;
        send(socket_cliente, &respuesta, sizeof(int), 0);
        break;
    }
    case SUSPENDER_PROCESO:
    {
        log_debug(logger, "Se recibio solicitud de SUSPENDER_PROCESO");

        t_paquete *paquete = recibir_paquete(socket_cliente);
        int pid;
        memcpy(&pid, paquete->buffer->stream, sizeof(int));
        eliminar_paquete(paquete);

        t_proceso_en_memoria *proceso = buscar_proceso_por_pid(pid);
        if (!proceso)
        {
            log_warning(logger, "PID %d no encontrado para suspender", pid);
            int error = RESPUESTA_ERROR;
            send(socket_cliente, &error, sizeof(int), 0);
            break;
        } // No deberia pasar

        suspender_proceso(proceso);

        log_trace(logger, "Estado del bitmap de frames tras suspender PID %d:", pid);
        for (int i = 0; i < cantidad_frames; i++) {
            log_trace(logger, "Frame %d: %d", i, bitarray_test_bit(bitmap_frames, i));
        }

        int ok = RESPUESTA_OK;
        send(socket_cliente, &ok, sizeof(int), 0);
        log_debug(logger, "Memoria: PID %d suspendido correctamente", pid);
        break;
    }

    case DESUSPENDER_PROCESO:
    {
        log_debug(logger, "Se recibio solicitud de DESUSPENDER_PROCESO");

        t_paquete *paquete = recibir_paquete(socket_cliente);
        int pid;
        memcpy(&pid, paquete->buffer->stream, sizeof(int));
        eliminar_paquete(paquete);

        t_proceso_en_memoria *proceso = buscar_proceso_por_pid(pid);
        if (!proceso)
        {
            log_warning(logger, "PID %d no encontrado para desuspender", pid);
            int error = RESPUESTA_ERROR;
            send(socket_cliente, &error, sizeof(int), 0);
            break;
        }

        int resultado = desuspender_proceso(proceso);

        if (resultado == -1)
        {
            int error = RESPUESTA_ERROR;
            send(socket_cliente, &error, sizeof(int), 0);
            log_warning(logger, "Memoria: no hay espacio suficiente para desuspender PID %d", pid);
        }
        else
        {
            int ok = RESPUESTA_OK;
            send(socket_cliente, &ok, sizeof(int), 0);
            log_debug(logger, "Memoria: PID %d desuspendido correctamente", pid);
        }

        break;
    }

    case FINALIZAR_PROCESO:
    {
        log_debug(logger, "Se recibio solicitud de FINALIZAR_PROCESO");

        t_paquete *paquete = recibir_paquete(socket_cliente);
        int pid;
        memcpy(&pid, paquete->buffer->stream, sizeof(int));
        eliminar_paquete(paquete);

        t_proceso_en_memoria *proceso = buscar_proceso_por_pid(pid);
        if (!proceso)
        {
            log_warning(logger, "PID %d no encontrado para finalizar", pid);
            int error = RESPUESTA_ERROR;
            send(socket_cliente, &error, sizeof(int), 0);
            break;
        }

        log_metricas_proceso(proceso);
        liberar_proceso_en_memoria(proceso);

        int ok = RESPUESTA_OK;
        send(socket_cliente, &ok, sizeof(int), 0);
        log_debug(logger, "Memoria: PID %d finalizado correctamente", pid);

        int ocupados = 0;
        for (int i = 0; i < cantidad_frames; i++) {
            if (bitarray_test_bit(bitmap_frames, i))
                ocupados++;
        }
        log_debug(logger, "Cantidad de marcos ocupados tras liberar: %d", ocupados);
        break;
    }

    default:
        log_warning(logger, "Operacion desconocida del Kernel: %d", codigo_operacion);
        break;
    }
}

void operarCPU(int socket_cliente)
{
    log_debug(logger, "Manejando operacion de CPU con Memoria...");

    while (1)
    {
        t_opcode opcode;
        int status = recv(socket_cliente, &opcode, sizeof(t_opcode), MSG_WAITALL);
        if (status <= 0)
        {
            log_warning(logger, "Se cerro la conexion con CPU.");
            close(socket_cliente);
            return;
        }

        switch (opcode)
        {
        case PEDIR_CONFIGURACION:
        {
            log_debug(logger, "CPU solicita configuracion de memoria");

            t_paquete *paquete = crear_paquete();
            agregar_int_a_paquete(paquete, configMEMORIA.tam_pagina);
            agregar_int_a_paquete(paquete, configMEMORIA.entradas_por_tabla);
            agregar_int_a_paquete(paquete, configMEMORIA.cantidad_niveles);

            enviar_paquete(paquete, socket_cliente);
            eliminar_paquete(paquete);

            break;
        }

        case PEDIR_INSTRUCCION:
        {
            log_debug(logger, "Se recibio PEDIR_INSTRUCCION desde CPU");
            t_paquete *paquete = recibir_paquete(socket_cliente);
            int offset = 0;
            int pid;
            memcpy(&pid, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);

            int pc;
            memcpy(&pc, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);

            eliminar_paquete(paquete);

            t_proceso_en_memoria *proceso = buscar_proceso_por_pid(pid);

            if (proceso == NULL)
            {
                log_error(logger, "PID %d no encontrado en memoria", pid);
                // No deberia pasar
                return;
            }

            char *instruccion = list_get(proceso->instrucciones, pc);
            proceso->metricas.instrucciones_solicitadas++;
            if (instruccion == NULL)
            {
                log_warning(logger, "PID %d - Instruccion no encontrada en PC %d", pid, pc);
                // VEREMOS QUE SUCEDE
                return;
            }
            log_info(logger, "## PID: %i- Obtener instrucción: %i - Instrucción: %s <...ARGS>", pid, pc, instruccion);
            log_debug(logger, "PID %d - PC %d - Enviando instruccion: %s", pid, pc, instruccion);
            t_paquete *respuesta = crear_paquete();
            agregar_string_a_paquete(respuesta, instruccion);
            enviar_paquete(respuesta, socket_cliente);
            eliminar_paquete(respuesta);
            break;
        }
        case PEDIR_MARCO:
        {
            t_paquete *paquete = recibir_paquete(socket_cliente);
            int offset = 0;

            int pid;
            memcpy(&pid, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);

            int *entradas_niveles = malloc(sizeof(int) * configMEMORIA.cantidad_niveles);
            for (int i = 0; i < configMEMORIA.cantidad_niveles; i++)
            {
                memcpy(&entradas_niveles[i], paquete->buffer->stream + offset, sizeof(int));
                offset += sizeof(int);
            }

            t_proceso_en_memoria *proceso = buscar_proceso_por_pid(pid);

            uint32_t marco = obtener_marco(proceso, entradas_niveles);

            send(socket_cliente, &marco, sizeof(int), 0);

            free(entradas_niveles);
            eliminar_paquete(paquete);
            break;
        }

        case LEER_PAGINA:
        {
            t_paquete *paquete = recibir_paquete(socket_cliente);
            int offset = 0;

            int pid, marco, desplazamiento, tamanio;
            memcpy(&pid, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);
            memcpy(&marco, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);
            memcpy(&desplazamiento, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);
            memcpy(&tamanio, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);

            t_proceso_en_memoria *proceso = buscar_proceso_por_pid(pid);
            char *contenido = leer_fragmento(proceso, marco, desplazamiento, tamanio);

            log_info(logger, "## PID: %i - Lectura - Dir. Física: (Marco: %i | Offset: %i) - Tamaño: %i", pid, marco, desplazamiento, tamanio);

            t_paquete *respuesta = crear_paquete();
            agregar_string_a_paquete(respuesta, contenido);
            enviar_paquete(respuesta, socket_cliente);
            eliminar_paquete(respuesta);
            free(contenido);
            eliminar_paquete(paquete);
            break;
        }

        case PEDIR_PAGINA_COMPLETA:
        {
            t_paquete *paquete = recibir_paquete(socket_cliente);
            int pid, marco;
            int offset = 0;

            memcpy(&pid, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);
            memcpy(&marco, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);

            t_proceso_en_memoria *proceso = buscar_proceso_por_pid(pid);
            char *pagina_completa = leer_pagina(proceso, marco);
            log_info(logger, "## PID: %i - Escritura - Dir. Física: (Marco: %i | Offset: ?) - Tamaño: %i", pid, marco, configMEMORIA.tam_pagina);

            t_paquete *respuesta = crear_paquete();
            agregar_bloque_a_paquete(respuesta, pagina_completa, configMEMORIA.tam_pagina);
            enviar_paquete(respuesta, socket_cliente);
            eliminar_paquete(respuesta);
            free(pagina_completa);
            eliminar_paquete(paquete);
            break;
        }

        case ESCRIBIR_PAGINA:
        {
            t_paquete *paquete = recibir_paquete(socket_cliente);
            int offset = 0;

            int pid, nro_pagina, marco, desplazamiento;

            memcpy(&pid, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);
            memcpy(&nro_pagina, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);
            memcpy(&marco, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);

            int *entradas_niveles = malloc(sizeof(int) * configMEMORIA.cantidad_niveles);
            for (int i = 0; i < configMEMORIA.cantidad_niveles; i++)
            {
                memcpy(&entradas_niveles[i], paquete->buffer->stream + offset, sizeof(int));
                offset += sizeof(int);
            }

            memcpy(&desplazamiento, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);

            char *datos = recibir_string_de_paquete_con_offset(paquete, &offset);
            log_debug(logger, "Recibido: pid=%d, nro_pagina=%d, marco=%d, desplazamiento=%d, datos=%s",
                     pid, nro_pagina, marco, desplazamiento, datos);

            for (int i = 0; i < configMEMORIA.cantidad_niveles; i++)
            {
                log_debug(logger, "Entrada recibida nivel[%d] = %d", i, entradas_niveles[i]);
            }

            t_proceso_en_memoria *proceso = buscar_proceso_por_pid(pid);

            escribir_en_pagina(proceso, marco, desplazamiento, datos);

            marcar_modificada(proceso, entradas_niveles);
            log_debug(logger, "## PID: %i - Escritura - Dir. Física: (Marco: %i | Offset: %i) - Tamaño: ????", pid, marco, desplazamiento);

            int ok = RESPUESTA_OK;
            send(socket_cliente, &ok, sizeof(int), 0);

            free(entradas_niveles);
            free(datos);
            eliminar_paquete(paquete);
            break;
        }
        case ACTUALIZAR_PAGINA_COMPLETA:
        {
            t_paquete *paquete = recibir_paquete(socket_cliente);
            int offset = 0;
            int pid, marco;
            memcpy(&pid, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);
            memcpy(&marco, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);

            char *contenido = recibir_string_de_paquete_con_offset(paquete, &offset);

            t_proceso_en_memoria *proceso = buscar_proceso_por_pid(pid);

            char *destino = memoria_fisica + marco * configMEMORIA.tam_pagina;
            memcpy(destino, contenido, configMEMORIA.tam_pagina);
            log_debug(logger, "Memoria: Página actualizada desde CPU - PID: %d - Marco: %d - Contenido: %s",
                     pid, marco, contenido);
            log_info(logger, "## PID: %i - Escritura - Dir. Física: (Marco: %i | Offset: ?) - Tamaño: %i", pid, marco, configMEMORIA.tam_pagina);

            proceso->metricas.escrituras_memoria++;

            int respuesta = RESPUESTA_OK;
            send(socket_cliente, &respuesta, sizeof(int), 0);

            free(contenido);
            eliminar_paquete(paquete);
            break;
        }

        default:
            log_warning(logger, "Operacion desconocida desde CPU: %d", opcode);
            break;
        }
    }
}
