#include "configuracion.h"

void cargarConfiguracionCPU(char* path, int id_cpu, config_cpu_t* configCPU, t_log** logger) {
    // Carga config
    t_config* config = iniciar_config(path);
    *configCPU = load_config_cpu(config);

    // Crea log file
    char* nombre_log_file = string_new();
    char* id_cpu_str = string_itoa(id_cpu);
    string_append_with_format(&nombre_log_file, "cpu%s", id_cpu_str);
    free(id_cpu_str);
    *logger = iniciar_logger(nombre_log_file, configCPU->log_level);
    free(nombre_log_file);

    // Asigna cache y tlb (si son necesarias)
    if(configCPU->entradas_cache > 0){
        cache_paginas = list_create();
    }
    if(configCPU->entradas_tlb > 0){
        tlb = list_create();
    }
}

