#include "sockets.h"
#include "utils/libs/logger.h"
#include "utils/libs/config.h"
#include "utils/structs.h"
#include "conexiones.h"

#include <pthread.h>


void* atender_cliente(void* socket_cliente_void) {
    int socket_cliente = (intptr_t)socket_cliente_void;

    t_modulo modulo_origen;
    recv(socket_cliente, &modulo_origen, sizeof(t_modulo), 0);

    if (modulo_origen == MODULO_KERNEL) {
        log_info(logger, "Se conecto el Kernel (conexion efimera).");
        
        operarKernel(socket_cliente);// Atendemos la operacion de Kernel
        close(socket_cliente); // Kernel = conexión efimera (DUDAS, pero hay que probar)
    }
    else if (modulo_origen == MODULO_CPU_DISPATCH) {
        log_info(logger, "Se conecto una CPU.");
        
        operarCPU(socket_cliente);// TENGO DUDAS CON ESTO (PARA MI)
        
    }
    else {
        log_warning(logger, "Se conecto un modulo desconocido.");
        close(socket_cliente);
    }

    return NULL;
}

void establecerConexiones() {
    int socket_servidor = iniciarServidor(string_itoa(configMEMORIA.puerto_escucha), logger, "MEMORIA");

    log_info(logger, "Servidor MEMORIA escuchando conexiones.");

    while (1) {
        int socket_cliente = esperarCliente(socket_servidor, logger);

        pthread_t hilo_cliente;
        pthread_create(&hilo_cliente, NULL, atender_cliente, (void*)(intptr_t)socket_cliente);
        pthread_detach(hilo_cliente); 
    }
}


void operarKernel(int socket_cliente) {
    log_info(logger, "Manejando operacion kernel...");
}

void operarCPU(int socket_cliente) {
    log_info(logger, "Manejando operación de CPU con Memoria...");
   while (1) {
        // Acá va la logica de recibir una operacion de Memoria o de enviar pedidos
        
    }
}