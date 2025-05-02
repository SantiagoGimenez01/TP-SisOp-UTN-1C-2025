#ifndef GLOBALES_H
#define GLOBALES_H

#include "utils/libs/config.h"
#include "utils/libs/logger.h"
#include "utils/paquete.h"
extern config_cpu_t configCPU;
extern t_log* logger;
// Sockets globales
extern int socket_memoria;
extern int socket_dispatch;
extern int socket_interrupt;

extern int tam_pagina;
extern int cant_entradas_por_tabla;
extern int cantidad_niveles;

typedef struct {
    uint32_t numero_pagina;
    uint32_t* entradas_niveles;  // tamanio = cantidad_niveles
    uint32_t desplazamiento;
} t_direccion_fisica;

typedef struct {
    uint32_t nro_pagina;
    int marco;
    uint64_t ultima_uso; 
} t_entrada_tlb;

typedef struct {
    uint32_t nro_pagina;
    char* contenido; //malloc tam pagina
    bool modificado;
    bool uso;  
} t_entrada_cache;

extern t_list* tlb;
extern t_list* cache_paginas;

#endif
