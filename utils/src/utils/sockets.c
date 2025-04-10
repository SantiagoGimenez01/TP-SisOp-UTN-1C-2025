#include "sockets.h"
#include <stdlib.h>
#include <unistd.h>    
#include <string.h>    
#include <stdio.h>     

int iniciarServidor(char* puerto, t_log* logger, char* nombre_modulo) {
    struct addrinfo hints, *server_info;
    int socket_servidor = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;        
    hints.ai_socktype = SOCK_STREAM;  
    hints.ai_flags = AI_PASSIVE;       

    if (getaddrinfo(NULL, puerto, &hints, &server_info) != 0) {
        perror("Error en getaddrinfo");
        exit(EXIT_FAILURE);
    }

    socket_servidor = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (socket_servidor == -1) {
        perror("Error creando socket");
        exit(EXIT_FAILURE);
    }

    int activado = 1;
    setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

    if (bind(socket_servidor, server_info->ai_addr, server_info->ai_addrlen) == -1) {
        perror("Error en bind");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(server_info);

    if (listen(socket_servidor, SOMAXCONN) == -1) {  
        perror("Error en listen");
        exit(EXIT_FAILURE);
    }

    log_info(logger, "Servidor %s escuchando en el puerto %s", nombre_modulo, puerto);
    return socket_servidor;
}

int esperarCliente(int socket_servidor, t_log* logger) {
    struct sockaddr_in direccion_cliente;
    socklen_t tamanio_direccion = sizeof(struct sockaddr_in);

    int socket_cliente = accept(socket_servidor, (void*)&direccion_cliente, &tamanio_direccion);

    if (socket_cliente == -1) {
        perror("Error en accept");
        exit(EXIT_FAILURE);
    }

    log_info(logger, "Se conecto un cliente nuevo. FD: %d", socket_cliente);
    return socket_cliente;
}

int crearConexion(char* ip, char* puerto, t_log* logger) {
    struct addrinfo hints, *server_info;
    int socket_cliente;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(ip, puerto, &hints, &server_info) != 0) {
        perror("Error en getaddrinfo");
        exit(EXIT_FAILURE);
    }

    socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    if (socket_cliente == -1) {
        perror("Error creando socket");
        exit(EXIT_FAILURE);
    }

    if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1) {
        perror("Error en connect");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(server_info);

    log_info(logger, "Cliente conectado a %s:%s", ip, puerto);
    return socket_cliente;
}

int enviarBuffer(void* buffer, uint32_t size, int socket_cliente) {
    return send(socket_cliente, buffer, size, 0);
}

void* recibirBuffer(uint32_t* size, int socket_cliente) {
    void* buffer;
    recv(socket_cliente, size, sizeof(uint32_t), 0);
    buffer = malloc(*size);
    recv(socket_cliente, buffer, *size, 0);
    return buffer;
}

void liberarConexion(int socket) {
    close(socket);
}
