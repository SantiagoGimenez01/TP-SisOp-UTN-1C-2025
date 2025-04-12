#ifndef MAIN_H
#define MAIN_H

#include "globales.h"
#include "conexiones.h"

void cargarConfiguracionCPU(char* path, config_cpu_t* configCPU, t_log** logger);


void cargarConfiguracionCPU(char* path, config_cpu_t* configCPU, t_log** logger) {
    t_config* config = iniciar_config(path);
    *configCPU = load_config_cpu(config);
    *logger = iniciar_logger("cpu", configCPU->log_level);
}

#endif
