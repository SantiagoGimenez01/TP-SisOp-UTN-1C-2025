#include "planificador.h"

extern config_kernel_t configKERNEL;
extern t_log* logger;

uint32_t proximo_pid = 0;



/* Agregar proceso a NEW
void encolar_en_new(t_pcb* nuevo_proceso) {
    pthread_mutex_lock(&mutex_new);
    list_add(cola_new, nuevo_proceso);
    pthread_mutex_unlock(&mutex_new);
    log_info(logger, "Proceso %d agregado a la cola NEW", nuevo_proceso->pid);
    sem_post(&sem_procesos_en_new); // Habilitar al planificador
}*/

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
        int indexMasChico = 0; //El proceso mas chico comienza siendo el 1ro de la lista
        obtenerIndiceDeProcesoMasChico(cola_new, &indexMasChico); //Obtenemos la posicion del verdadero proceso mas chico
        candidato = list_remove(cola_new, indexMasChico); //Seleccionamos el mas chico y lo sacamos de new
    }

    pthread_mutex_unlock(&mutex_new);
    return candidato;
}

void obtenerIndiceDeProcesoMasChico(t_list* cola_new, int* indexMasChico){
    for(int i = 1; i < list_size(cola_new); i++){
        t_pcb* actual = list_get(cola_new, i);
        t_pcb* menor_actual = list_get(cola_new, *indexMasChico);
        if(actual->tamanio < menor_actual->tamanio)
            *indexMasChico = i;
    }
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
        int indexMasCorto = 0;
        obtenerIndiceDeProcesoMasCorto(cola_ready, &indexMasCorto);
        proceso = list_remove(cola_ready, indexMasCorto); 
    } else { // FALTA SRT
        log_error(logger, "Algoritmo de planificacion desconocido: %s", configKERNEL.algoritmo_planificacion);
    }

    pthread_mutex_unlock(&mutex_ready);
    return proceso;
}

void obtenerIndiceDeProcesoMasCorto(t_list* cola_ready, int* indexMasCorto){
    for(int i = 1; i < list_size(cola_ready); i++){
        t_pcb* actual = list_get(cola_ready, i);
        t_pcb* menor_actual = list_get(cola_ready, *indexMasCorto);
        if(actual->estimacion_rafaga < menor_actual->estimacion_rafaga)
            *indexMasCorto = i;
    }
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
        log_info(logger, "Proceso %d con estimacion inicial: %d", siguiente->pid, siguiente->estimacion_rafaga);
        if (aceptado) {
            cambiar_estado(siguiente, READY);
            //pthread_mutex_lock(&mutex_ready);
            //list_add(cola_ready, siguiente);
            //pthread_mutex_unlock(&mutex_ready);
            //sem_post(&sem_procesos_en_ready);  // Avisar al planificador corto plazo
            //sem_post(&sem_cpu_disponible); // esto no lo tengo que hacer aca solo cuando me instancio por primera vez! 

            log_info(logger, "Proceso %d aceptado por Memoria y paso a READY", siguiente->pid);

        } else {
            log_warning(logger, "Memoria rechazo al proceso %d (no hay espacio)", siguiente->pid);
            // Dejarlo en NEW dependiento QUE
        }
    }

    return NULL;
}

void* planificador_corto_plazo(void* arg) {

    log_info(logger, "Planifiacion de corto plazo iniciada");

    while (1) {
        // Esperar que haya al menos una CPU libre
        sem_wait(&sem_cpu_disponible);
        t_cpu* cpu = obtener_cpu_libre();  

        // Esperamos que haya al menos un proceso en READY
        sem_wait(&sem_procesos_en_ready);
        t_pcb* proceso = obtener_siguiente_de_ready();  

        if (!proceso) {
            //log_warning(logger, "No se encontro proceso en READY, se libera la CPU.");
            cpu->disponible = true;
            //sem_post(&sem_cpu_disponible);  
            continue;
        }

        cpu->disponible = false;

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





void enviar_proceso(t_cpu* cpu, t_pcb* pcb) {
    t_paquete* paquete = crear_paquete();

    agregar_int_a_paquete(paquete, pcb->pid);
    agregar_int_a_paquete(paquete, pcb->pc);
    agregar_int_a_paquete(paquete, pcb->estimacion_rafaga);

    enviar_paquete(paquete, cpu->socket_dispatch);
    eliminar_paquete(paquete);

    log_info(logger, "Enviado PCB al CPU %d: PID=%d, PC=%d, Estimacion=%d", 
             cpu->id, pcb->pid, pcb->pc, pcb->estimacion_rafaga);
}

void agregar_double_a_paquete(t_paquete* paquete, double valor) {
    uint32_t nuevo_tamanio = paquete->buffer->size + sizeof(double);
    paquete->buffer->stream = realloc(paquete->buffer->stream, nuevo_tamanio);

    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(double));

    paquete->buffer->size = nuevo_tamanio;
}


void finalizar_kernel() {
    pthread_mutex_destroy(&mutex_new);
    pthread_mutex_destroy(&mutex_ready);
    pthread_mutex_destroy(&mutex_exit);
    pthread_mutex_destroy(&mutex_susp_ready);
    pthread_mutex_destroy(&mutex_susp_blocked);

    //  destruir las listas y semaforos
}