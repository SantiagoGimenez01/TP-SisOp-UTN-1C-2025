#include "main.h"

int main(int argc, char* argv[]) {
    
    cargarConfiguracion("cpu.config");

    int socketKernel = crearConexion("127.0.0.1", "4000", logger); // Conexion con kernel
    int socketMemoria = crearConexion("127.0.0.1", "5003", logger); // Conexion con memoria
  

    return 0;
}
