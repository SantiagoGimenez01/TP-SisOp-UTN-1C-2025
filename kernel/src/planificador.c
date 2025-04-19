#include "planificador.h"
#include "utils/libs/logger.h"
#include "sockets.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

t_list* cola_new;
t_list* cola_ready;
t_list* cola_exit;
t_list* cola_susp_ready;
t_list* cola_susp_blocked;
t_list* pcbs = NULL;  


sem_t sem_procesos_en_new;
pthread_mutex_t mutex_new = PTHREAD_MUTEX_INITIALIZER;

extern config_kernel_t configKERNEL;
extern t_log* logger;

uint32_t proximo_pid = 0;

void inicializarEstados() {
    cola_new = list_create();
    cola_ready = list_create();
    cola_exit = list_create();
    cola_susp_ready = list_create();
    cola_susp_blocked = list_create();
    sem_init(&sem_procesos_en_new, 0, 0);
}

// Agregar proceso a NEW
void encolar_en_new(t_pcb* nuevo_proceso) {
    pthread_mutex_lock(&mutex_new);
    list_add(cola_new, nuevo_proceso);
    pthread_mutex_unlock(&mutex_new);
    log_info(logger, "Proceso %d agregado a la cola NEW", nuevo_proceso->pid);
    sem_post(&sem_procesos_en_new); // Habilitar al planificador
}

t_pcb* obtener_siguiente_de_new() {
    t_pcb* candidato = NULL;

    pthread_mutex_lock(&mutex_new);

    if (list_is_empty(cola_new)) {
        pthread_mutex_unlock(&mutex_new);
        return NULL;
    }

    if (strcmp(configKERNEL.algoritmo_cola_new, "FIFO") == 0) {
        candidato = list_remove(cola_new, 0);  //Debate que tengo
    } else {
        int index_mas_chico = 0;
        for (int i = 1; i < list_size(cola_new); i++) {
            t_pcb* actual = list_get(cola_new, i);
            t_pcb* menor_actual = list_get(cola_new, index_mas_chico);
            if (actual->tamanio < menor_actual->tamanio) {
                index_mas_chico = i;
            }
        }
        candidato = list_remove(cola_new, index_mas_chico);
    }

    pthread_mutex_unlock(&mutex_new);
    return candidato;
}

void* planificador_largo_plazo(void* arg) {
    log_info(logger, "Esperando Enter para iniciar planificacion...");
    getchar();
    log_info(logger, "Planificacion de largo plazo iniciada.");

    while (1) {
        sem_wait(&sem_procesos_en_new);

        t_pcb* siguiente = obtener_siguiente_de_new();
        if (!siguiente) continue;

        // consulta a MEMORIA
        // Por ahora lo pasamos directamente a READY
        list_add(cola_ready, siguiente);
        log_info(logger, "Proceso %d paso a READY", siguiente->pid);
    }

    return NULL;
}

void iniciar_planificacion_largo_plazo() {
    pthread_t hilo_largo_plazo;
    pthread_create(&hilo_largo_plazo, NULL, planificador_largo_plazo, NULL);
    pthread_detach(hilo_largo_plazo);
}
