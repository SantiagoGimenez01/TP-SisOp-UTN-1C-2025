#include <utils/sockets.h>
#include <utils/libs/logger.h>

int main(int argc, char* argv[]) {
    t_log* logger = iniciar_logger("MEMORIA", LOG_LEVEL_INFO);
    log_info(logger, "Hola!");

    int socket = iniciarServidor("5003", logger, "Memoria");
    esperarCliente(socket, logger);
 
    return 0;
}
