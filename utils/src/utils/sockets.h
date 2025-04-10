#ifndef SOCKETS_H
#define SOCKETS_H

#include <stdint.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>

int iniciarServidor(char* puerto, t_log* logger, char* nombre_modulo);
int esperarCliente(int socket_servidor, t_log* logger);
int crearConexion(char* ip, char* puerto, t_log* logger);
int enviarBuffer(void* buffer, uint32_t size, int socket_cliente);
void* recibirBuffer(uint32_t* size, int socket_cliente);
void liberarConexion(int socket);

#endif 
