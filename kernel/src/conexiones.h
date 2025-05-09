#ifndef CONEXIONES_H
#define CONEXIONES_H

#include "conexiones.h"
#include "conexionMemoria.h"
#include <pthread.h>
#include "inicializar.h"
#include "syscalls.h"

void establecerConexiones();

void operarDispatch(int socket_cliente);
void operarInterrupt(int socket_cliente);
void operarIo(int socket_cliente);

#endif
