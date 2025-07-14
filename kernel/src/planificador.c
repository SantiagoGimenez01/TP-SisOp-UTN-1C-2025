#include "planificador.h"

extern config_kernel_t configKERNEL;
extern t_log *logger;

uint32_t proximo_pid = 0;

/* Agregar proceso a NEW
void encolar_en_new(t_pcb* nuevo_proceso) {
    pthread_mutex_lock(&mutex_new);
    list_add(cola_new, nuevo_proceso);
    pthread_mutex_unlock(&mutex_new);
    log_info(logger, "Proceso %d agregado a la cola NEW", nuevo_proceso->pid);
    sem_post(&sem_procesos_en_new); // Habilitar al planificador
}*/

t_pcb *obtener_siguiente_de_new()
{
    t_pcb *candidato = NULL;

    pthread_mutex_lock(&mutex_new);

    if (list_is_empty(cola_new))
    {
        pthread_mutex_unlock(&mutex_new);
        return NULL;
    }

    if (strcmp(configKERNEL.algoritmo_cola_new, "FIFO") == 0)
    {
        candidato = list_get(cola_new, 0); // Debate que tengo
    }
    else if (strcmp(configKERNEL.algoritmo_cola_new, "PMCP") == 0)
    {
        //int indexMasChico = 0;                                    // El proceso mas chico comienza siendo el 1ro de la lista
        //obtenerIndiceDeProcesoMasChico(cola_new, &indexMasChico); // Obtenemos la posicion del verdadero proceso mas chico
        list_sort(cola_new, procesoMasChico); //Ordena los procesos de forma ascendente
        candidato = list_get(cola_new, 0);  // Seleccionamos el primero luego del sort (O sea el mas chico)
    }
    else
    {
        log_error(logger, "Algoritmo de planificacion de cola NEW desconocido: %s", configKERNEL.algoritmo_cola_new);
    }

    pthread_mutex_unlock(&mutex_new);
    return candidato;
}

bool procesoMasChico(void* a, void* b) {
    t_pcb* proceso_a = (t_pcb*) a;
    t_pcb* proceso_b = (t_pcb*) b;
    return proceso_a->tamanio <= proceso_b->tamanio;
}

// void obtenerIndiceDeProcesoMasChico(t_list *cola_new, int *indexMasChico)
// {
//     for (int i = 1; i < list_size(cola_new); i++)
//     {
//         t_pcb *actual = list_get(cola_new, i);
//         t_pcb *menor_actual = list_get(cola_new, *indexMasChico);
//         if (actual->tamanio < menor_actual->tamanio)
//             *indexMasChico = i;
//     }
// }

t_pcb *obtener_siguiente_de_ready()
{
    pthread_mutex_lock(&mutex_ready);

    t_pcb *proceso = NULL;

    if (list_is_empty(cola_ready))
    {
        pthread_mutex_unlock(&mutex_ready);
        return NULL;
    }

    if (strcmp(configKERNEL.algoritmo_planificacion, "FIFO") == 0)
    {
        proceso = list_remove(cola_ready, 0); // HACER BIEN
    }
    else if (strcmp(configKERNEL.algoritmo_planificacion, "SJF") == 0 || strcmp(configKERNEL.algoritmo_planificacion, "SRT") == 0)
    {
        int indexMasCorto = 0;
        obtenerIndiceDeProcesoMasCorto(cola_ready, &indexMasCorto);
        proceso = list_remove(cola_ready, indexMasCorto);
    }
    else
    {
        log_error(logger, "Algoritmo de planificacion desconocido: %s", configKERNEL.algoritmo_planificacion);
    }

    pthread_mutex_unlock(&mutex_ready);
    return proceso;
}

t_pcb *obtener_siguiente_de_blocked()
{
    pthread_mutex_lock(&mutex_blocked);

    t_pcb *proceso = NULL;

    if (list_is_empty(cola_blocked))
    {
        pthread_mutex_unlock(&mutex_blocked);
        return NULL;
    }

    proceso = list_remove(cola_blocked, 0);
    pthread_mutex_unlock(&mutex_blocked);
    return proceso;
}

void obtenerIndiceDeProcesoMasCorto(t_list *cola_ready, int *indexMasCorto)
{
    for (int i = 1; i < list_size(cola_ready); i++)
    {
        t_pcb *actual = list_get(cola_ready, i);
        t_pcb *menor_actual = list_get(cola_ready, *indexMasCorto);
        if (actual->estimacion_rafaga < menor_actual->estimacion_rafaga)
            *indexMasCorto = i;
    }
}

t_cpu *obtener_cpu_con_proc_mas_largo()
{
    pthread_mutex_lock(&mutex_cpus);

    t_cpu *cpu = list_get(cpus, 0);
    // log_info(logger, "ENtre");
    for (int i = 1; i < list_size(cpus); i++)
    {
        t_cpu *cpu_mayor = list_get(cpus, i);
        uint64_t ahora = get_timestamp();
        uint64_t tiempo_ejecucion_mayor = ahora - cpu_mayor->pcb_exec->momento_entrada_estado;
        uint64_t estimacion_restante_mayor = cpu_mayor->pcb_exec->estimacion_rafaga - tiempo_ejecucion_mayor;

        uint64_t tiempo_ejecucion = ahora - cpu->pcb_exec->momento_entrada_estado;
        uint64_t estimacion_restante = cpu->pcb_exec->estimacion_rafaga - tiempo_ejecucion;

        if (estimacion_restante_mayor > estimacion_restante)
            cpu = cpu_mayor;
    }
    pthread_mutex_unlock(&mutex_cpus);
    return cpu;
}

t_cpu *obtener_cpu_libre()
{
    pthread_mutex_lock(&mutex_cpus);

    t_cpu *cpu_libre = NULL;

    for (int i = 0; i < list_size(cpus); i++)
    {
        t_cpu *cpu = list_get(cpus, i);
        if (cpu->disponible)
        {
            log_debug(logger, "Encontro cpu disponible");
            cpu_libre = cpu;
            // cpu->disponible = false;
            break;
        }
    }
    log_debug(logger, "La cpu elegida es %d", cpu_libre->id);
    pthread_mutex_unlock(&mutex_cpus);
    return cpu_libre;
}

t_pcb *obtener_siguiente_de_suspReady()
{
    pthread_mutex_lock(&mutex_susp_ready);
    t_pcb *proceso = NULL;
    if (list_is_empty(cola_susp_ready))
    {
        pthread_mutex_unlock(&mutex_susp_ready);
        return NULL;
    }
    // log_info(logger, "Encontro un proceso en susp ready");
    if (strcmp(configKERNEL.algoritmo_cola_new, "FIFO") == 0)
    {
        proceso = list_get(cola_susp_ready, 0); // Debate que tengo
    }
    else
    {
        //int indexMasChico = 0;                                           // El proceso mas chico comienza siendo el 1ro de la lista
        //obtenerIndiceDeProcesoMasChico(cola_susp_ready, &indexMasChico); // Obtenemos la posicion del verdadero proceso mas chico
        list_sort(cola_susp_ready, procesoMasChico); //Ordena los procesos de forma ascendente
        proceso = list_get(cola_susp_ready, 0);           // Seleccionamos el mas chico y lo sacamos de new
    }

    pthread_mutex_unlock(&mutex_susp_ready);
    // log_info(logger, "## (%d) Es el proceso en susp ready", proceso->pid);
    return proceso;
}

void *planificador_largo_plazo(void *arg)
{
    log_debug(logger, "Esperando Enter para iniciar planificacion...");
    getchar();
    log_debug(logger, "Planificacion de largo plazo iniciada.");

    while (1)
    {
        t_pcb *siguiente;
        log_debug(logger, "ENTRO PLANIFICACION.");
        sem_wait(&sem_procesos_que_van_a_ready);
        if (list_is_empty(cola_susp_ready))
        {
            sem_wait(&sem_procesos_en_new);
            siguiente = obtener_siguiente_de_new();
        }
        else
        {
            sem_wait(&sem_procesos_en_suspReady);
            siguiente = obtener_siguiente_de_suspReady();
        }

        if (!siguiente)
            continue;
    
        bool aceptado;
        
        //Si viene de NEW crea el proceso (INIT_PROC) y si viene de SUSP_READY desuspende el proceso (DESUSPENDER)
        if(siguiente->estado_actual == SUSP_READY)
            aceptado = solicitar_desuspender_proceso(siguiente->pid);
        else if (siguiente->estado_actual == NEW)
            aceptado = solicitar_espacio_a_memoria(siguiente);

        // log_info(logger, "Proceso %d con estimacion inicial: %d", siguiente->pid, siguiente->estimacion_rafaga);
        if (aceptado)
        {
            pthread_mutex_lock(&mutex_new);
        //Si el proceso se elimina de la lista de su estado actual ya que pasa a ready
            if(siguiente->estado_actual == NEW)
                list_remove_element(cola_new, siguiente);
            else
                list_remove_element(cola_susp_ready, siguiente);

            pthread_mutex_unlock(&mutex_new);
            cambiar_estado(siguiente, READY);
            // pthread_mutex_lock(&mutex_ready);
            // list_add(cola_ready, siguiente);
            // pthread_mutex_unlock(&mutex_ready);
            // sem_post(&sem_procesos_en_ready);  // Avisar al planificador corto plazo
            // sem_post(&sem_cpu_disponible); // esto no lo tengo que hacer aca solo cuando me instancio por primera vez!

            log_debug(logger, "Proceso %d aceptado por Memoria y paso a READY", siguiente->pid);
        }
        else
        {
            log_warning(logger, "Memoria rechazo al proceso %d (no hay espacio)", siguiente->pid);
        }
    }

    return NULL;
}

bool hayDesalojo()
{
    if (strcmp(configKERNEL.algoritmo_planificacion, "SJF") == 0 || strcmp(configKERNEL.algoritmo_planificacion, "FIFO") == 0)
        return false;
    else if (strcmp(configKERNEL.algoritmo_planificacion, "SRT") == 0)
        return true;
    else
        log_error(logger, "Algoritmo desconocido");
}

bool hayCpus()
{
    pthread_mutex_lock(&mutex_cpus);
    for (int i = 0; i < list_size(cpus); i++)
    {
        t_cpu *cpu = list_get(cpus, i);
        if (cpu->disponible)
        {
            pthread_mutex_unlock(&mutex_cpus);
            return true;
        }
    }
    pthread_mutex_unlock(&mutex_cpus);
    return false;
}

/*
Basicamente lo que hace es ejecutar el 1er proceso correctamente, luego entran los 4 que crea, compara a todos con el que esta ejecutando, compara bien, no desaloja ya que al
principio todos tienen la misma estimacion inicial pero el que esta en ejecucion tiene mas prioridad (igualmente cuando me logeaba lo que le quedaba ponia 10mil que es la
estimacion inicial y deberia ser estimacion inicial menos lo que ya ejecuto) pero cuando el que esta ejecutando termina, se queda en el semaforo de procesos en ready ya que
los 4 procesos que entraron ya lo consumieron cuando compararon su estimacion con la del proceso en ejecucion, puse un arreglo a esto en inicializar.c que cuando un proceso
pase a exit haga un sem_post pero no estaria funcionando (ademas creo que deberiamos agregarlo como condicion que su estado actual sea EXEC para que si un proceso finaliza de la
nada y pase a EXIT no haga un sem_post de mas).
*/

void *planificador_corto_plazo(void *arg)
{

    log_debug(logger, "Planifiacion de corto plazo iniciada");
    t_cpu *cpu;
    bool desalojo = hayDesalojo(); // Comprueba si el algoritmo del planificador tiene desalojo
    while (1)
    {

        // Espera hasta que: haya procesos en la cola de ready o haya alguna cpu libre
        // El post de este semaforo se hace en dos lugares:
        // - Al cambiar de estado un proceso a READY (mientras no venga de EXEC, ya que ese cambio seria por el desalojo, y no queremos replanificar en ese caso)
        // - Al recibir CPU_LIBRE de cualquier cpu.
        sem_wait(&sem_corto_plazo);

        log_debug(logger, "Entro a corto plazo");
        // Si no hay desalojo...
        if (!desalojo)
        {
            log_debug(logger, "No hay desalojo en este algoritmo");
            // Esperar que haya al menos una CPU libre
            if (!hayCpus())
            {
                continue;
            }
            cpu = obtener_cpu_libre();
            log_debug(logger, "Obtuvo cpu libre");
        }

        // Este semaforo se colgaba esperando un proceso en ready cuando ya tenia en procesos en cola_ready
        // Por eso implemento el otro semaforo que chequea tanto si una CPU se marco libre o si un proceso LLEGÓ a READY
        // Enfasis en llego, ya que no me interesa replanificar si ya chequee desalojo en los procesos de la lista (para eso espero a la CPU_LIBRE)

        // Esperamos que haya al menos un proceso en READY
        // sem_wait(&sem_procesos_en_ready);
        // log_info(logger, "Pasa semaforo procesos en ready");
        // Si hay desalojo comprueba si el proceso que llego a ready requiere una replanificacion

        if (desalojo)
        {
            bool cpusLibres = hayCpus(); // Comprueba si hay CPU libre
            t_pcb *procesoEntrante = NULL;
            pthread_mutex_lock(&mutex_ready);
            if (!list_is_empty(cola_ready))
            {
                procesoEntrante = list_get(cola_ready, list_size(cola_ready) - 1); // Get the last process that entered
                pthread_mutex_unlock(&mutex_ready);
            }
            else
            {
                // En caso que no haya proceso entrante
                // (si puede pasar :) ya que el sem_corto_plazo puede dejar pasar en caso de CPU_LIBRE pero la lista estar vacia)
                log_debug(logger, "Cola ready vacia.");
                pthread_mutex_unlock(&mutex_ready);
                continue;
            }
            // Si no hay CPUs libres...
            if (!cpusLibres)
            {
                log_debug(logger, "No hay CPUs libres");
                cpu = obtener_cpu_con_proc_mas_largo();
                uint64_t ahora = get_timestamp();
                uint64_t tiempo_ejecucion = ahora - cpu->pcb_exec->momento_entrada_estado;
                uint64_t estimacion_restante = cpu->pcb_exec->estimacion_rafaga - tiempo_ejecucion;
                if (procesoEntrante->estimacion_rafaga < estimacion_restante)
                { // El proceso entrante tiene mas prioridad q el q ejecuta
                    log_debug(logger, "HAY DESALOJO");
                    log_debug(logger, "Desalojando proceso %d en CPU %d para ejecutar proceso %d", cpu->pcb_exec->pid, cpu->id, procesoEntrante->pid);
                    log_debug(logger, "Proceso %d en CPU -> Estimacion: %d | Proceso %d en READY -> Estimacion %d", cpu->pcb_exec->pid, estimacion_restante,
                              procesoEntrante->pid, procesoEntrante->estimacion_rafaga);
                    enviar_opcode(INTERRUPCION, cpu->socket_interrupt);
                    cpu->pcb_exec = NULL;
                }
                else
                { // El q ejecuta tiene mas prioridad
                    log_debug(logger, "NO HAY DESALOJO");
                    log_debug(logger, "Proceso %d en CPU %d requiere menos tiempo que proceso %d. NO DESALOJA", cpu->pcb_exec->pid, cpu->id, procesoEntrante->pid);
                    log_debug(logger, "Proceso %d en CPU -> Timer_exec: %d | Proceso %d en READY -> Timer_exec %d", cpu->pcb_exec->pid, estimacion_restante,
                              procesoEntrante->pid, procesoEntrante->timer_exec);
                    continue;
                }
            }
            else
            { // Hay CPU libre, la agarra y ejecuta normal
                log_debug(logger, "Hay CPU libre");
                cpu = obtener_cpu_libre();
                log_debug(logger, "Obtuvo cpu libre");
            }
        }

        t_pcb *proceso; // Aca guardamos el proceso que va a terminar ejecutando

        if (!desalojo || cpu->pcb_exec == NULL)
        {                                           // Si el algoritmo no tiene desalojo (FIFO/SJF) u ocurrio un desalojo (La cpu no tiene proceso en ejecucion)
            proceso = obtener_siguiente_de_ready(); // Se busca el proximo proceso con mas prioridad
        }
        else
        {                            // Si el algoritmo es con desalojo y la cpu tiene un proceso en ejecucion...
            proceso = cpu->pcb_exec; // Significa que no hubo desalojo, por lo que el proceso seguira siendo el mismo
        }

        // Si la cpu no tiene proceso la liberamos
        if (!proceso)
        {
            // log_warning(logger, "No se encontro proceso en READY, se libera la CPU.");
            cpu->disponible = true;
            // sem_post(&sem_cpu_disponible);
            continue;
        }

        if (cpu == NULL)
        {
            log_error(logger, "CPU NULL en planificador de corto plazo");
            continue;
        }

        cpu->disponible = false; // Esto nose si se pisaria con lo anterior si es que la cpu no tiene proceso, ni idea
        cpu->pcb_exec = proceso; // Asentamos que el proceso en ejecucion es el que seleccionamos de ready o el que ya venia ejecutando
        // Si el proceso viene de un estado distinto de EXEC significa que viene de ready o blocked, por lo que lo va a cambiar de estado, poner a ejecutar, etc...
        // Si no hubo desalojo en SRT significa que el proceso que esta en CPU ya esta ejecutando, por lo que no haria falta cambiarlo de estado, mandarlo a ejecutar a cpu, etc..
        if (proceso->estado_actual != EXEC)
        {
            cambiar_estado(proceso, EXEC);
            enviar_opcode(EJECUTAR_PROCESO, cpu->socket_dispatch);
            enviar_proceso(cpu, proceso);
            log_debug(logger, "Proceso %d enviado a ejecucion en CPU %d", proceso->pid, cpu->id);
        }
    }

    return NULL;
}

void *planificador_mediano_plazo(void *arg)
{
    log_debug(logger, "Planificacion de mediano plazo iniciada");

    while (1)
    {
        sem_wait(&sem_procesos_en_blocked);          // Espero a que haya algun proceso bloqueado
        t_pcb *pcb = obtener_siguiente_de_blocked(); // Obtiene proceso bloqueado
        pcb->timer_flag = 1;
        // ACA INICIA EL TIMER DE SUSPENSION!!!!
        pthread_t hilo_timer;
        pthread_create(&hilo_timer, NULL, timer_bloqueo, pcb);
        pthread_detach(hilo_timer);
    }

    return NULL;
}

void *timer_bloqueo(void *arg)
{
    t_pcb *pcb = (t_pcb *)arg;

    log_info(logger, "##(%d) Comenzando timer...", pcb->pid);
    usleep(configKERNEL.tiempo_suspension * 1000); // Espera
    // Si todavia sigue bloqueado lo suspende
    pthread_mutex_lock(&pcb->mutex_pcb);
    if (pcb->estado_actual == BLOCKED && pcb->timer_flag > 0)
    {
        bool resultado = solicitar_suspender_proceso(pcb->pid);
        if (resultado)
        {
            log_info(logger, "##(%d) Suspendiendose...", pcb->pid);
            cambiar_estado(pcb, SUSP_BLOCKED);
        }
        else
        {
            log_error(logger, "##(%d) No se suspendio correctamente", pcb->pid);
        }
    }
    else
    {
        log_info(logger, "##(%d) El proceso ya se desbloqueó antes, timer invalido", pcb->pid);
    }

    pthread_mutex_unlock(&pcb->mutex_pcb);
    return NULL;
}

void iniciar_planificadores()
{
    pthread_t hilo_largo_plazo;
    pthread_t hilo_corto_plazo;
    pthread_t hilo_mediano_plazo;

    pthread_create(&hilo_largo_plazo, NULL, planificador_largo_plazo, NULL);
    pthread_detach(hilo_largo_plazo);

    pthread_create(&hilo_corto_plazo, NULL, planificador_corto_plazo, NULL);
    pthread_detach(hilo_corto_plazo);

    pthread_create(&hilo_mediano_plazo, NULL, planificador_mediano_plazo, NULL);
    pthread_detach(hilo_mediano_plazo);

    log_debug(logger, "Planificadores de largo, corto y mediano plazo iniciados.");
}

void enviar_proceso(t_cpu *cpu, t_pcb *pcb)
{
    t_paquete *paquete = crear_paquete();

    agregar_int_a_paquete(paquete, pcb->pid);
    agregar_int_a_paquete(paquete, pcb->pc);
    agregar_int_a_paquete(paquete, pcb->estimacion_rafaga);
    agregar_int_a_paquete(paquete, pcb->timer_exec);

    enviar_paquete(paquete, cpu->socket_dispatch);
    eliminar_paquete(paquete);

    log_debug(logger, "Enviado PCB al CPU %d: PID=%d, PC=%d, Estimacion=%d, Timer Exec=%d",
              cpu->id, pcb->pid, pcb->pc, pcb->estimacion_rafaga, pcb->timer_exec);
}

void agregar_double_a_paquete(t_paquete *paquete, double valor)
{
    uint32_t nuevo_tamanio = paquete->buffer->size + sizeof(double);
    paquete->buffer->stream = realloc(paquete->buffer->stream, nuevo_tamanio);

    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(double));

    paquete->buffer->size = nuevo_tamanio;
}

void finalizar_kernel()
{
    pthread_mutex_destroy(&mutex_new);
    pthread_mutex_destroy(&mutex_ready);
    pthread_mutex_destroy(&mutex_exit);
    pthread_mutex_destroy(&mutex_susp_ready);
    pthread_mutex_destroy(&mutex_susp_blocked);
    pthread_mutex_destroy(&mutex_blocked);

    //  destruir las listas y semaforos
}