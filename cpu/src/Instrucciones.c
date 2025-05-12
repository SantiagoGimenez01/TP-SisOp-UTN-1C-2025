#include "Instrucciones.h"

void ejecutar_ciclo(uint32_t pid, uint32_t pc) {
    bool ejecutando = true;

    while (ejecutando) {
        // Fase 1: FETCH
        char* instruccion = pedir_instruccion_a_memoria(pid, pc);
        log_info(logger, "PID %d - PC %d - Instruccion recibida: %s", pid, pc, instruccion);

        // Fase 2: DECODE
        t_instruccion* inst = decode_instruccion(instruccion);        
        // Fase 3: EXECUTE
        ejecutando = ejecutar_instruccion(inst, pid, &pc);

        // Avanzar el PC solo si no fue modificado por una instruccion, OJO CON ESTO
        if (inst->id != GOTO && inst->id != EXIT) {
            pc++;
        }
        liberar_instruccion(inst);
        free(instruccion);  
    }
}

t_instruccion_id obtener_id_instruccion(char* nombre) {
    if (strcmp(nombre, "NOOP") == 0) return NOOP;
    if (strcmp(nombre, "READ") == 0) return READ;
    if (strcmp(nombre, "WRITE") == 0) return WRITE;
    if (strcmp(nombre, "GOTO") == 0) return GOTO;
    if (strcmp(nombre, "IO") == 0) return IO;
    if (strcmp(nombre, "INIT_PROC") == 0) return INIT_PROC;
    if (strcmp(nombre, "DUMP_MEMORY") == 0) return DUMP_MEMORY;
    if (strcmp(nombre, "EXIT") == 0) return EXIT;

    return -1;
}

t_instruccion* decode_instruccion(char* linea) {
    t_instruccion* inst = malloc(sizeof(t_instruccion));
    inst->parametros = NULL;
    inst->cantidad_parametros = 0;

    char* linea_copia = strdup(linea);  // strtok modifica el string
    char* token = strtok(linea_copia, " \n");

    if (!token) {
        free(linea_copia);
        free(inst);
        return NULL;
    }

    inst->id = obtener_id_instruccion(token);

    char** parametros = NULL;
    int cantidad = 0;

    while ((token = strtok(NULL, " \n")) != NULL) {
        parametros = realloc(parametros, sizeof(char*) * (cantidad + 1));
        parametros[cantidad] = strdup(token);
        cantidad++;
    }

    inst->parametros = parametros;
    inst->cantidad_parametros = cantidad;

    free(linea_copia);
    return inst;
}

bool ejecutar_instruccion(t_instruccion* inst, uint32_t pid, uint32_t* pc) {
    switch (inst->id) {
        case NOOP:
            log_info(logger, "CPU (PID %d): Ejecutando NOOP", pid);
            break;

        case READ: {
            int direccion = atoi(inst->parametros[0]);
            int tamanio = atoi(inst->parametros[1]);
            log_info(logger, "CPU (PID %d): READ en direccion %d, tamanio %d", pid, direccion, tamanio);

            uint32_t nro_pagina = direccion / tam_pagina;
            uint32_t desplazamiento = direccion % tam_pagina;
            t_direccion_fisica* dir = traducir_direccion_logica(direccion);
            char* contenido = NULL;

            // 1 Cache (si esta habilitada)
            if (configCPU.entradas_cache > 0) {
                t_entrada_cache* entrada_cache = buscar_en_cache(nro_pagina);
                if (entrada_cache) {
                    log_info(logger, "PID: %d - Cache Hit - Pagina: %d", pid, nro_pagina);
                    contenido = extraer_fragmento_con_desplazamiento(entrada_cache->contenido, desplazamiento, tamanio);
                    log_info(logger, "Contenido leido desde Cache: %s", contenido);
                    free(contenido);
                    break;
                } else {
                    log_info(logger, "PID: %d - Cache Miss - Pagina: %d", pid, nro_pagina);
                }
            }

            // 2 TLB (si esta habilitada)
            int marco = -1;
            if (configCPU.entradas_tlb > 0) {
                t_entrada_tlb* entrada_tlb = buscar_en_tlb(nro_pagina);
                if (entrada_tlb) {
                    log_info(logger, "PID: %d - TLB HIT - Pagina: %d", pid, nro_pagina);
                    marco = entrada_tlb->marco;
                } else {
                    log_info(logger, "PID: %d - TLB MISS - Pagina: %d", pid, nro_pagina);
                }
            }

            //3 MMU (solo si no encontro en la TLB)
            if (marco == -1) {
                 
                marco = pedir_marco_a_memoria(pid, dir); 
                log_info(logger, "PID: %d - OBTENER MARCO - Pagina: %d - Marco: %d", pid, nro_pagina, marco);
                if(tlb != NULL){
                    agregar_a_tlb(nro_pagina, marco);
                }
                
            }

            if (configCPU.entradas_cache > 0) {
                // Cache habilitada, traigo pagina entera
                char* pagina_completa = pedir_contenido_de_pagina(pid, marco);
                contenido = extraer_fragmento_con_desplazamiento(pagina_completa, desplazamiento, tamanio);
                log_info(logger, "Contenido leido desde Memoria: %s", contenido);
                agregar_a_cache(pid, dir, marco, pagina_completa);
                log_info(logger, "PID: %d - Cache Add - Pagina: %d", pid, nro_pagina);
                free(pagina_completa);
            } else {
                // Cache deshabilitada, pido solo el fragmento
                contenido = pedir_fragmento_de_memoria(pid, marco, desplazamiento, tamanio);
                log_info(logger, "Contenido leido desde Memoria: %s", contenido);
            }
            free(contenido);
            free(dir->entradas_niveles);
            free(dir);
            break;
        }

        case WRITE: {
            int direccion = atoi(inst->parametros[0]);
            char* datos = inst->parametros[1];
            log_info(logger, "CPU (PID %d): WRITE en direccion %d, datos: %s", pid, direccion, datos);

            uint32_t nro_pagina = direccion / tam_pagina;
            uint32_t desplazamiento = direccion % tam_pagina;
            t_direccion_fisica* dir = traducir_direccion_logica(direccion);;
            //1 Si la cache esta habilitada
            if (configCPU.entradas_cache > 0) {
                t_entrada_cache* entrada_cache = buscar_en_cache(nro_pagina);
                if (entrada_cache) {
                    log_info(logger, "PID: %d - Cache Hit - Pagina: %d", pid, nro_pagina);
                    escribir_en_cache(nro_pagina, desplazamiento, datos);
                    entrada_cache->modificado = true;
                    break;
                } else {
                    log_info(logger, "PID: %d - Cache Miss - Pagina: %d", pid, nro_pagina);
                }
            }

            // 2 Buscar en TLB (si esta habilitada)
            int marco = -1;
            if (configCPU.entradas_tlb > 0) {
                t_entrada_tlb* entrada_tlb = buscar_en_tlb(nro_pagina);
                if (entrada_tlb) {
                    log_info(logger, "PID: %d - TLB HIT - Pagina: %d", pid, nro_pagina);
                    marco = entrada_tlb->marco;
                } else {
                    log_info(logger, "PID: %d - TLB MISS - Pagina: %d", pid, nro_pagina);
                }
            }

            // 3 Si no estaba en la TLB, usar MMU para obtener el marco
            if (marco == -1) {
                
                marco = pedir_marco_a_memoria(pid, dir);
                log_info(logger, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, nro_pagina, marco);
                if(tlb != NULL){
                    agregar_a_tlb(nro_pagina, marco);
                }
                
            }

            // 4 Si cache esta habilitada, traer pagina completa y modificar
            if (configCPU.entradas_cache > 0) {
                char* pagina_completa = pedir_contenido_de_pagina(pid, marco);
                escribir_fragmento_en_pagina(pagina_completa, desplazamiento, datos);
                agregar_a_cache(pid, dir, marco, pagina_completa);  
                marcar_modificada_en_cache(nro_pagina);
                log_info(logger, "PID: %d - Cache Add - Pagina: %d", pid, nro_pagina);
                free(pagina_completa);
            } else {
                //5. Cache deshabilitada, escribir directamente en Memoria
                escribir_en_memoria(pid, dir, marco, datos);
                log_info(logger, "PID: %d - WRITE directo a Memoria", pid);
            }
            free(dir->entradas_niveles);
            free(dir);
            break;
        }
        case GOTO: {
            *pc = atoi(inst->parametros[0]);
            log_info(logger, "CPU (PID %d): GOTO a PC %d", pid, *pc);
            break;
        }

        case IO:        
        case INIT_PROC: 
        case DUMP_MEMORY:
        case EXIT:

            return enviar_syscall_a_kernel(inst, pid, *pc); // TENGO QUE PROBAR ESTO PARA QUE SUME en caso de que se haya desalojado, HAY QUE PROBAR
            break;

        default:
            log_warning(logger, "CPU (PID %d): Instruccion invalida o no implementada", pid);
            return false;
    }

    return true;
}


void liberar_instruccion(t_instruccion* inst) {
    for (int i = 0; i < inst->cantidad_parametros; i++) {
        free(inst->parametros[i]);
    }
    free(inst->parametros);
    free(inst);
}
