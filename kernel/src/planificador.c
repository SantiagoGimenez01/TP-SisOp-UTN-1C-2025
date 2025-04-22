#include "planificador.h"
#include "utils/libs/logger.h"
#include "sockets.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "inicializar.h"


t_list* cola_new;
t_list* cola_ready;
t_list* cola_exit;
t_list* cola_susp_ready;
t_list* cola_susp_blocked;
t_list* pcbs = NULL;  


sem_t sem_procesos_en_new;
pthread_mutex_t mutex_new = PTHREAD_MUTEX_INITIALIZER;

sem_t sem_procesos_en_ready;
sem_t sem_cpu_disponible;
pthread_mutex_t mutex_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cpus = PTHREAD_MUTEX_INITIALIZER;


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
    sem_init(&sem_procesos_en_ready, 0, 0);
    sem_init(&sem_cpu_disponible, 0, 0);

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

t_pcb* obtener_siguiente_de_ready() {
    pthread_mutex_lock(&mutex_ready);

    t_pcb* proceso = NULL;

    if (list_is_empty(cola_ready)) {
        pthread_mutex_unlock(&mutex_ready);
        return NULL;
    }

    if (strcmp(configKERNEL.algoritmo_planificacion, "FIFO") == 0) {
        proceso = list_remove(cola_ready, 0); //HACER BIEN 
    } else if (strcmp(configKERNEL.algoritmo_planificacion, "SJF") == 0) {
        int index_menor = 0;
        for (int i = 1; i < list_size(cola_ready); i++) {
            t_pcb* actual = list_get(cola_ready, i);
            t_pcb* menor = list_get(cola_ready, index_menor);
            if (actual->estimacion_rafaga < menor->estimacion_rafaga) { // Hay que delegar
                index_menor = i;
            }
        }
        proceso = list_remove(cola_ready, index_menor); 
    } else { // FALTA SRT
        log_error(logger, "Algoritmo de planificacion desconocido: %s", configKERNEL.algoritmo_planificacion);
    }

    pthread_mutex_unlock(&mutex_ready);
    return proceso;
}

t_cpu* obtener_cpu_libre() {
    pthread_mutex_lock(&mutex_cpus);

    t_cpu* cpu_libre = NULL;

    for (int i = 0; i < list_size(cpus); i++) {
        t_cpu* cpu = list_get(cpus, i);
        if (cpu->disponible) {
            cpu_libre = cpu;
            cpu->disponible = false;
            break;
        }
    }

    pthread_mutex_unlock(&mutex_cpus);
    return cpu_libre;
}


void* planificador_largo_plazo(void* arg) {
    log_info(logger, "Esperando Enter para iniciar planificacion...");
    getchar();
    log_info(logger, "Planificacion de largo plazo iniciada.");

    while (1) {
        sem_wait(&sem_procesos_en_new);

        t_pcb* siguiente = obtener_siguiente_de_new();
        if (!siguiente) continue;

        bool aceptado = solicitar_espacio_a_memoria(siguiente);

        if (aceptado) {
            cambiar_estado(siguiente, READY);
            pthread_mutex_lock(&mutex_ready);
            list_add(cola_ready, siguiente);
            pthread_mutex_unlock(&mutex_ready);
            sem_post(&sem_procesos_en_ready);  // Avisar al planificador corto plazo

            log_info(logger, "Proceso %d aceptado por Memoria y paso a READY", siguiente->pid);
        } else {
            log_warning(logger, "Memoria rechazo al proceso %d (no hay espacio)", siguiente->pid);
            // Dejarlo en NEW dependiento QUE
        }
    }

    return NULL;
}

void* planificador_corto_plazo(void* arg) {
    while (1) {
        // Espera hasta que haya al menos un proceso en READY
        sem_wait(&sem_procesos_en_ready);
        sem_wait(&sem_cpu_disponible); // me deja dudas por ahora
        
        t_cpu* cpu = obtener_cpu_libre();

        // Obtener el siguiente proceso listo para ejecutar
        t_pcb* proceso = obtener_siguiente_de_ready();
        if (!proceso) {
            log_warning(logger, "No se encontro ningun proceso en READY (posible condición de carrera).");
            cpu->disponible = true;
            continue;
        }

        // Marcar el cambio de estado y enviar el PCB
        cambiar_estado(proceso, EXEC);
        enviar_opcode(EJECUTAR_PROCESO, cpu->socket_dispatch);
        enviar_proceso(cpu, proceso);
        log_info(logger, "Proceso %d enviado a ejecucion en CPU %d", proceso->pid, cpu->id);
    }

    return NULL;
}




void iniciar_planificadores() {
    pthread_t hilo_largo_plazo;
    pthread_t hilo_corto_plazo;

    pthread_create(&hilo_largo_plazo, NULL, planificador_largo_plazo, NULL);
    pthread_detach(hilo_largo_plazo);

    pthread_create(&hilo_corto_plazo, NULL, planificador_corto_plazo, NULL);
    pthread_detach(hilo_corto_plazo);

    log_info(logger, "Planificadores de largo y corto plazo iniciados.");
}



bool solicitar_espacio_a_memoria(t_pcb* pcb) {
    char* puerto_memoria = string_itoa(configKERNEL.puerto_memoria);
    int socket_memoria = crearConexion(configKERNEL.ip_memoria, puerto_memoria, logger);
    free(puerto_memoria);

    if (socket_memoria < 0) {
        log_error(logger, "No se pudo conectar a Memoria para iniciar proceso %d", pcb->pid);
        return false;
    }

    enviar_handshake(socket_memoria, MODULO_KERNEL);
    enviar_opcode(INICIAR_PROCESO, socket_memoria);

    t_paquete* paquete = crear_paquete();
    agregar_int_a_paquete(paquete, pcb->pid);
    agregar_int_a_paquete(paquete, pcb->tamanio);
    agregar_string_a_paquete(paquete, pcb->archivo_pseudocodigo);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    // Esperamos respuesta de MEMORIA
    int cod_respuesta;
    recv(socket_memoria, &cod_respuesta, sizeof(int), MSG_WAITALL);

    bool aceptado = false;

    if (cod_respuesta == RESPUESTA_OK) {
        aceptado = true;
    }

    close(socket_memoria);
    return aceptado;
}

void enviar_proceso(t_cpu* cpu, t_pcb* pcb) {
    t_paquete* paquete = crear_paquete();

    agregar_int_a_paquete(paquete, pcb->pid);
    agregar_int_a_paquete(paquete, pcb->pc);
    //agregar_double_a_paquete(paquete, pcb->estimacion_rafaga);  // por ahora no, veremos

    enviar_paquete(paquete, cpu->socket_dispatch);
    eliminar_paquete(paquete);

    log_info(logger, "Enviado PCB al CPU %d: PID=%d, PC=%d, Estimacion=%.2f", 
             cpu->id, pcb->pid, pcb->pc, pcb->estimacion_rafaga);
}

void agregar_double_a_paquete(t_paquete* paquete, double valor) {
    uint32_t nuevo_tamanio = paquete->buffer->size + sizeof(double);
    paquete->buffer->stream = realloc(paquete->buffer->stream, nuevo_tamanio);

    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(double));

    paquete->buffer->size = nuevo_tamanio;
}

