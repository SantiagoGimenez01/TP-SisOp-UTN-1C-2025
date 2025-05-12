#ifndef PETICIONES_H
#define PETICIONES_H
#include "globales.h"

char* pedir_contenido_de_pagina(uint32_t pid, uint32_t nro_pagina);
void escribir_en_memoria(uint32_t pid, t_direccion_fisica* dir, uint32_t marco, char* nuevo_contenido);
uint32_t pedir_marco_a_memoria(int pid, t_direccion_fisica* dir);
char* pedir_fragmento_de_memoria(int pid, int marco, int desplazamiento, int tamanio);
void actualizar_paginas_modificadas_en_memoria(uint32_t pid);
char* pedir_instruccion_a_memoria(uint32_t pid, uint32_t pc);
bool enviar_syscall_a_kernel(t_instruccion* inst, uint32_t pid, uint32_t pc);
void limpiar_tlb();
void limpiar_cache();

#endif