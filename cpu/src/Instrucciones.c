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
            log_info(logger, "CPU (PID %d): READ en direccion %d, tamaño %d", pid, direccion, tamanio);
            // TODO
            break;
        }

        case WRITE: {
            int direccion = atoi(inst->parametros[0]);
            char* datos = inst->parametros[1];
            log_info(logger, "CPU (PID %d): WRITE en direccion %d, datos: %s", pid, direccion, datos);
            // TODO
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

            return enviar_syscall_a_kernel(inst, pid, *pc);
            break;

        default:
            log_warning(logger, "CPU (PID %d): Instruccion invalida o no implementada", pid);
            return false;
    }

    return true;
}

bool enviar_syscall_a_kernel(t_instruccion* inst, uint32_t pid, uint32_t pc) {
    log_info(logger, "CPU (PID %d): Deteniendo ejecucion por SYSCALL %s", pid, nombre_syscall(inst->id));


    enviar_opcode(SYSCALL, socket_dispatch);
    t_paquete* paquete = crear_paquete();
    agregar_int_a_paquete(paquete, inst->id);
    if(inst->id == IO){
        int tiempo = atoi(inst->parametros[1]);
        char* nombre_io = inst->parametros[0];
        agregar_int_a_paquete(paquete, tiempo);
        agregar_string_a_paquete(paquete, nombre_io);
    }
    if(inst->id == INIT_PROC){
        char* nombre_archivo = inst->parametros[0];
        int tamanio = atoi(inst->parametros[1]);
        agregar_string_a_paquete(paquete, nombre_archivo);
        agregar_int_a_paquete(paquete, tamanio);
    }
    agregar_int_a_paquete(paquete, pid);
    agregar_int_a_paquete(paquete, pc); //ESTOY EN DUDAS POR AHORA
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
    } else if (respuesta == DESALOJAR_PROCESO) {
        log_info(logger, "CPU (PID %d): Kernel indico DESALOJO", pid);
        return false; // Termina ejecutar_ciclo()
    } else {
        log_warning(logger, "CPU (PID %d): Respuesta inesperada de Kernel: %d", pid, respuesta);
        return false;
    }
}



void liberar_instruccion(t_instruccion* inst) {
    for (int i = 0; i < inst->cantidad_parametros; i++) {
        free(inst->parametros[i]);
    }
    free(inst->parametros);
    free(inst);
}