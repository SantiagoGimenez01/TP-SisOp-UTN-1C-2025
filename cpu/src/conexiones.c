#include "sockets.h"
#include "utils/libs/logger.h"
#include "utils/libs/config.h"
#include "utils/structs.h"
#include "conexiones.h"
#include "globales.h"


void comprobarSocket(int socket, char* modulo){
    if(socket == -1){
        log_error(logger, "No se pudo conectar a %s", modulo);
        exit(EXIT_FAILURE);
    }
}

void establecerConexiones(int id_cpu) {
    char* puerto_memoria = string_itoa(configCPU.puerto_memoria);
    char* puerto_dispatch = string_itoa(configCPU.puerto_kernel_dispatch);
    char* puerto_interrupt = string_itoa(configCPU.puerto_kernel_interrupt);

    // Conexion persistente a MEMORIA
    socket_memoria = crearConexion(configCPU.ip_memoria, puerto_memoria, logger);
    comprobarSocket(socket_memoria, "MEMORIA");
    log_info(logger, "Conectado a MEMORIA.");
    enviar_handshake(socket_memoria, MODULO_CPU_DISPATCH);
    send(socket_memoria, &id_cpu, sizeof(int), 0);
    log_info(logger, "ID de CPU enviado a Memoria: %d", id_cpu);

    // Conexion persistente a KERNEL - DISPATCH
    socket_dispatch = crearConexion(configCPU.ip_kernel, puerto_dispatch, logger);
    comprobarSocket(socket_dispatch, "KERNEL DISPATCH");
    log_info(logger, "Conectado a KERNEL DISPATCH.");
    
    send(socket_dispatch, &id_cpu, sizeof(int), 0);
    enviar_handshake(socket_dispatch, MODULO_CPU_DISPATCH);

    // Conexion persistente a KERNEL - INTERRUPT
    socket_interrupt = crearConexion(configCPU.ip_kernel, puerto_interrupt, logger);
    comprobarSocket(socket_interrupt, "KERNEL INTERRUPT");
    log_info(logger, "Conectado a KERNEL INTERRUPT.");
    send(socket_interrupt, &id_cpu, sizeof(int), 0);
    enviar_handshake(socket_interrupt, MODULO_CPU_INTERRUPT);

    free(puerto_memoria);
    free(puerto_dispatch);
    free(puerto_interrupt);
}

void escucharOperaciones() {
    pthread_t hilo_dispatch, hilo_interrupt;

    pthread_create(&hilo_dispatch, NULL, escuchar_dispatch, NULL);
    pthread_detach(hilo_dispatch);

    pthread_create(&hilo_interrupt, NULL, escuchar_interrupt, NULL);
    pthread_detach(hilo_interrupt);
}

void* escuchar_dispatch(void* arg) {
    while (1) {
        t_opcode codigo_operacion;
        int bytes = recv(socket_dispatch, &codigo_operacion, sizeof(t_opcode), MSG_WAITALL);

        if (bytes <= 0) {
            log_error(logger, "Error al recibir mensaje por DISPATCH o desconexion");
            break;
        }

        switch (codigo_operacion) {
            case EJECUTAR_PROCESO:
                log_info(logger, "Se recibio EJECUTAR_PROCESO por DISPATCH");

                uint32_t pid = 0;
                uint32_t pc = 0;
                double estimacion = 0;

                recibir_pcb(socket_dispatch, &pid, &pc, &estimacion);

                log_info(logger, "Ejecutando proceso PID=%d desde PC=%d", pid, pc);

                ejecutar_ciclo(pid, pc); 

                break;

            default:
                log_warning(logger, "Código de operacion no reconocido en DISPATCH: %d", codigo_operacion);
                break;
        }
    }

    return NULL;
}

void* escuchar_interrupt(void* arg) {
    while (1) {
        t_opcode codigo_operacion;
        int bytes = recv(socket_interrupt, &codigo_operacion, sizeof(t_opcode), MSG_WAITALL);

        if (bytes <= 0) {
            log_error(logger, "Error al recibir mensaje por INTERRUPT o desconexion");
            break;
        }

        switch (codigo_operacion) {
            case INTERRUPCION:
                log_info(logger, "Se recibio una INTERRUPCION desde Kernel");

                // ciclo de ejecución que debe detenerse
                // interrumpir_proceso(); //

                break;

            default:
                log_warning(logger, "Codigo de operacion no reconocido en INTERRUPT: %d", codigo_operacion);
                break;
        }
    }

    return NULL;
}

void recibir_pcb(int socket_dispatch, uint32_t* pid, uint32_t* pc, double* estimacion) {
    t_paquete* paquete = recibir_paquete(socket_dispatch);

    int offset = 0;
    memcpy(pid, paquete->buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(pc, paquete->buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(estimacion, paquete->buffer->stream + offset, sizeof(double));

    eliminar_paquete(paquete);

    log_info(logger, "Recibido PCB: PID=%d, PC=%d, Estimacion=%.2f", *pid, *pc, *estimacion);
}
