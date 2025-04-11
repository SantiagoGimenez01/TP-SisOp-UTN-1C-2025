#include <utils/sockets.h>
#include <utils/libs/logger.h>
int main(int argc, char* argv[]) {
    t_log* logger = iniciar_logger("KERNEL", LOG_LEVEL_INFO);
    log_info(logger, "Hola!");

    crearConexion("127.0.0.1", "5003", logger); //Maqueta

    int socketServidor = iniciarServidor("4000", logger, "kernel");
    int socketCliente = esperarCliente(socketServidor, logger);

    return 0;
}
