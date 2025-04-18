#include "main.h"
#include "configuracion.h"

config_cpu_t configCPU;
t_log* logger;
int id_cpu;
int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Falta pasar el ID de CPU como argumento.\n");
        return EXIT_FAILURE;
    }
    id_cpu = atoi(argv[1]); 
    cargarConfiguracionCPU("cpu.config", &configCPU, &logger);
    establecerConexiones(id_cpu);

    while(1){
        usleep(1);
    }
    return 0;
}
