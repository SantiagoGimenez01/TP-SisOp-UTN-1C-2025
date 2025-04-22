#ifndef CONEXIONES_H
#define CONEXIONES_H

#include <stdint.h>
#include "globales.h"
#include "inicializar.h"
void establecerConexiones();
void operarKernel(int socket_cliente);
void operarCPU(int socket_cliente);
bool hay_espacio_para(uint32_t paginas_requeridas);
t_proceso_en_memoria* buscar_proceso_por_pid(uint32_t pid);

#endif
