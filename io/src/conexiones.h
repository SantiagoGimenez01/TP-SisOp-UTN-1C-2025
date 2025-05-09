#ifndef CONEXIONES_H
#define CONEXIONES_H

#include "globales.h"
#include "../../utils/src/utils/paquete.h"
#include "sockets.h"
#include "utils/libs/logger.h"
#include "utils/libs/config.h"
#include "utils/structs.h"

void establecerConexiones(char* nombre_IO);
void escuchar_pedidos_io();

#endif