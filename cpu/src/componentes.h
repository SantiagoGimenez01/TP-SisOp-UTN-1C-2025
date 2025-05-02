#ifndef COMPONENTES_H
#define COMPONENTES_H

#include "globales.h"
#include <math.h>


t_direccion_fisica* traducir_direccion_logica(int direccion_logica);
t_entrada_tlb* buscar_en_tlb(uint32_t nro_pagina);
void agregar_a_tlb(uint32_t nro_pagina, uint32_t marco);
char* pedir_contenido_de_pagina(uint32_t pid, uint32_t nro_pagina);
void escribir_en_memoria(uint32_t pid, uint32_t nro_pagina, uint32_t desplazamiento, char* nuevo_contenido);
char* extraer_fragmento_con_desplazamiento(char* contenido, uint32_t desplazamiento, int tamanio);
t_entrada_cache* buscar_en_cache(uint32_t nro_pagina);
void agregar_a_cache(uint32_t nro_pagina, char* contenido);
void escribir_en_cache(uint32_t nro_pagina, uint32_t desplazamiento, const char* datos);
uint32_t pedir_marco_a_memoria(int pid, t_direccion_fisica* dir);
char* pedir_fragmento_de_memoria(int pid, int marco, int desplazamiento, int tamanio);
void escribir_fragmento_en_pagina(char* pagina_completa, uint32_t desplazamiento, const char* datos);
void marcar_modificada_en_cache(uint32_t nro_pagina);

#endif