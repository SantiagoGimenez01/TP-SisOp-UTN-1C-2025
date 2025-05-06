#include "syscalls.h"

void procesar_syscall(t_paquete* paquete, int socket_cpu) {
    int offset = 0;
    t_instruccion_id syscall_id;
    memcpy(&syscall_id, paquete->buffer->stream + offset, sizeof(int));
    offset += sizeof(int);

    uint32_t pid;
    uint32_t pc;
    memcpy(&pid, paquete->buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&pc, paquete->buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    t_pcb* pcb = buscar_pcb_por_pid(pid);
    pcb->pc = pc;

    log_info(logger, "Proceso %d devuelto a Kernel desde socket CPU %d. Motivo: %s", pcb->pid, socket_cpu, nombre_syscall(syscall_id));

    switch (syscall_id) {
        case IO: {
            char* nombre_io = recibir_string_de_paquete_con_offset(paquete, &offset);
            int tiempo;
            memcpy(&tiempo, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);

            log_info(logger, "SYSCALL IO: Proceso %d requiere IO %s por %d ms", pid, nombre_io, tiempo);

            atender_syscall_io(pcb, nombre_io, tiempo, socket_cpu);

            free(nombre_io);
            break;
        }

        case INIT_PROC: {
            char* nombre_archivo = recibir_string_de_paquete_con_offset(paquete, &offset);
            int tamanio;
            memcpy(&tamanio, paquete->buffer->stream + offset, sizeof(int));
            offset += sizeof(int);

            log_info(logger, "SYSCALL INIT_PROC: Crear proceso con archivo %s y tamaño %d", nombre_archivo, tamanio);

            atender_syscall_init_proc(pcb, nombre_archivo, tamanio, socket_cpu);

            free(nombre_archivo);
            break;
        }

        case DUMP_MEMORY: {
            log_info(logger, "SYSCALL DUMP_MEMORY: PID %d solicita dump de memoria", pid);

            atender_syscall_dump_memory(pcb, socket_cpu);
            break;
        }

        case EXIT: {
            log_info(logger, "SYSCALL EXIT: Proceso %d finalizando.", pid);

            atender_syscall_exit(pcb, socket_cpu);
            break;
        }

        default:
            log_error(logger, "Syscall desconocida recibida: %d", syscall_id);
            break;
    }
}

void atender_syscall_io(t_pcb* pcb, char* nombre_io, int tiempo, int socket_cpu) {
    t_io* dispositivo = buscar_io_por_nombre(nombre_io);

    if (dispositivo == NULL) {
        enviar_opcode(DESALOJAR_PROCESO, socket_cpu);
        log_warning(logger, "Proceso %d solicita IO '%s' inexistente. Terminando proceso.", pcb->pid, nombre_io);
        cambiar_estado(pcb, EXIT);
        finalizar_proceso(pcb); 
        return;
    }
    log_info(logger, "El nombre del dispositivo que se esta atendiendo es %s", dispositivo->nombre);
    cambiar_estado(pcb, BLOCKED);
    log_info(logger, "El proceso %d ahora esta bloqueado por %d segundos", pcb->pid, tiempo);
    enviar_opcode(DESALOJAR_PROCESO, socket_cpu);
    log_info(logger, "El proceso %d se desalojo", pcb->pid);
    usar_o_encolar_io(dispositivo, pcb, tiempo);
}


void atender_syscall_init_proc(t_pcb* pcb, char* archivo, int tamanio, int socket_cpu) {
    log_info(logger, "Proceso %d solicita INIT_PROC para archivo %s, tamaño %d.", pcb->pid, archivo, tamanio);
    
    inicializar_proceso(archivo, tamanio); 
    enviar_opcode(CONTINUAR_PROCESO, socket_cpu);
}


void atender_syscall_dump_memory(t_pcb* pcb, int socket_cpu) {
    cambiar_estado(pcb, BLOCKED);
    enviar_opcode(DESALOJAR_PROCESO, socket_cpu);

    if (solicitar_dump_a_memoria(pcb->pid)) {
        log_info(logger, "Dump de Memoria exitoso para PID %d. Volviendo a READY.", pcb->pid);
        cambiar_estado(pcb, READY);
        sem_post(&sem_procesos_en_ready);
    } else {
        log_warning(logger, "Dump de Memoria fallido para PID %d. Finalizando proceso.", pcb->pid);
        cambiar_estado(pcb, EXIT_PROCESS);
        finalizar_proceso(pcb);
    }
}


void atender_syscall_exit(t_pcb* pcb, int socket_cpu) {

    cambiar_estado(pcb, EXIT);
    log_info(logger, "Se envia al socket %d el desalojo", socket_cpu);
    enviar_opcode(DESALOJAR_PROCESO, socket_cpu);
    finalizar_proceso(pcb);
}

t_io* buscar_io_por_nombre(char* nombre_io) {
    for (int i = 0; i < list_size(ios); i++) {
        t_io* dispositivo = list_get(ios, i);
        if (strcmp(dispositivo->nombre, nombre_io) == 0) {
            return dispositivo;
        }
    }
    return NULL;
}

t_io* buscar_io_por_socket(int socket) {
    for (int i = 0; i < list_size(ios); i++) {
        t_io* dispositivo = list_get(ios, i);
        if (dispositivo->socket == socket) {
            return dispositivo;
        }
    }
    return NULL;
}


void usar_o_encolar_io(t_io* dispositivo, t_pcb* pcb, int tiempo) {
    
    if (dispositivo->disponible) {

        dispositivo->disponible = 0;
        dispositivo->pid_actual = pcb->pid;
        
        log_info(logger, "Proceso %d usando IO %s inmediatamente.", pcb->pid, dispositivo->nombre);

        // Enviar al modulo IO el PID y el tiempo
        enviar_opcode(SOLICITUD_IO, dispositivo->socket);
        t_paquete* paquete = crear_paquete();
        agregar_int_a_paquete(paquete, pcb->pid);
        agregar_int_a_paquete(paquete, tiempo);  
        enviar_paquete(paquete, dispositivo->socket);
        eliminar_paquete(paquete);

    } else {
        pcb->tiempoIO = tiempo;
        queue_push(dispositivo->cola_procesos, pcb);
        log_info(logger, "Proceso %d encolado esperando IO %s.", pcb->pid, dispositivo->nombre);
    }
}

void finalizar_proceso(t_pcb* pcb) {
    log_info(logger, "Finalizando proceso %d...", pcb->pid);

    if (!liberar_en_memoria(pcb->pid)) {
        log_error(logger, "Error liberando memoria para PID %d", pcb->pid);
    }

    loguear_metricas_estado(pcb);
    
    remover_pcb(pcb);

    sem_post(&sem_procesos_en_new); // OJO CON ESTO, NO ME TENGO QUE OLVIDAR DE QUE PRIMERO VAN LOS SUSPREADY


}


void loguear_metricas_estado(t_pcb* pcb) {
    log_info(logger, "Metricas del proceso %d:", pcb->pid);
    
    for (int i = 0; i < list_size(pcb->metricas); i++) {
        t_metricas_estado* metrica = list_get(pcb->metricas, i);
        log_info(logger, "Estado: %s | Veces: %d | Tiempo total: %" PRIu64 " ms",
                 nombre_estado(metrica->estado),
                 metrica->cantVeces,
                 metrica->tiempoTotal);
    }
}

void remover_pcb(t_pcb* pcb) {
    pthread_mutex_lock(&mutex_pcbs);
    list_remove_element(pcbs, pcb);
    pthread_mutex_unlock(&mutex_pcbs);

    list_destroy_and_destroy_elements(pcb->metricas, free);
    free(pcb->archivo_pseudocodigo);
    log_info(logger, "Proceso %d finalizado y recursos liberados.", pcb->pid);
    free(pcb);
}
