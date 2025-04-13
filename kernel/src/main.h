#include "conexiones.h"
#include "globales.h"

void cargarConfiguracionKernel(char* path, config_kernel_t* configKernel, t_log** logger);


void cargarConfiguracionKernel(char* path, config_kernel_t* configKernel, t_log** logger) {
    t_config* config = iniciar_config(path);          // Carga el config en memoria
    *configKernel = load_config_kernel(config);        // Usa el config para cargar la estructura
    *logger = iniciar_logger("kernel", configKernel->log_level); // Crea el logger
    cpus_incompletas = list_create();
    cpus = list_create();
    ios = list_create();
}

