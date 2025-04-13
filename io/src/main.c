#include "main.h"

config_io_t configIO;
t_log* logger;  //GENERAL PARA TODOS LOS MODULOS, siento que aca la estoy manqueando pero por ahora funciona esto

int main(int argc, char* argv[]) { // ACA TENEMOS QUE LABURAR CON LOS ARGUMENTOS PARA INICIAR EL 
    if (argc < 2) {
        printf("Falta pasar el nombre de la IO como argumento.\n");
        return EXIT_FAILURE;
    }
    char* nombre_io = argv[1];

    cargarConfiguracionIO("io.config", &configIO, &logger);
    establecerConexiones(nombre_io);
    while(1){
        usleep(1);
    }
    return 0;
}
