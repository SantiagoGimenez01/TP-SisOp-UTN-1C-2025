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

void establecerConexiones() {
    char* puerto_memoria = string_itoa(configCPU.puerto_memoria);
    char* puerto_dispatch = string_itoa(configCPU.puerto_kernel_dispatch);
    char* puerto_interrupt = string_itoa(configCPU.puerto_kernel_interrupt);

    // Conexion persistente a MEMORIA
    socket_memoria = crearConexion(configCPU.ip_memoria, puerto_memoria, logger);
    if (socket_memoria == -1) {
        log_error(logger, "No se pudo conectar a MEMORIA.");
        exit(EXIT_FAILURE);
    }
    log_info(logger, "Conectado a MEMORIA.");
    enviar_handshake(socket_memoria, MODULO_CPU_DISPATCH);

    // Conexion persistente a KERNEL - DISPATCH
    socket_dispatch = crearConexion(configCPU.ip_kernel, puerto_dispatch, logger);
    if (socket_dispatch == -1) {
        log_error(logger, "No se pudo conectar a KERNEL DISPATCH.");
        exit(EXIT_FAILURE);
    }
    log_info(logger, "Conectado a KERNEL DISPATCH.");
    enviar_handshake(socket_dispatch, MODULO_CPU_DISPATCH);

    // Conexion persistente a KERNEL - INTERRUPT
    socket_interrupt = crearConexion(configCPU.ip_kernel, puerto_interrupt, logger);
    if (socket_interrupt == -1) {
        log_error(logger, "No se pudo conectar a KERNEL INTERRUPT.");
        exit(EXIT_FAILURE);
    }
    log_info(logger, "Conectado a KERNEL INTERRUPT.");
    enviar_handshake(socket_interrupt, MODULO_CPU_INTERRUPT);

    free(puerto_memoria);
    free(puerto_dispatch);
    free(puerto_interrupt);
}

