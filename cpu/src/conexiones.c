#include "sockets.h"
#include "utils/libs/logger.h"
#include "utils/libs/config.h"
#include "utils/structs.h"
#include "conexiones.h"
#include "globales.h"
// Sockets globales
int socket_memoria;
int socket_dispatch;
int socket_interrupt;

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

