#ifndef CONEXIONES_H
#define CONEXIONES_H

#include "globales.h"
void establecerConexiones();

void operarDispatch(int socket_cliente);
void operarInterrupt(int socket_cliente);
void operarIo(int socket_cliente);
void conectar_con_memoria();
#endif
