#include "main.h"

config_cpu_t configCPU;
t_log* logger;
int id_cpu;
int socket_memoria = -1;
int socket_dispatch = -1;
int socket_interrupt = -1;
bool flag_desalojo = false;
pthread_mutex_t mutex_flag_desalojo;
uint32_t estimacion_restante;

int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Falta pasar el ID de CPU como argumento.\n");
        return EXIT_FAILURE;
    }
    id_cpu = atoi(argv[1]); 
    pthread_mutex_init(&mutex_flag_desalojo, NULL);
    cargarConfiguracionCPU("cpu.config", &configCPU, &logger);
    establecerConexiones(id_cpu);
    escucharOperaciones();
    while(1){
        usleep(1);
    }
    return 0;
}
