#ifndef CONEXIONES_H
#define CONEXIONES_H

#include <stdint.h>
#include "globales.h"
void establecerConexiones();
void operarKernel(int socket_cliente);
void operarCPU(int socket_cliente);

#endif
