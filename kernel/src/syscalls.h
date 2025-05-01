
#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include "conexionMemoria.h"
#include "../../utils/src/utils/structs.h"
#include "inicializar.h"
#include <inttypes.h>

void procesar_syscall(t_paquete* paquete, int socket_cpu);

void atender_syscall_io(t_pcb* pcb, char* nombre_io, int tiempo, int socket_cpu);
void atender_syscall_init_proc(t_pcb* pcb, char* archivo, int tamanio, int socket_cpu);
void atender_syscall_dump_memory(t_pcb* pcb, int socket_cpu);
void atender_syscall_exit(t_pcb* pcb, int socket_cpu);
t_io* buscar_io_por_nombre(char* nombre_io);
void usar_o_encolar_io(t_io* dispositivo, t_pcb* pcb, int tiempo);
void finalizar_proceso(t_pcb* pcb);
bool liberar_en_memoria(uint32_t pid);
void remover_pcb(t_pcb* pcb);
void loguear_metricas_estado(t_pcb* pcb);
t_io* buscar_io_por_socket(int socket);
#endif 
