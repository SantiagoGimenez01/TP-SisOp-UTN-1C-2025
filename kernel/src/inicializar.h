#ifndef INICIALIZAR_H
#define INICIALIZAR_H

extern int proximo_id_cpu;
extern t_list* cpus_incompletas;
extern t_list* cpus;
extern t_list* ios;

void agregarNuevaCpuInc(int socket_cliente);
void agregarNuevaCpu(int socket_cliente);
void agregarNuevaIo(char* nombre, int socket_cliente);

#endif
