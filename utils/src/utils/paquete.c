#include "paquete.h"

t_paquete* crear_paquete() {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
    return paquete;
}

void agregar_string_a_paquete(t_paquete* paquete, char* string) {
    uint32_t tamanio_string = strlen(string) + 1;
    uint32_t nuevo_tamanio = paquete->buffer->size + sizeof(uint32_t) + tamanio_string;

    paquete->buffer->stream = realloc(paquete->buffer->stream, nuevo_tamanio);

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio_string, sizeof(uint32_t));
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(uint32_t), string, tamanio_string);

    paquete->buffer->size = nuevo_tamanio;
}

void agregar_int_a_paquete(t_paquete* paquete, int valor) {
    uint32_t nuevo_tamanio = paquete->buffer->size + sizeof(int);
    paquete->buffer->stream = realloc(paquete->buffer->stream, nuevo_tamanio);

    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(int));

    paquete->buffer->size = nuevo_tamanio;
}

void enviar_paquete(t_paquete* paquete, int socket) {
    send(socket, &(paquete->buffer->size), sizeof(uint32_t), 0);               
    send(socket, paquete->buffer->stream, paquete->buffer->size, 0);           
}

void eliminar_paquete(t_paquete* paquete) {
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

t_paquete* recibir_paquete(int socket_cliente) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));

    recv(socket_cliente, &(paquete->buffer->size), sizeof(uint32_t), MSG_WAITALL);

    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);

    return paquete;
}

// ESTO POR AHORA ANDA, tanto RECIBIR y ENVIAR. Pero a grandes rasgos para cuando enviemos cosas serializadas lo tenemos que modificar


char* recibir_string_de_paquete(t_paquete* paquete) {
    uint32_t longitud;
    int desplazamiento = 0;

    memcpy(&longitud, paquete->buffer->stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    char* str = malloc(longitud + 1); 
    memcpy(str, paquete->buffer->stream + desplazamiento, longitud);

    str[longitud] = '\0';

    return str;
}

char* recibir_string_de_paquete_con_offset(t_paquete* paquete, int* offset) {
    uint32_t longitud;
    memcpy(&longitud, paquete->buffer->stream + *offset, sizeof(uint32_t));
    *offset += sizeof(uint32_t);

    char* str = malloc(longitud + 1);
    memcpy(str, paquete->buffer->stream + *offset, longitud);
    *offset += longitud;

    str[longitud] = '\0';
    return str;
}


/*char* recibir_string_de_paquete(t_paquete* paquete) {
    uint32_t size;
    memcpy(&size, paquete->buffer->stream, sizeof(uint32_t));

    char* str = malloc(size);
    memcpy(str, paquete->buffer->stream + sizeof(uint32_t), size);
    
    return str;
}*/


void enviar_opcode(t_opcode codigo, int socket) {
    send(socket, &codigo, sizeof(t_opcode), 0);
}

const char* nombre_syscall(t_instruccion_id id) {
    switch (id) {
        case IO: return "IO";
        case INIT_PROC: return "INIT_PROC";
        case DUMP_MEMORY: return "DUMP_MEMORY";
        case EXIT: return "EXIT";
        default: return "DESCONOCIDA";
    }
} // lo puse aca solo porque lo uso en 2 modulos

uint64_t get_timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000); // tremenda fruta pero vamos a ver que sale
}