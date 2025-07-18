#include "main.h"

config_cpu_t configCPU;
t_config *config;
t_log *logger;
int id_cpu;
int socket_memoria = -1;
int socket_dispatch = -1;
int socket_interrupt = -1;
bool flag_desalojo = false;
pthread_mutex_t mutex_flag_desalojo;

void handler_sigint(int signo)
{
    log_info(logger, "SIGINT recibido, liberando recursos...");
    log_destroy(logger);
    config_destroy(config);
    limpiar_cache();
    limpiar_tlb();
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Falta pasar el ID de CPU como argumento.\n");
        return EXIT_FAILURE;
    }
    id_cpu = atoi(argv[1]);
    pthread_mutex_init(&mutex_flag_desalojo, NULL);

    char *path_config = string_new();
    char *id_cpu_str = string_itoa(id_cpu);

    string_append_with_format(&path_config, "cpu%s.config", id_cpu_str);
    free(id_cpu_str);

    cargarConfiguracionCPU(path_config, id_cpu, &configCPU, &logger);
    free(path_config);

    establecerConexiones(id_cpu);
    escucharOperaciones();
    signal(SIGINT, handler_sigint);

    pause();

    return 0;
}
