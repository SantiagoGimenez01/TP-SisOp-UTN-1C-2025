#include "sockets.h"
#include "utils/libs/logger.h"
#include "utils/libs/config.h"
#include "utils/structs.h"
#include "conexiones.h"
int socket_kernel;

void comprobarSocket(int socket, char* moduloOrigen, char* moduloDestino){
    if(socket == -1){
        log_error(logger, "No se pudo conectar a %s desde %s", moduloDestino, moduloOrigen);
        exit(EXIT_FAILURE);
    }
}

void establecerConexiones() {
    char* puerto_kernel = string_itoa(configIO.puerto_kernel);

    // Conexion persistente al KERNEL
    socket_kernel = crearConexion(configIO.ip_kernel, puerto_kernel, logger);
    comprobarSocket(socket_kernel, "IO", "KERNEL");
    log_info(logger, "Conectado a KERNEL desde IO.");
    enviar_handshake(socket_kernel, MODULO_IO);

    free(puerto_kernel);
}


