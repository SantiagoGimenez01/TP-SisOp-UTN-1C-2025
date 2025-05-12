#ifndef TABLAPLAGINAS_H
#define TABLAPLAGINAS_H

#include "globales.h"

uint32_t obtener_marco(t_proceso_en_memoria* proceso, int* entradas_niveles);
int buscar_frame_libre();
char* leer_fragmento(t_proceso_en_memoria* proceso, int marco, int desplazamiento, int tamanio);
char* leer_pagina(t_proceso_en_memoria* proceso, int marco);
void escribir_en_pagina(t_proceso_en_memoria* proceso, uint32_t marco, uint32_t desplazamiento, char* datos);
void marcar_modificada(t_proceso_en_memoria* proceso, int* entradas_niveles);
bool hay_espacio_para(uint32_t paginas_requeridas);
t_proceso_en_memoria* buscar_proceso_por_pid(uint32_t pid);
#endif
