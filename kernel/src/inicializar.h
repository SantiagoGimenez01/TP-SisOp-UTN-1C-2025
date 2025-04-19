#ifndef INICIALIZAR_H
#define INICIALIZAR_H

#include "../../utils/src/utils/structs.h"

extern int proximo_id_cpu;
extern t_list* cpus_incompletas;
extern t_list* cpus;
extern t_list* ios;

void agregarNuevaCpuInc(int socket_cliente, int id_cpu);
void agregarNuevaCpu(int socket_cliente, int id_cpu);
void agregarNuevaIo(char* nombre, int socket_cliente);
t_cpu* buscar_cpu_por_id(int id_cpu);
void inicializar_proceso(char* archivo_pseudocodigo, int tamanio);
uint64_t get_timestamp();
void cambiar_estado(t_pcb* pcb, t_estado_proceso nuevo_estado);
t_metricas_estado* buscar_o_crear_metrica(t_list* metricas, t_estado_proceso estado);
const char* nombre_estado(t_estado_proceso estado);
void log_metrica_final(t_pcb* pcb);


#endif
