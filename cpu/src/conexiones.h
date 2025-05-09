#ifndef CONEXIONES_H
#define CONEXIONES_H
#include "globales.h"
#include "Instrucciones.h"

void establecerConexiones(int id_cpu);
void escucharOperaciones();
void* escuchar_dispatch(void* arg);
void* escuchar_interrupt(void* arg);
void recibir_pcb(int socket_dispatch, uint32_t* pid, uint32_t* pc, uint32_t* estimacion);


#endif