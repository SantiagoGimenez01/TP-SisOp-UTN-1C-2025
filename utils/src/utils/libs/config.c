#include "config.h"

t_config* iniciar_config(char* path) {
    t_config* config = config_create(path);
    if (config == NULL) {
        printf("No se pudo leer el archivo de configuracion: %s\n", path);
        exit(EXIT_FAILURE);
    }
    return config;
}
void check_null_config(t_config* config) {
    if (config != NULL) return;
    fprintf(stderr, "ERROR: config es NULL\n");
    exit(EXIT_FAILURE);
}

t_log_level get_log_level(t_config* config) {
    check_null_config(config);
    char* str = config_get_string_value(config, "LOG_LEVEL");
    return log_level_from_string(str);
}

config_cpu_t load_config_cpu(t_config* config) {
    check_null_config(config);
    config_cpu_t configCpu;

    configCpu.ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    configCpu.puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
    configCpu.ip_kernel = config_get_string_value(config, "IP_KERNEL");
    configCpu.puerto_kernel_dispatch = config_get_int_value(config, "PUERTO_KERNEL_DISPATCH");
    configCpu.puerto_kernel_interrupt = config_get_int_value(config, "PUERTO_KERNEL_INTERRUPT");
    configCpu.entradas_tlb = config_get_int_value(config, "ENTRADAS_TLB");
    configCpu.reemplazo_tlb = config_get_string_value(config, "REEMPLAZO_TLB");
    configCpu.entradas_cache = config_get_int_value(config, "ENTRADAS_CACHE");
    configCpu.reemplazo_cache = config_get_string_value(config, "REEMPLAZO_CACHE");
    configCpu.retardo_cache = config_get_int_value(config, "RETARDO_CACHE");
    configCpu.log_level = get_log_level(config);

    return configCpu;
}

config_kernel_t load_config_kernel(t_config* config) {
    check_null_config(config);
    config_kernel_t configKernel;

    configKernel.ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    configKernel.puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
    configKernel.puerto_escucha_dispatch = config_get_int_value(config, "PUERTO_ESCUCHA_DISPATCH");
    configKernel.puerto_escucha_interrupt = config_get_int_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    configKernel.puerto_escucha_io = config_get_int_value(config, "PUERTO_ESCUCHA_IO");
    configKernel.algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    configKernel.algoritmo_cola_new = config_get_string_value(config, "ALGORITMO_COLA_NEW");
    configKernel.alfa = config_get_double_value(config, "ALFA");  // o config_get_string_value y parsear a double si falla
    configKernel.tiempo_suspension = config_get_int_value(config, "TIEMPO_SUSPENSION");
    configKernel.log_level = get_log_level(config);

    return configKernel;
}
config_io_t load_config_io(t_config* config) {
    check_null_config(config);
    config_io_t configIo;

    configIo.ip_kernel = config_get_string_value(config, "IP_KERNEL");
    configIo.puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
    configIo.log_level = get_log_level(config);

    return configIo;
}

config_memoria_t load_config_memoria(t_config* config) {
    check_null_config(config);
    config_memoria_t configMemoria;

    configMemoria.puerto_escucha = config_get_int_value(config, "PUERTO_ESCUCHA");
    configMemoria.tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
    configMemoria.tam_pagina = config_get_int_value(config, "TAM_PAGINA");
    configMemoria.entradas_por_tabla = config_get_int_value(config, "ENTRADAS_POR_TABLA");
    configMemoria.cantidad_niveles = config_get_int_value(config, "CANTIDAD_NIVELES");
    configMemoria.retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");
    configMemoria.path_swapfile = config_get_string_value(config, "PATH_SWAPFILE");
    configMemoria.retardo_swap = config_get_int_value(config, "RETARDO_SWAP");
    configMemoria.log_level = get_log_level(config);
    configMemoria.dump_path = config_get_string_value(config, "DUMP_PATH");
    configMemoria.pseudocodigoPath = config_get_string_value(config, "PSEUDOCODIGO_PATH");

    return configMemoria;
}

