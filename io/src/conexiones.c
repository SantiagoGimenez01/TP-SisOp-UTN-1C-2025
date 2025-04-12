#include "sockets.h"
#include "utils/libs/logger.h"
#include "utils/libs/config.h"
#include "utils/structs.h"
#include "conexiones.h"
int socket_kernel;

void establecerConexiones() {
    char* puerto_kernel = string_itoa(configIO.puerto_kernel);

    // Conexion persistente al KERNEL
    socket_kernel = crearConexion(configIO.ip_kernel, puerto_kernel, logger);
    if (socket_kernel == -1) {
        log_error(logger, "No se pudo conectar al KERNEL desde IO.");
        exit(EXIT_FAILURE);
    }

    log_info(logger, "Conectado a KERNEL desde IO.");
    enviar_handshake(socket_kernel, MODULO_IO);

    free(puerto_kernel);
}

