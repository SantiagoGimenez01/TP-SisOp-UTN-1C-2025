#include "main.h"


int main(int argc, char* argv[]) {
    cargarConfiguracion("kernel.config");

    crearConexion("127.0.0.1", "5003", logger); //Maqueta

    int socketServidor = iniciarServidor("4000", logger, "kernel");
    int socketCliente = esperarCliente(socketServidor, logger);

    return 0;
}
