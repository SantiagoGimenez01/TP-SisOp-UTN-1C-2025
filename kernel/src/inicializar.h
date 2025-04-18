#ifndef INICIALIZAR_H
#define INICIALIZAR_H

extern int proximo_id_cpu;
extern t_list* cpus_incompletas;
extern t_list* cpus;
extern t_list* ios;

void agregarNuevaCpuInc(int socket_cliente, int id_cpu);
void agregarNuevaCpu(int socket_cliente, int id_cpu);
void agregarNuevaIo(char* nombre, int socket_cliente);
t_cpu* buscar_cpu_por_id(int id_cpu);

#endif
