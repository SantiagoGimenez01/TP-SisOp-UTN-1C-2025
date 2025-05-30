#ifndef PLANIFICADOR_H
#define PLANIFICADOR_H

#include "inicializar.h"
#include <stdint.h>
#include <semaphore.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include "configuracion.h"
#include "globales.h" 
#include "utils/libs/logger.h"
#include "sockets.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



void inicializarEstados();

void* planificador_largo_plazo(void* arg);
void* planificador_corto_plazo(void* arg);
void* planificador_mediano_plazo(void* arg);
void encolar_en_new(t_pcb* nuevo_proceso);
void iniciar_planificadores();
bool solicitar_espacio_a_memoria(t_pcb* pcb);
t_pcb* obtener_siguiente_de_ready();
t_pcb* obtener_siguiente_de_suspReady();
t_pcb* obtener_siguiente_de_blocked();
t_cpu* obtener_cpu_libre();
void enviar_proceso(t_cpu* cpu, t_pcb* pcb);
void obtenerIndiceDeProcesoMasChico(t_list* cola_new, int* indexMasChico);
void obtenerIndiceDeProcesoMasCorto(t_list* cola_ready, int* indexMasCorto);
void* timer_bloqueo(void* arg);

#endif
