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

void establecerConexiones(char* nombre_IO) {
    char* puerto_kernel = string_itoa(configIO.puerto_kernel);

    // Conexion persistente al KERNEL
    socket_kernel = crearConexion(configIO.ip_kernel, puerto_kernel, logger);
    comprobarSocket(socket_kernel, "IO", "KERNEL");
    log_info(logger, "Conectado a KERNEL desde IO.");

    enviar_handshake(socket_kernel, MODULO_IO);

    enviar_opcode(INICIAR_IO, socket_kernel); // esto podriamos hacerlo mejor, no lo hacia asi antes. Puede salir mas CLEAN
    t_paquete* paquete = crear_paquete();
    agregar_string_a_paquete(paquete, nombre_IO);
    enviar_paquete(paquete, socket_kernel);
    eliminar_paquete(paquete);

    free(puerto_kernel);
}


