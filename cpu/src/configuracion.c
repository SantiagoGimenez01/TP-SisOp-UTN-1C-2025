#include "configuracion.h"

void cargarConfiguracionCPU(char* path, config_cpu_t* configCPU, t_log** logger) {
    t_config* config = iniciar_config(path);
    *configCPU = load_config_cpu(config);
    *logger = iniciar_logger("cpu", configCPU->log_level);

    if(configCPU->entradas_cache > 0){
        cache_paginas = list_create();
    }
    if(configCPU->entradas_tlb > 0){
        tlb = list_create();
    }
}

