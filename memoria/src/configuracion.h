#ifndef CONFIGURACION_H
#define CONFIGURACION_H

#include "configuracion.h"
#include "utils/libs/config.h"
#include "utils/libs/logger.h"
#include "conexiones.h"
#include "globales.h"

void cargarConfiguracionMemoria(char* path, config_memoria_t* configMemoria, t_log** logger);

#endif 