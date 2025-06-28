#include "peticiones.h"

char* pedir_contenido_de_pagina(uint32_t pid, uint32_t marco) {
    enviar_opcode(PEDIR_PAGINA_COMPLETA, socket_memoria);
    t_paquete* paquete = crear_paquete();
    agregar_int_a_paquete(paquete, pid);
    agregar_int_a_paquete(paquete, marco);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    t_paquete* respuesta = recibir_paquete(socket_memoria);
    char* contenido = recibir_bloque_de_paquete(respuesta, tam_pagina);
    eliminar_paquete(respuesta);
    return contenido;
}

void escribir_en_memoria(uint32_t pid, t_direccion_fisica* dir, uint32_t marco, char* nuevo_contenido) {
    enviar_opcode(ESCRIBIR_PAGINA, socket_memoria);
    t_paquete* paquete = crear_paquete();

    agregar_int_a_paquete(paquete, pid);
    agregar_int_a_paquete(paquete, dir->numero_pagina);
    agregar_int_a_paquete(paquete, marco);

    for (int i = 0; i < cantidad_niveles; i++) {
        agregar_int_a_paquete(paquete, dir->entradas_niveles[i]);
    }

    agregar_int_a_paquete(paquete, dir->desplazamiento);
    agregar_string_a_paquete(paquete, nuevo_contenido);
        log_info(logger, "Escribiendo: pid=%d, nro_pagina=%d, marco=%d, desplazamiento=%d, contenido=%s",
         pid, dir->numero_pagina, marco, dir->desplazamiento, nuevo_contenido);

        for (int i = 0; i < cantidad_niveles; i++) {
            log_info(logger, "Entrada nivel[%d] = %d", i, dir->entradas_niveles[i]);
        }

    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    int respuesta_ok;
    recv(socket_memoria, &respuesta_ok, sizeof(int), MSG_WAITALL);
    if (respuesta_ok != RESPUESTA_OK) {
        log_error(logger, "Error en respuesta de escritura a memoria");
        
    }
}


uint32_t pedir_marco_a_memoria(int pid, t_direccion_fisica* dir) {
    enviar_opcode(PEDIR_MARCO, socket_memoria);

    t_paquete* paquete = crear_paquete();
    agregar_int_a_paquete(paquete, pid);

    for (int i = 0; i < cantidad_niveles; i++) {
        agregar_int_a_paquete(paquete, dir->entradas_niveles[i]);
    }

    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    uint32_t marco;
    recv(socket_memoria, &marco, sizeof(int), MSG_WAITALL);
    return marco;
}

void actualizar_paginas_modificadas_en_memoria(uint32_t pid) {
    for (int i = 0; i < list_size(cache_paginas); i++) {
        t_entrada_cache* entrada = list_get(cache_paginas, i);
        if (entrada->modificado) {
            enviar_opcode(ACTUALIZAR_PAGINA_COMPLETA, socket_memoria);
            t_paquete* paquete = crear_paquete();
            agregar_int_a_paquete(paquete, pid);
            agregar_int_a_paquete(paquete, entrada->marco);
            log_info(logger, "DEBUG CPU - Mandando al marco %d el contenido '%s'", entrada->marco, entrada->contenido);

            agregar_bloque_a_paquete(paquete, entrada->contenido, tam_pagina);
            log_info(logger, "DEBUG CPU - Mandando al marco %d el contenido (parcial): '%.20s'", entrada->marco, entrada->contenido);
            enviar_paquete(paquete, socket_memoria);
            eliminar_paquete(paquete);

            int respuesta_ok;
            recv(socket_memoria, &respuesta_ok, sizeof(int), MSG_WAITALL);
            if (respuesta_ok != RESPUESTA_OK) {
                log_error(logger, "Fallo al actualizar pagina en memoria: PID=%d, Marco=%d", pid, entrada->marco);
            }


            log_info(logger, "PID: %d - Pagina Actualizada de Cache a Memoria - Página: %d - Frame: %d", pid, entrada->nro_pagina, entrada->marco);
        }
    }
    
}

char* pedir_fragmento_de_memoria(int pid, int marco, int desplazamiento, int tamanio) {
    enviar_opcode(LEER_PAGINA, socket_memoria);

    t_paquete* paquete = crear_paquete();
    agregar_int_a_paquete(paquete, pid);
    agregar_int_a_paquete(paquete, marco);
    agregar_int_a_paquete(paquete, desplazamiento);
    agregar_int_a_paquete(paquete, tamanio);

    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    t_paquete* recibido = recibir_paquete(socket_memoria);
    char* resultado = recibir_string_de_paquete(recibido);
    eliminar_paquete(recibido);
    return resultado;
}

char* pedir_instruccion_a_memoria(uint32_t pid, uint32_t pc) {
    // Enviar pedido
    enviar_opcode(PEDIR_INSTRUCCION, socket_memoria);
    t_paquete* paquete = crear_paquete();
    agregar_int_a_paquete(paquete, pid);
    agregar_int_a_paquete(paquete, pc);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    // Esperar respuesta
    t_paquete* respuesta = recibir_paquete(socket_memoria);
    char* instruccion = recibir_string_de_paquete(respuesta);
    eliminar_paquete(respuesta);

    return instruccion; 
}

bool enviar_syscall_a_kernel(t_instruccion* inst, uint32_t pid, uint32_t pc) {
    log_info(logger, "CPU (PID %d): Deteniendo ejecucion por SYSCALL %s", pid, nombre_syscall(inst->id));


    enviar_opcode(SYSCALL, socket_dispatch);
    t_paquete* paquete = crear_paquete();
    agregar_int_a_paquete(paquete, inst->id);
    agregar_int_a_paquete(paquete, pid);
    agregar_int_a_paquete(paquete, pc); //ESTOY EN DUDAS POR AHORA
    if(inst->id == IO){
        int tiempo = atoi(inst->parametros[1]);
        char* nombre_io = inst->parametros[0];
        //log_info(logger, "Es una SYSCALL IO, nombre del dispositivo: %s", nombre_io);
        agregar_string_a_paquete(paquete, nombre_io);
        agregar_int_a_paquete(paquete, tiempo);
    }
    if(inst->id == INIT_PROC){
        char* nombre_archivo = inst->parametros[0];
        int tamanio = atoi(inst->parametros[1]);
        agregar_string_a_paquete(paquete, nombre_archivo);
        log_info(logger, "Se agrego el nombre archivo: %s  y el tamanio %d al paquete", nombre_archivo, tamanio);
        agregar_int_a_paquete(paquete, tamanio);
    }
    
    enviar_paquete(paquete, socket_dispatch);
    eliminar_paquete(paquete);
    
    t_opcode respuesta;
    int bytes = recv(socket_dispatch, &respuesta, sizeof(t_opcode), MSG_WAITALL);

    if (bytes <= 0) {
        log_error(logger, "Error al recibir respuesta de Kernel en SYSCALL");
        exit(EXIT_FAILURE);
    }

    if (respuesta == CONTINUAR_PROCESO) {
        log_info(logger, "CPU (PID %d): Kernel indico CONTINUAR", pid);
        return true;  // Sigue ejecutando
    }else if(respuesta == DESALOJAR_PROCESO) {
        log_info(logger, "CPU (PID %d): Kernel indico DESALOJO", pid);
    
        if (cache_paginas != NULL) {
            limpiar_cache();
        }

        if (tlb != NULL) {
            limpiar_tlb();
        }       
        return false; // Terminar ciclo()
    }
     else {
        log_warning(logger, "CPU (PID %d): Respuesta inesperada de Kernel: %d", pid, respuesta);
        return false;
    }
}


void limpiar_tlb() {
    list_destroy_and_destroy_elements(tlb, free);
    tlb = list_create();
    log_info(logger, "TLB limpiada");
}

void limpiar_cache() {
    for (int i = 0; i < list_size(cache_paginas); i++) {
    t_entrada_cache* entrada = list_get(cache_paginas, i);
    if (entrada->contenido != NULL) {
        free(entrada->contenido);
        entrada->contenido = NULL;
    }
}

    list_destroy_and_destroy_elements(cache_paginas, free);
    cache_paginas = list_create();
    log_info(logger, "Cache de páginas limpiada");
}