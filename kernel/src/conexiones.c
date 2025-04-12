#include "sockets.h"
#include "utils/libs/logger.h"
#include "utils/libs/config.h"
#include "utils/structs.h"
#include <pthread.h>
#include "conexiones.h"


// --- Funciones auxiliares para cada puerto ---

void* escuchar_dispatch(void* socket_servidor_void) {
    int socket_servidor = (intptr_t)socket_servidor_void;
    log_info(logger, "Servidor KERNEL_DISPATCH escuchando conexiones.");

    while (1) {
        int socket_cliente = esperarCliente(socket_servidor, logger);

        // estoy en duda con esto si hacer recv o recibir handshake
        t_modulo modulo_origen;
        recv(socket_cliente, &modulo_origen, sizeof(t_modulo), 0);

        if (modulo_origen == MODULO_CPU_DISPATCH) {
            log_info(logger, "Se conecto CPU por DISPATCH.");
            operarDispatch(socket_cliente); // aca es donde vamos a trabajar las operaciones que provengan de CPU
        } else {
            log_warning(logger, "No es DISPATCH");
            close(socket_cliente);
        }
    }
    return NULL;
}

void* escuchar_interrupt(void* socket_servidor_void) {
    int socket_servidor = (intptr_t)socket_servidor_void;
    log_info(logger, "Servidor KERNEL_INTERRUPT escuchando conexiones.");

    while (1) {
        int socket_cliente = esperarCliente(socket_servidor, logger);

        t_modulo modulo_origen;
        recv(socket_cliente, &modulo_origen, sizeof(t_modulo), 0);

        if (modulo_origen == MODULO_CPU_INTERRUPT) {
            log_info(logger, "Se conecto CPU por INTERRUPT.");
            operarInterrupt(socket_cliente); // mismo caso interrupciones
        } else {
            log_warning(logger, "No es INTERRUPT.");
            close(socket_cliente);
        }
    }
    return NULL;
}

void* escuchar_io(void* socket_servidor_void) {
    int socket_servidor = (intptr_t)socket_servidor_void;
    log_info(logger, "Servidor KERNEL_IO escuchando conexiones.");

    while (1) {
        int socket_cliente = esperarCliente(socket_servidor, logger);

        t_modulo modulo_origen;
        recv(socket_cliente, &modulo_origen, sizeof(t_modulo), 0);

        if (modulo_origen == MODULO_IO) {
            log_info(logger, "Se conecto un modulo IO.");
            operarIo(socket_cliente); // operandon conlas IO
        } else {
            log_warning(logger, "No es IO.");
            close(socket_cliente);
        }
    }
    return NULL;
}

// --- Funcion principal ---

void establecerConexiones() {
    // Iniciar 3 servidores TCP (en teoria como entendi deberia ser asi...)
    int socket_dispatch = iniciarServidor(string_itoa(configKERNEL.puerto_escucha_dispatch), logger, "KERNEL_DISPATCH");
    int socket_interrupt = iniciarServidor(string_itoa(configKERNEL.puerto_escucha_interrupt), logger, "KERNEL_INTERRUPT");
    int socket_io = iniciarServidor(string_itoa(configKERNEL.puerto_escucha_io), logger, "KERNEL_IO");

    // Crear 3 hilos para escuchar en paralelo
    pthread_t hilo_dispatch, hilo_interrupt, hilo_io;
    pthread_create(&hilo_dispatch, NULL, escuchar_dispatch, (void*)(intptr_t)socket_dispatch);
    pthread_create(&hilo_interrupt, NULL, escuchar_interrupt, (void*)(intptr_t)socket_interrupt);
    pthread_create(&hilo_io, NULL, escuchar_io, (void*)(intptr_t)socket_io);

    pthread_detach(hilo_dispatch);
    pthread_detach(hilo_interrupt);
    pthread_detach(hilo_io);

    // El Kernel sigue ejecutando otras cosas mientras estos hilos aceptan clientes, cada uno tiene un bucle esperando muchas instancias
}

void conectar_con_memoria() { //ESTO ES DE PRUEBA, AHORA SI ANDA la conexion, PERO AL SER EFIMERA esta conexion la debemos de hacer en cada interaccion con memoria
 // Por lo tanto podriamos usar esta funcion como handshake pero a la vez englobarlo con otro para crear conexion y enviar el paquete deseado
 
    char* puerto_memoria = string_itoa(configKERNEL.puerto_memoria);
    printf("IP_MEMORIA = %s\n", configKERNEL.ip_memoria);
    printf("PUERTO_MEMORIA = %s\n", puerto_memoria);

    int socket_memoria = crearConexion(configKERNEL.ip_memoria, puerto_memoria, logger);

    free(puerto_memoria);

    if (socket_memoria < 0) {
        log_error(logger, "Error al conectar con Memoria.");
        exit(EXIT_FAILURE);
    }
    log_info(logger, "Conexion establecida con Memoria.");

    enviar_handshake(socket_memoria, MODULO_KERNEL);
    log_info(logger, "Handshake enviado a Memoria.");

    close(socket_memoria);
}


void operarDispatch(int socket_cliente) {
    log_info(logger, "Manejando conexion DISPATCH");
}

void operarInterrupt(int socket_cliente) {
    log_info(logger, "Manejando conexion INTERRUPT");
}

void operarIo(int socket_cliente) {
    log_info(logger, "Manejando conexion IO");
}