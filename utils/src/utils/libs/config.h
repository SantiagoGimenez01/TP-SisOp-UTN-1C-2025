// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include "../commons/common.h"
#include "commons/log.h"

typedef struct {
    char* ip_memoria;
    uint32_t puerto_memoria;
    char* ip_kernel;
    uint32_t puerto_kernel_dispatch;
    uint32_t puerto_kernel_interrupt;
    uint32_t entradas_tlb;
    char* reemplazo_tlb;    // FIFO o LRU
    uint32_t entradas_cache;
    char* reemplazo_cache;  // CLOCK o CLOCK-M
    uint32_t retardo_cache; // en milisegundos
    t_log_level log_level;
} config_cpu_t;


typedef struct {
    char* ip_kernel;
    uint32_t puerto_kernel;
    t_log_level log_level;
} config_io_t;

typedef struct {
    uint32_t puerto_escucha;
    uint32_t tam_memoria;           
    uint32_t tam_pagina;            
    uint32_t entradas_por_tabla;    
    uint32_t cantidad_niveles;     
    uint32_t retardo_memoria;       
    char* path_swapfile;            
    uint32_t retardo_swap;          
    t_log_level log_level;
    char* dump_path;
    char* pseudocodigoPath;                
} config_memoria_t;

typedef struct {
    char* ip_memoria;
    uint32_t puerto_memoria;
    uint32_t puerto_escucha_dispatch;
    uint32_t puerto_escucha_interrupt;
    uint32_t puerto_escucha_io;
    char* algoritmo_planificacion;  // FIFO, SJF, SRT
    char* algoritmo_cola_new;       // FIFO, PMCP
    double alfa;                    // Para SJF 
    uint32_t tiempo_suspension;      // en milisegundos
    t_log_level log_level;
} config_kernel_t;

t_config* iniciar_config(char* path);
void check_null_config(t_config* config);
t_log_level get_log_level(t_config* config);
config_cpu_t load_config_cpu(t_config* config);
config_kernel_t load_config_kernel(t_config* config);
config_io_t load_config_io(t_config* config);
config_memoria_t load_config_memoria(t_config* config);
#endif
