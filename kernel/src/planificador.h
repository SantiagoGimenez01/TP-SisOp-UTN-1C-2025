#ifndef PLANIFICADOR_H
#define PLANIFICADOR_H

#include <stdint.h>
#include <semaphore.h>
#include <pthread.h>
#include <commons/collections/list.h>

#include "configuracion.h"
#include "globales.h" 

void inicializarEstados();

void* planificador_largo_plazo(void* arg);
void encolar_en_new(t_pcb* nuevo_proceso);
void iniciar_planificadores();
bool solicitar_espacio_a_memoria(t_pcb* pcb);
t_pcb* obtener_siguiente_de_ready();
t_cpu* obtener_cpu_libre();
void enviar_proceso(t_cpu* cpu, t_pcb* pcb);

#endif
