#include "sockets.h"
#include "utils/libs/logger.h"
#include "utils/libs/config.h"
#include "utils/structs.h"
#include "conexiones.h"
#include "utils/paquete.c"
#include <math.h>
#include <pthread.h>



void* atender_cliente(void* socket_cliente_void) {
    int socket_cliente = (intptr_t)socket_cliente_void;

    t_modulo modulo_origen;
    recv(socket_cliente, &modulo_origen, sizeof(t_modulo), 0);

    if (modulo_origen == MODULO_KERNEL) {
        log_info(logger, "Se conecto el Kernel (conexion efimera).");
        
        operarKernel(socket_cliente);// Atendemos la operacion de Kernel
        close(socket_cliente); // Kernel = conexión efimera (DUDAS, pero hay que probar)
    }
    else if (modulo_origen == MODULO_CPU_DISPATCH) {
        int id_cpu;
        recv(socket_cliente, &id_cpu, sizeof(int), 0);
        log_info(logger, "Se conecto una CPU. ID: %d", id_cpu);

        operarCPU(socket_cliente);
        
    }
    else {
        log_warning(logger, "Se conecto un modulo desconocido.");
        close(socket_cliente);
    }

    return NULL;
}

void establecerConexiones() {
    int socket_servidor = iniciarServidor(string_itoa(configMEMORIA.puerto_escucha), logger, "MEMORIA");

    log_info(logger, "Servidor MEMORIA escuchando conexiones.");

    while (1) {
        int socket_cliente = esperarCliente(socket_servidor, logger);

        pthread_t hilo_cliente;
        pthread_create(&hilo_cliente, NULL, atender_cliente, (void*)(intptr_t)socket_cliente);
        pthread_detach(hilo_cliente); 
    }
}


void operarKernel(int socket_cliente) {
    log_info(logger, "Manejando operacion del Kernel...");

    t_opcode codigo_operacion;
    int recibidos = recv(socket_cliente, &codigo_operacion, sizeof(t_opcode), MSG_WAITALL);

    if (recibidos <= 0) {
        log_error(logger, "Error al recibir codigo de operacion desde Kernel.");
        return;
    }

    switch (codigo_operacion) {
        case INICIAR_PROCESO:
            log_info(logger, "Kernel solicita INICIAR_PROCESO");

            // Recibir paquete con PID, tamaño y archivo
            t_paquete* paquete = recibir_paquete(socket_cliente);

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

            char* archivo = malloc(tam_string);
            memcpy(archivo, paquete->buffer->stream + offset, tam_string);

            log_info(logger, "Datos recibidos del Kernel: PID=%d, TAM=%d, ARCHIVO=%s", pid, tamanio, archivo);

            uint32_t paginas_necesarias = (uint32_t) ceil((double)tamanio / configMEMORIA.tam_pagina);
            log_info(logger, "El proceso %d requiere %d páginas", pid, paginas_necesarias);

            if (hay_espacio_para(paginas_necesarias)) {
                crear_estructuras_para_proceso(pid, archivo, tamanio);  // crea la jerarquica sin asignar marcos por ahora

                int respuesta_ok = RESPUESTA_OK;
                send(socket_cliente, &respuesta_ok, sizeof(int), 0);
                log_info(logger, "Proceso %d aceptado. Estructuras creadas en memoria.", pid);

            } else {
                int respuesta_error = RESPUESTA_ERROR;
                send(socket_cliente, &respuesta_error, sizeof(int), 0);
                log_warning(logger, "No hay espacio suficiente para el proceso PID %d", pid);
            }

            free(archivo);
            eliminar_paquete(paquete);
            break;

        default:
            log_warning(logger, "Operacion desconocida del Kernel: %d", codigo_operacion);
            break;
    }
}


void operarCPU(int socket_cliente) {
    log_info(logger, "Manejando operacion de CPU con Memoria...");

    while (1) {
        t_opcode opcode;
        int status = recv(socket_cliente, &opcode, sizeof(t_opcode), MSG_WAITALL);
        if (status <= 0) {
            log_warning(logger, "Se cerro la conexion con CPU.");
            close(socket_cliente);
            return;
        }

        switch (opcode) {
            case PEDIR_INSTRUCCION:
                log_info(logger, "Se recibio PEDIR_INSTRUCCION desde CPU");
                t_paquete* paquete = recibir_paquete(socket_cliente);
                int offset = 0;
                int pid;
                memcpy(&pid, paquete->buffer->stream + offset, sizeof(int));
                offset += sizeof(int);

                int pc;
                memcpy(&pc, paquete->buffer->stream + offset, sizeof(int));
                offset += sizeof(int);

                eliminar_paquete(paquete);

                t_proceso_en_memoria* proceso = buscar_proceso_por_pid(pid);

                if (proceso == NULL) {
                    log_error(logger, "PID %d no encontrado en memoria", pid);
                    // No deberia pasar
                    return;
                }

                char* instruccion = list_get(proceso->instrucciones, pc);

                if (instruccion == NULL) {
                    log_warning(logger, "PID %d - Instruccion no encontrada en PC %d", pid, pc);
                    // VEREMOS QUE SUCEDE
                    return;
                }

                log_info(logger, "PID %d - PC %d - Enviando instruccion: %s", pid, pc, instruccion);
                t_paquete* respuesta = crear_paquete();
                agregar_string_a_paquete(respuesta, instruccion);
                enviar_paquete(respuesta, socket_cliente);
                eliminar_paquete(respuesta);


                break;

            case LEER_PAGINA:
                
                break;

            case ESCRIBIR_PAGINA:
                
                break;

            default:
                log_warning(logger, "Operacion desconocida desde CPU: %d", opcode);
                break;
        }
    }
}



bool hay_espacio_para(uint32_t paginas_requeridas) {
    int disponibles = 0;

    for (int i = 0; i < bitarray_get_max_bit(bitmap_frames); i++) {
        if (!bitarray_test_bit(bitmap_frames, i))
            disponibles++;
    }

    return disponibles >= paginas_requeridas;
}


t_proceso_en_memoria* buscar_proceso_por_pid(uint32_t pid) {
    for (int i = 0; i < list_size(procesos_en_memoria); i++) {
        t_proceso_en_memoria* p = list_get(procesos_en_memoria, i);
        if (p->pid == pid)
            return p;
    }
    return NULL;
}
