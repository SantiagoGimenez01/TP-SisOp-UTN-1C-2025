#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "utils/paquete.h"     
#include "utils/structs.h"     
#include "globales.h"  

typedef struct {
    t_instruccion_id id;
    char** parametros;          // Vector de strings
    int cantidad_parametros;  
} t_instruccion;

// Ciclo completo de instrucciones
void ejecutar_ciclo(uint32_t pid, uint32_t pc);

// Fases del ciclo
char* pedir_instruccion_a_memoria(uint32_t pid, uint32_t pc);
t_instruccion_id obtener_id_instruccion(char* nombre);
t_instruccion* decode_instruccion(char* linea);
bool ejecutar_instruccion(t_instruccion* inst, uint32_t pid, uint32_t* pc);
void enviar_syscall_a_kernel(t_instruccion* inst, uint32_t pid, uint32_t pc);
const char* nombre_syscall(t_instruccion_id id);
void liberar_instruccion(t_instruccion* inst);

#endif 
