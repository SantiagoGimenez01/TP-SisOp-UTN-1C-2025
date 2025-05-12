#ifndef CONEXIONES_H
#define CONEXIONES_H

#include <stdint.h>
#include "tablaPaginas.h"
#include "inicializar.h"

void establecerConexiones();
void operarKernel(int socket_cliente);
void operarCPU(int socket_cliente);
void* atender_cliente(void* socket_cliente_void);

#endif
