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

void escuchar_pedidos_io() {
    while (1) {
        t_opcode codigo_operacion;
        int recibidos = recv(socket_kernel, &codigo_operacion, sizeof(t_opcode), MSG_WAITALL);

        if (recibidos <= 0) {
            log_error(logger, "Se cerro la conexion con el Kernel.");
            break;
        }

        switch (codigo_operacion) {
            case SOLICITUD_IO: {
                // Recibir paquete con PID y tiempo
                t_paquete* paquete = recibir_paquete(socket_kernel);
                int offset = 0;

                int pid;
                memcpy(&pid, paquete->buffer->stream + offset, sizeof(int));
                offset += sizeof(int);

                int tiempo;
                memcpy(&tiempo, paquete->buffer->stream + offset, sizeof(int));

                eliminar_paquete(paquete);

                log_info(logger, "## PID: %d - Inicio de IO - Tiempo: %d", pid, tiempo);
                usleep(tiempo * 1000);  // Convertimos a microsegundos, me deja dudas si esto va con el 1000

                log_info(logger, "## PID: %d - Fin de IO", pid);

                enviar_opcode(FIN_IO, socket_kernel);
                t_paquete* respuesta = crear_paquete();
                agregar_int_a_paquete(respuesta, pid);
                enviar_paquete(respuesta, socket_kernel);
                eliminar_paquete(respuesta);

                break;
            }

            default:
                log_warning(logger, "Opcode desconocido recibido en IO: %d", codigo_operacion);
                break;
        }
    }
}


