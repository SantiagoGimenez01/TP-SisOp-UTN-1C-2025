#include "componentes.h"

t_list* tlb = NULL;
t_list* cache_paginas = NULL;
int puntero_clock = 0;
int puntero_clock_m = 0;


t_direccion_fisica* traducir_direccion_logica(int direccion_logica) {
    t_direccion_fisica* dir_fisica = malloc(sizeof(t_direccion_fisica));
    dir_fisica->entradas_niveles = malloc(sizeof(int) * cantidad_niveles);

    dir_fisica->numero_pagina = direccion_logica / tam_pagina;

    for (uint32_t nivel = 0; nivel < cantidad_niveles; nivel++) {
        uint32_t exponente = cantidad_niveles - (nivel + 1);
        uint32_t divisor = pow(cant_entradas_por_tabla, exponente);
        dir_fisica->entradas_niveles[nivel] = (dir_fisica->numero_pagina / divisor) % cant_entradas_por_tabla;
    }

    dir_fisica->desplazamiento = direccion_logica % tam_pagina;

    return dir_fisica;
}



t_entrada_tlb* buscar_en_tlb(uint32_t nro_pagina) {
    for (int i = 0; i < list_size(tlb); i++) {
        t_entrada_tlb* entrada = list_get(tlb, i);
        if (entrada->nro_pagina == nro_pagina) {
            return entrada;
        }
    }
    return NULL;
}

void agregar_a_tlb(uint32_t nro_pagina, uint32_t marco) {

    if (list_size(tlb) == configCPU.entradas_tlb) {
        if (strcmp(configCPU.reemplazo_tlb, "FIFO") == 0){
            reemplazar_tlb_fifo(nro_pagina, marco);
            return;
        }
        else{
            reemplazar_tlb_lru(nro_pagina, marco);
            return;
        }
    } else {
        t_entrada_tlb* nueva = malloc(sizeof(t_entrada_tlb));
        nueva->nro_pagina = nro_pagina;
        nueva->marco = marco;
        nueva->ultima_uso = get_timestamp();
        list_add(tlb, nueva);
    }
}

void reemplazar_tlb_fifo(uint32_t nro_pagina, uint32_t marco) {
    if (list_size(tlb) == 0) return; 

    t_entrada_tlb* victima = list_remove(tlb, 0);
    log_info(logger, "TLB reemplazo FIFO: se reemplazo la pagina %d", victima->nro_pagina);

    free(victima);

    t_entrada_tlb* nueva = malloc(sizeof(t_entrada_tlb));
    nueva->nro_pagina = nro_pagina;
    nueva->marco = marco;
    nueva->ultima_uso = 0; 
    list_add(tlb, nueva);
}


void reemplazar_tlb_lru(uint32_t nro_pagina, int marco) {
    t_entrada_tlb* victima = NULL;
    uint64_t min_tiempo = UINT64_MAX;

    for (int i = 0; i < list_size(tlb); i++) {
        t_entrada_tlb* entrada = list_get(tlb, i);
        if (entrada->ultima_uso < min_tiempo) {
            min_tiempo = entrada->ultima_uso;
            victima = entrada;
        }
    }
    log_info(logger, "TLB reemplazo LRU: se reemplazo la pagina %d", victima->nro_pagina);

    victima->nro_pagina = nro_pagina;
    victima->marco = marco;
    victima->ultima_uso = get_timestamp();
}



char* extraer_fragmento_con_desplazamiento(char* contenido, uint32_t desplazamiento, int tamanio) {
    char* fragmento = malloc(tamanio + 1);
    memcpy(fragmento, contenido + desplazamiento, tamanio);
    fragmento[tamanio] = '\0';
    return fragmento;
}

t_entrada_cache* buscar_en_cache(uint32_t nro_pagina) {
    for (int i = 0; i < list_size(cache_paginas); i++) {
        t_entrada_cache* entrada = list_get(cache_paginas, i);
        if (entrada->nro_pagina == nro_pagina) {
            entrada->uso = true; // clock
            return entrada;
        }
    }
    return NULL;
}


void agregar_a_cache(uint32_t pid, t_direccion_fisica* dir, uint32_t marco, char* contenido) {
    
    if (list_size(cache_paginas) == configCPU.entradas_cache) {
        if (strcmp(configCPU.reemplazo_cache, "CLOCK") == 0){
            reemplazar_cache_clock(pid, dir, marco, contenido);
            return;
        }
        else{
            reemplazar_cache_clock_m(pid, dir, marco, contenido);
            return;
        }
    }
    else{

        t_entrada_cache* nueva = malloc(sizeof(t_entrada_cache));
        nueva->nro_pagina = dir->numero_pagina;
        nueva->contenido = contenido;
        nueva->uso = true;
        nueva->modificado = false;
        nueva->marco = marco;

        list_add(cache_paginas, nueva);
    }
}

void escribir_en_cache(uint32_t nro_pagina, uint32_t desplazamiento, const char* datos) {
    t_entrada_cache* entrada = buscar_en_cache(nro_pagina);
    if (entrada == NULL) {
        log_error(logger, "ERROR: intento de escribir en pagina no presente en cache.");
        return;
    }

    memcpy(entrada->contenido + desplazamiento, datos, strlen(datos));  
    entrada->modificado = true;
    entrada->uso = true;
}


void escribir_fragmento_en_pagina(char* pagina_completa, uint32_t desplazamiento, const char* datos) {
    // Sobrescribe a partir del desplazamiento con los datos dados
    int len_datos = strlen(datos);

    // Asegurar que no se escriba mas alla
    if (desplazamiento + len_datos > tam_pagina) {
        len_datos = tam_pagina - desplazamiento;
    }

    memcpy(pagina_completa + desplazamiento, datos, len_datos);
}

void marcar_modificada_en_cache(uint32_t nro_pagina) {
    for (int i = 0; i < list_size(cache_paginas); i++) {
        t_entrada_cache* entrada = list_get(cache_paginas, i);
        if (entrada->nro_pagina == nro_pagina) {
            entrada->modificado = true;
            return;
        }
    }
}


void reemplazar_cache_clock_m(uint32_t pid, t_direccion_fisica* dir, uint32_t marco, char* contenido) {
    int size = list_size(cache_paginas);
    int iteraciones = 0;
    bool reemplazo_realizado = false;

    // Primera pasada: uso == 0, modificado == 0
    while (iteraciones < size) {
        t_entrada_cache* actual = list_get(cache_paginas, puntero_clock_m);
        if (!actual->uso && !actual->modificado) {
            reemplazo_realizado = true;
            break;
        }
        puntero_clock_m = (puntero_clock_m + 1) % size;
        iteraciones++;
    }

    // Segunda pasada: uso == 0, modificado == 1
    if (!reemplazo_realizado) {
        iteraciones = 0;
        while (iteraciones < size) {
            t_entrada_cache* actual = list_get(cache_paginas, puntero_clock_m);
            if (!actual->uso && actual->modificado) {
                reemplazo_realizado = true;
                break;
            }
            actual->uso = false;  // Preparar para siguiente ciclo
            puntero_clock_m = (puntero_clock_m + 1) % size;
            iteraciones++;
        }
    }

    // Reemplazo final
    t_entrada_cache* victima = list_get(cache_paginas, puntero_clock_m);
    if (victima->modificado) {
        escribir_en_memoria(pid, dir, victima->marco, victima->contenido);
        log_info(logger, "PID: %d - Pagina Actualizada de Cache a Memoria - Página: %d - Frame: %d", pid, victima->nro_pagina, victima->marco);
    }

    free(victima->contenido);
    victima->nro_pagina = dir->numero_pagina;
    victima->contenido = contenido;
    victima->uso = true;
    victima->modificado = false;
    victima->marco = marco;

    puntero_clock_m = (puntero_clock_m + 1) % size;
}

void reemplazar_cache_clock(uint32_t pid, t_direccion_fisica* dir, uint32_t marco, char* contenido) {
    int size = list_size(cache_paginas);
    int reemplazada = 0;

    while (!reemplazada) {
        t_entrada_cache* actual = list_get(cache_paginas, puntero_clock);

        if (!actual->uso) {
            if (actual->modificado) {
                escribir_en_memoria(pid, dir, actual->marco, actual->contenido);
                log_info(logger, "PID: %d - Pagina Actualizada de Cache a Memoria - Pagina: %d - Frame: %d", pid, actual->nro_pagina, actual->marco);
            }

            free(actual->contenido);
            actual->nro_pagina = dir->numero_pagina;
            actual->contenido = contenido;
            actual->uso = true;
            actual->modificado = false;
            actual->marco = marco;

            reemplazada = 1;
        } else {
            actual->uso = false;
        }

        puntero_clock = (puntero_clock + 1) % size;
    }
}

