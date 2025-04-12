#ifndef GLOBALES_H
#define GLOBALES_H

#include "utils/libs/config.h"
#include "utils/libs/logger.h"

extern config_kernel_t configKERNEL;  //Esto por ahora lo pongo asi porque los tengo que usar en otro archivo, como conexiones.c pero creo que hay algo mejor
extern t_log* logger;

typedef enum {
    CORTO_PLAZO,
    MEDIANO_PLAZO,
    LARGO_PLAZO,
} t_planificador;

#endif
