#include "globales.h"
#include "inicializar.h"
t_list* cpus;
t_list* ios;
t_list* cpus_incompletas;
int proximo_id_cpu = -1;

void agregarNuevaCpuInc(int socket_cliente) {
    t_cpu* nueva_cpu = malloc(sizeof(t_cpu));
    nueva_cpu->socket_dispatch = socket_cliente;
    nueva_cpu->socket_interrupt = -1; 
    nueva_cpu->disponible = 1;
    nueva_cpu->id = proximo_id_cpu++;

    list_add(cpus_incompletas, nueva_cpu);
    log_info(logger, "CPU incompleta (solo DISPATCH) con ID: %d", nueva_cpu->id);
}

void agregarNuevaCpu(int socket_cliente) {
    for (int i = 0; i < list_size(cpus_incompletas); i++) {
        t_cpu* cpu = list_get(cpus_incompletas, i);
        if (cpu->socket_interrupt == -1) {
            cpu->socket_interrupt = socket_cliente; 
            list_add(cpus, cpu);           
            list_remove(cpus_incompletas, i);        
            log_info(logger, "CPU ID %d completa (DISPATCH + INTERRUPT)", cpu->id);
            return;
        }
    }
    
    log_warning(logger, "Llego un INTERRUPT pero no habia CPU esperando.");
    close(socket_cliente);
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