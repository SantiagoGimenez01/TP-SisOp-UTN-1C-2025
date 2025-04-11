#ifndef SERIALIZACION_H
#define SERIALIZACION_H

#include "./commons/commons.h"
#include "structs.h"

void* serializar_pcb(t_pcb* pcb, uint32_t* size);
t_pcb* deserializar_pcb(void* buffer);


#endif
