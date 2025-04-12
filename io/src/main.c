#include "main.h"

config_io_t configIO;
t_log* logger;

int main(int argc, char* argv[]) { // ACA TENEMOS QUE LABURAR CON LOS ARGUMENTOS PARA INICIAR EL MODULO
    cargarConfiguracionIO("io.config", &configIO, &logger);
    establecerConexiones();
    return 0;
}
