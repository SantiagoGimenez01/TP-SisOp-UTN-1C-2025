#ifndef PACKET_H
#define PACKET_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "structs.h"
#include <sys/time.h> //PRUEBO


typedef struct {
    uint32_t size;
    void* stream;
} t_buffer;

typedef struct {
    t_opcode opcode;
    t_buffer* buffer;
} t_paquete;



t_paquete* crear_paquete();
void agregar_string_a_paquete(t_paquete* paquete, char* string);
void agregar_int_a_paquete(t_paquete* paquete, int valor);
void enviar_paquete(t_paquete* paquete, int socket);
void eliminar_paquete(t_paquete* paquete);
t_paquete* recibir_paquete(int socket_cliente);
char* recibir_string_de_paquete(t_paquete* paquete);
void enviar_opcode(t_opcode codigo, int socket);
char* recibir_string_de_paquete_con_offset(t_paquete* paquete, int* offset);
const char* nombre_syscall(t_instruccion_id id);
uint64_t get_timestamp();
void agregar_bloque_a_paquete(t_paquete* paquete, void* bloque, int tamanio);
char* recibir_bloque_de_paquete(t_paquete* paquete, int tamanio_esperado);
#endif 
