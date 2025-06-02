#ifndef SWAP_DUMP_H
#define SWAP_DUMP_H

#include "globales.h"
#include <math.h>
#include "inicializar.h"
// Dump
void generar_dump(t_proceso_en_memoria* proceso);
void escribir_paginas_recursivamente(t_tabla_nivel* tabla, int nivel_actual, FILE* archivo, int* paginas_escritas);

// Swap
void suspender_proceso(t_proceso_en_memoria* proceso);
void suspender_paginas_recursivamente(t_tabla_nivel* tabla, int nivel_actual, t_proceso_en_memoria* proceso, FILE* swap);

int desuspender_proceso(t_proceso_en_memoria* proceso);
int contar_paginas_en_swap(int pid);
int obtener_slot_libre();
bool slot_ocupado(void* elemento);
t_entrada_pagina* buscar_entrada_pagina(t_tabla_nivel* tabla, int nro_pagina);
int contar_marcos_libres();

#endif
