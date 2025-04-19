#ifndef GLOBALES_H
#define GLOBALES_H

#include "utils/libs/config.h"
#include "utils/libs/logger.h"

extern config_memoria_t configMEMORIA;
extern t_log* logger;
extern t_bitarray* bitmap_frames;
extern int cantidad_frames;
extern int frames_libres;
extern t_list* procesos_en_memoria; 
extern void* memoria_fisica;

typedef struct {
    uint32_t marco;
    bool presencia;
    bool uso;
    bool modificado;
} t_entrada_pagina;

typedef struct {
    t_entrada_pagina** entradas;
} t_tabla_nivel;

typedef struct {
    uint32_t pid;
    t_tabla_nivel* tabla_nivel_1;
} t_proceso_en_memoria;


#endif