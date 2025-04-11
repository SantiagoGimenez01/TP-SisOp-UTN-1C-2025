#include "main.h"

int main(int argc, char* argv[]) {
    cargarConfiguracion("memoria.config");

    int socket = iniciarServidor("5003", logger, "Memoria");
    esperarCliente(socket, logger);
 
    return 0;
}
