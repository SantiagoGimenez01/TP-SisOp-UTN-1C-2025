#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H

#include "Instrucciones.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "utils/paquete.h"     
#include "utils/structs.h"     
#include "componentes.h"  



// Ciclo completo de instrucciones
void ejecutar_ciclo(uint32_t pid, uint32_t pc, uint32_t* timer_exec);

// Fases del ciclo
t_instruccion_id obtener_id_instruccion(char* nombre);
t_instruccion* decode_instruccion(char* linea);
bool ejecutar_instruccion(t_instruccion* inst, uint32_t pid, uint32_t* pc);
void liberar_instruccion(t_instruccion* inst);

#endif 
