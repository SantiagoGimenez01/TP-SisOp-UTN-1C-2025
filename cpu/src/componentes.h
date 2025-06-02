#ifndef COMPONENTES_H
#define COMPONENTES_H

#include "peticiones.h"
#include <math.h>


t_direccion_fisica* traducir_direccion_logica(int direccion_logica);

t_entrada_tlb* buscar_en_tlb(uint32_t nro_pagina);
void agregar_a_tlb(uint32_t nro_pagina, uint32_t marco);
void reemplazar_tlb_fifo(uint32_t nro_pagina, uint32_t marco);
void reemplazar_tlb_lru(uint32_t nro_pagina, int marco);

t_entrada_cache* buscar_en_cache(uint32_t nro_pagina);
char* extraer_fragmento_con_desplazamiento(char* contenido, uint32_t desplazamiento, int tamanio);
void agregar_a_cache(uint32_t pid, t_direccion_fisica* dir, uint32_t marco, char* contenido);
void escribir_en_cache(uint32_t nro_pagina, uint32_t desplazamiento, const char* datos);
void escribir_fragmento_en_pagina(char* pagina_completa, uint32_t desplazamiento, const char* datos);
void marcar_modificada_en_cache(uint32_t nro_pagina);
void reemplazar_cache_clock_m(uint32_t pid, t_direccion_fisica* dir, uint32_t marco, char* contenido);
void reemplazar_cache_clock(uint32_t pid, t_direccion_fisica* dir, uint32_t marco, char* contenido);

#endif