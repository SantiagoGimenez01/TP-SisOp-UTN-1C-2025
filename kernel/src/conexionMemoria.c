#include "conexionMemoria.h"

int conectar_con_memoria() {
    char* puerto_memoria = string_itoa(configKERNEL.puerto_memoria);
    int socket_memoria = crearConexion(configKERNEL.ip_memoria, puerto_memoria, logger);
    free(puerto_memoria);

    if (socket_memoria < 0) {
        log_error(logger, "No se pudo conectar a Memoria");
        return -1;
    }

    enviar_handshake(socket_memoria, MODULO_KERNEL);
    return socket_memoria;
}

bool solicitar_espacio_a_memoria(t_pcb* pcb) {
    int socket_memoria = conectar_con_memoria(); 
    if (socket_memoria < 0) {
        log_error(logger, "No se pudo conectar a Memoria para INICIAR PROCESO %d", pcb->pid);
        return false;
    }

    enviar_opcode(INICIAR_PROCESO, socket_memoria);

    t_paquete* paquete = crear_paquete();
    agregar_int_a_paquete(paquete, pcb->pid);
    agregar_int_a_paquete(paquete, pcb->tamanio);
    agregar_string_a_paquete(paquete, pcb->archivo_pseudocodigo);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    // Esperamos respuesta de MEMORIA
    int cod_respuesta;
    recv(socket_memoria, &cod_respuesta, sizeof(int), MSG_WAITALL);

    bool aceptado = false;

    if (cod_respuesta == RESPUESTA_OK) {
        aceptado = true;
    }

    close(socket_memoria);
    return aceptado;
}

bool solicitar_dump_a_memoria(uint32_t pid) {
    int socket_memoria = conectar_con_memoria();  
    if (socket_memoria < 0) {
        log_error(logger, "No se pudo conectar a Memoria para DUMP MEMORY %d", pid);
        return false;
    }

    enviar_opcode(DUMP_MEMORY, socket_memoria);

    t_paquete* paquete = crear_paquete();
    agregar_int_a_paquete(paquete, pid);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    int resultado;
    recv(socket_memoria, &resultado, sizeof(int), MSG_WAITALL);
    close(socket_memoria);

    return resultado == RESPUESTA_OK;
}

bool liberar_en_memoria(uint32_t pid) {
    int socket_memoria = conectar_con_memoria();
    if (socket_memoria < 0) {
        log_error(logger, "No se pudo conectar a Memoria para FINALIZAR PROCESO %d", pid);
        return false;
    }

    enviar_opcode(FINALIZAR_PROCESO, socket_memoria);

    t_paquete* paquete = crear_paquete();
    agregar_int_a_paquete(paquete, pid);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    int respuesta;
    recv(socket_memoria, &respuesta, sizeof(int), MSG_WAITALL);
    close(socket_memoria);

    return respuesta == RESPUESTA_OK;
}
