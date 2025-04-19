#include "globales.h"
#include "inicializar.h"
#include "../../utils/src/utils/structs.h"
#include <sys/time.h>
#include "planificador.h"

t_list* cpus;
t_list* ios;
t_list* cpus_incompletas;

void agregarNuevaCpuInc(int socket_cliente, int id_cpu) {
    t_cpu* nueva_cpu = malloc(sizeof(t_cpu));
    nueva_cpu->socket_dispatch = socket_cliente;
    nueva_cpu->socket_interrupt = -1; 
    nueva_cpu->disponible = 1;
    nueva_cpu->id = id_cpu;

    list_add(cpus_incompletas, nueva_cpu);
    log_info(logger, "CPU incompleta (solo DISPATCH) con ID: %d", nueva_cpu->id);
}

t_cpu* buscar_cpu_por_id(int id_cpu) {
    for (int i = 0; i < list_size(cpus_incompletas); i++) {
        t_cpu* cpu = list_get(cpus_incompletas, i);
        if (cpu->id == id_cpu) {
            return cpu;
        }
    }
    return NULL;
}


void agregarNuevaCpu(int socket_interrupt, int id_cpu) {
    t_cpu* cpu = buscar_cpu_por_id(id_cpu);
    if (cpu != NULL) {
        cpu->socket_interrupt = socket_interrupt;
        list_add(cpus, cpu);
        list_remove_element(cpus_incompletas, cpu);
        log_info(logger, "CPU ID %d completa (DISPATCH + INTERRUPT)", cpu->id);
    } else {
        log_warning(logger, "No se encontro CPU con ID %d para completar INTERRUPT.", id_cpu);
        close(socket_interrupt);
    }
}



// Ahora se me ocurrio con estas 2 funciones la manera de agregar la nueva CPU, seguro lo podemos mejorar

void agregarNuevaIo(char* nombre, int socket_cliente) {
    t_io* nueva_io = malloc(sizeof(t_io));
    nueva_io->nombre = strdup(nombre);  
    nueva_io->socket = socket_cliente;
    nueva_io->disponible = 0;         

    list_add(ios, nueva_io);

    log_info(logger, "Se agrego IO '%s' al sistema.", nombre);
}


void inicializar_proceso(char* archivo_pseudocodigo, int tamanio) {
    t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));
    nuevo_pcb->pid = pid_global++;
    nuevo_pcb->pc = 0;
    nuevo_pcb->tamanio = tamanio;
    nuevo_pcb->tablaPaginas  = -1;
    nuevo_pcb->estado_actual = NEW;
    nuevo_pcb->momento_entrada_estado = get_timestamp();
    nuevo_pcb->metricas = list_create();
    nuevo_pcb->estimacion_rafaga = 0; 
    nuevo_pcb->rafaga_real = 0;
    nuevo_pcb->archivo_pseudocodigo = strdup(archivo_pseudocodigo);
    
    log_info(logger, "## (%d) Se crea el proceso - Estado: NEW", nuevo_pcb->pid);

    encolar_en_new(nuevo_pcb);
    cambiar_estado(nuevo_pcb, NEW);
}

void cambiar_estado(t_pcb* pcb, t_estado_proceso nuevo_estado) {
    uint64_t ahora = get_timestamp(); //con pinzas

    uint64_t duracion = ahora - pcb->momento_entrada_estado;
    t_metricas_estado* metrica = buscar_o_crear_metrica(pcb->metricas, pcb->estado_actual);
    metrica->cantVeces++;
    metrica->tiempoTotal += duracion;

    pcb->estado_actual = nuevo_estado;
    pcb->momento_entrada_estado = ahora;

    log_info(logger, "## (%d) Pasa del estado %s al estado %s", 
             pcb->pid, nombre_estado(metrica->estado), nombre_estado(nuevo_estado));
}

uint64_t get_timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000); // tremenda fruta pero vamos a ver que sale
}

t_metricas_estado* buscar_o_crear_metrica(t_list* metricas, t_estado_proceso estado) {
    for (int i = 0; i < list_size(metricas); i++) {
        t_metricas_estado* m = list_get(metricas, i);
        if (m->estado == estado)
            return m;
    }

    t_metricas_estado* nueva = malloc(sizeof(t_metricas_estado));
    nueva->estado = estado;
    nueva->cantVeces = 0;
    nueva->tiempoTotal = 0;
    list_add(metricas, nueva);
    return nueva;
}

const char* nombre_estado(t_estado_proceso estado) {
    switch (estado) {
        case NEW: return "NEW";
        case READY: return "READY";
        case EXEC: return "EXEC";
        case BLOCKED: return "BLOCKED";
        case SUSP_READY: return "SUSP_READY";
        case SUSP_BLOCKED: return "SUSP_BLOCKED";
        case EXIT_PROCESS: return "EXIT";
        default: return "DESCONOCIDO";
    }
}

void log_metrica_final(t_pcb* pcb) {
    log_info(logger, "## (%d) - Metricas de estado:", pcb->pid);
    for (int i = 0; i < list_size(pcb->metricas); i++) {
        t_metricas_estado* m = list_get(pcb->metricas, i);
        log_info(logger, "  %s (%d) (%lu ms)", nombre_estado(m->estado), m->cantVeces, m->tiempoTotal);
    }
}