#include <utils/libs/logger.h>
#include <utils/sockets.h>


int main(int argc, char* argv[]) {
    t_log* logger = iniciar_logger("CPU", LOG_LEVEL_INFO);
    log_info(logger, "Hola!");

    int socketKernel = crearConexion("127.0.0.1", "4000", logger); // Conexion con kernel
    int socketMemoria = crearConexion("127.0.0.1", "5003", logger); // Conexion con memoria
  

    return 0;
}
