#ifndef MEMORIA_UTILS_H
#define MEMORIA_UTILS_H

#include "conexionMemoria.h"
#include "globales.h"
#include "sockets.h"
#include "utils/structs.h"

int conectar_con_memoria();
bool solicitar_espacio_a_memoria(t_pcb* pcb);
bool solicitar_dump_a_memoria(uint32_t pid);
bool liberar_en_memoria(uint32_t pid);
bool solicitar_desuspender_proceso(uint32_t pid);
bool solicitar_suspender_proceso(uint32_t pid);

#endif
