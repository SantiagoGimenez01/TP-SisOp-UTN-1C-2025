#ifndef INICIALIZAR_H
#define INICIALIZAR_H

#include "inicializar.h"
#include "../../utils/src/utils/structs.h"
#include <semaphore.h>
#include "globales.h"
#include "../../utils/src/utils/structs.h"
#include <sys/time.h>
#include "planificador.h"

extern int proximo_id_cpu;
extern t_list* cpus_incompletas;
extern t_list* cpus;
extern t_list* ios;

extern t_list* cola_new;
extern t_list* cola_ready;
extern t_list* cola_blocked;
extern t_list* cola_exit;
extern t_list* cola_susp_ready;
extern t_list* cola_susp_blocked;
extern t_list* pcbs;  

extern sem_t sem_corto_plazo;

extern sem_t sem_procesos_en_new;
extern sem_t sem_procesos_en_ready;
extern sem_t sem_procesos_en_blocked;
extern sem_t sem_procesos_en_suspReady;
extern sem_t sem_procesos_que_van_a_ready;

extern sem_t respuesta_estimacion;

extern sem_t sem_agregar_cpu;

extern pthread_mutex_t mutex_new;
extern pthread_mutex_t mutex_ready;
extern pthread_mutex_t mutex_blocked;
extern pthread_mutex_t mutex_exit;
extern pthread_mutex_t mutex_susp_ready;
extern pthread_mutex_t mutex_susp_blocked;

extern sem_t sem_cpu_disponible;
extern pthread_mutex_t mutex_cpus;
extern pthread_mutex_t mutex_pcbs;


void inicializarEstados();
void agregarNuevaCpuInc(int socket_cliente, int id_cpu);
void agregarNuevaCpu(int socket_cliente, int id_cpu);
void agregarNuevaIo(char* nombre, int socket_cliente);
t_cpu* buscar_cpu_por_id(int id_cpu);
void inicializar_proceso(char* archivo_pseudocodigo, int tamanio);

void cambiar_estado(t_pcb* pcb, t_estado_proceso nuevo_estado);
t_metricas_estado* buscar_o_crear_metrica(t_list* metricas, t_estado_proceso estado);
const char* nombre_estado(t_estado_proceso estado);
//void log_metrica_final(t_pcb* pcb); SUPUESTAMENTE al contabilizar con un uint64 el tiempo ee mejor usar PRIU64 pero esto lo dejo por las dudas cuando probemos SFJ
void remover_de_cola(t_pcb* pcb, t_estado_proceso estado);
void agregar_a_cola(t_pcb* pcb, t_estado_proceso estado);
t_pcb* buscar_pcb_por_pid(uint32_t pid);
t_cpu* obtener_cpu_por_socket_dispatch(int socket_dispatch);
t_cpu* obtener_cpu_por_socket_interrupt(int socket_interrupt);
void marcar_cpu_como_libre(int socket_dispatch, bool es_socket_dispatch);
int calcularEstimacion(t_pcb *pcb);

#endif
