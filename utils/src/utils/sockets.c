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

    log_debug(logger, "Servidor %s escuchando en el puerto %s", nombre_modulo, puerto);
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

    log_debug(logger, "Se conecto un cliente nuevo. FD: %d", socket_cliente);
    return socket_cliente;
}

int crearConexion(char* ip, char* puerto, t_log* logger) {
    struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;

    int rv = getaddrinfo(ip, puerto, &hints, &servinfo);
    if (rv != 0) {
        log_error(logger, "Error en getaddrinfo: %s", gai_strerror(rv));
        return -1;
    }

    int socket_cliente = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (socket_cliente == -1) {
        log_error(logger, "Error al crear socket cliente");
        freeaddrinfo(servinfo);
        return -1;
    }

    if (connect(socket_cliente, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        log_error(logger, "Error al conectar al servidor");
        close(socket_cliente);
        freeaddrinfo(servinfo);
        return -1;
    }

    freeaddrinfo(servinfo);
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


void enviar_handshake(int socket, t_modulo modulo) {
    if (send(socket, &modulo, sizeof(t_modulo), 0) <= 0) {
        perror("Error al enviar handshake");
        exit(EXIT_FAILURE);
    }
}

t_modulo recibir_handshake(int socket) {
    t_modulo modulo;
    ssize_t bytes_recibidos = recv(socket, &modulo, sizeof(t_modulo), MSG_WAITALL);
    if (bytes_recibidos <= 0) {
        perror("Error al recibir handshake");
        exit(EXIT_FAILURE);
    }
    return modulo;
}