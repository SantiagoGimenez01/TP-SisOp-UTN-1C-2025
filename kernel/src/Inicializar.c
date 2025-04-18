#include "globales.h"
#include "inicializar.h"
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