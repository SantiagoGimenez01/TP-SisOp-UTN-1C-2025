#include <utils/sockets.h>
#include <utils/libs/logger.h>
int main(int argc, char* argv[]) {
    t_log* logger = iniciar_logger("KERNEL", LOG_LEVEL_INFO);
    log_info(logger, "Hola!");
    iniciarServidor("4000", logger, "io"); //Maqueta
    iniciarServidor("4001", logger, "cpu"); //Maqueta
    //crearConexion(ip, puerto, logger); //Maqueta

    return 0;
}
