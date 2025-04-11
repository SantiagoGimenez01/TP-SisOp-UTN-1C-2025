#include "serializacion.h"

void* serializar_pcb(t_pcb* pcb, uint32_t* size) {
    
    *size = sizeof(uint32_t) * 3 + sizeof(double) * 2 + sizeof(uint32_t); 
    void* buffer = malloc(*size);
    int offset = 0;
    memcpy(buffer + offset, &(pcb->pid), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer + offset, &(pcb->pc), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer + offset, &(pcb->tablaPaginas), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer + offset, &(pcb->estRafagaAnt), sizeof(double));
    offset += sizeof(double);
    memcpy(buffer + offset, &(pcb->realRafaga), sizeof(double));
    offset += sizeof(double);
    memcpy(buffer + offset, &(pcb->tamanio), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    return buffer;
}

t_pcb* deserializar_pcb(void* buffer) {
    t_pcb* pcb = malloc(sizeof(t_pcb));
    int offset = 0;

    memcpy(&(pcb->pid), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&(pcb->pc), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&(pcb->tablaPaginas), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&(pcb->estRafagaAnt), buffer + offset, sizeof(double));
    offset += sizeof(double);
    memcpy(&(pcb->realRafaga), buffer + offset, sizeof(double));
    offset += sizeof(double);
    memcpy(&(pcb->tamanio), buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    pcb->metricas_estado = NULL;
    pcb->metricas_tiempo = NULL;

    return pcb;
}
