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
extern t_list* paginas_en_swap;

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
    int accesos_tablas_paginas;
    int instrucciones_solicitadas;
    int bajadas_a_swap;
    int subidas_de_swap;
    int lecturas_memoria;
    int escrituras_memoria;
} t_metricas_proceso;

typedef struct {
    uint32_t pid;
    t_tabla_nivel* tabla_nivel_1;
    char* nombre_archivo;
    int tamanio;
    t_list* instrucciones;  // Lista de char*
    t_metricas_proceso metricas;
    uint32_t paginas_necesarias;
    
} t_proceso_en_memoria;

typedef struct {
    int pid;
    int nro_pagina;
    int slot; // slot = posicion en swapfile.bin (0 = 0 bytes, 1 = 64 bytes, etc)
} t_registro_swap;


#endif