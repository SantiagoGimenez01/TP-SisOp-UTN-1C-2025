#include "tablaPaginas.h"

uint32_t obtener_marco(t_proceso_en_memoria* proceso, int* entradas_niveles) {
    t_tabla_nivel* actual = proceso->tabla_nivel_1;

    for (int i = 0; i < configMEMORIA.cantidad_niveles - 1; i++) {
        actual = (t_tabla_nivel*) actual->entradas[entradas_niveles[i]];
        proceso->metricas.accesos_tablas_paginas++;
        usleep(configMEMORIA.retardo_memoria * 1000);
    }

    t_entrada_pagina* entrada_final = actual->entradas[entradas_niveles[configMEMORIA.cantidad_niveles - 1]];
    proceso->metricas.accesos_tablas_paginas++;
    usleep(configMEMORIA.retardo_memoria * 1000);

    if (!entrada_final->presencia) {
        log_error(logger, "PID %d - Se intento acceder a una pagina no presente (pagina %d)", 
                  proceso->pid, entradas_niveles[configMEMORIA.cantidad_niveles - 1]);
        return -1;
    }

    return entrada_final->marco;
}


char* leer_fragmento(t_proceso_en_memoria* proceso, int marco, int desplazamiento, int tamanio) {
    
    void* base = memoria_fisica + marco * configMEMORIA.tam_pagina;

    char* fragmento = malloc(tamanio + 1); 
    memcpy(fragmento, base + desplazamiento, tamanio);
    fragmento[tamanio] = '\0';  

    proceso->metricas.lecturas_memoria++;

    log_info(logger, "## PID: %d - Lectura - Dir. Fisica: %d - Tamaño: %d", 
             proceso->pid, marco * configMEMORIA.tam_pagina + desplazamiento, tamanio);

    return fragmento;
}


char* leer_pagina(t_proceso_en_memoria* proceso, int marco) {
    
    void* origen = memoria_fisica + marco * configMEMORIA.tam_pagina;
    
    char* contenido = malloc(configMEMORIA.tam_pagina + 1);
    memcpy(contenido, origen, configMEMORIA.tam_pagina);
    contenido[configMEMORIA.tam_pagina] = '\0';

    proceso->metricas.lecturas_memoria++;

    log_info(logger, "## PID: %d - Lectura - Dir. Física: %d - Tamaño: %d",
             proceso->pid, marco * configMEMORIA.tam_pagina, configMEMORIA.tam_pagina);

    return contenido;
}

void escribir_en_pagina(t_proceso_en_memoria* proceso, uint32_t marco, uint32_t desplazamiento, char* datos) {
    
    char* destino = memoria_fisica + marco * configMEMORIA.tam_pagina + desplazamiento;

    memcpy(destino, datos, strlen(datos)); 

    proceso->metricas.escrituras_memoria++;

    log_info(logger, "## PID: %d - Escritura - Dir. Fisica: %u - Tamaño: %lu",
             proceso->pid, marco * configMEMORIA.tam_pagina + desplazamiento, strlen(datos));
}

void marcar_modificada(t_proceso_en_memoria* proceso, int* entradas_niveles) {
    t_tabla_nivel* actual = proceso->tabla_nivel_1;

    for (int i = 0; i < configMEMORIA.cantidad_niveles - 1; i++) {
        int idx = entradas_niveles[i];

        if (idx < 0 || idx >= configMEMORIA.entradas_por_tabla || actual->entradas[idx] == NULL) {
            log_error(logger, "Error al marcar pagina modificada: entrada invalida en nivel %d (idx=%d)", i + 1, idx);
            return;
        }

        actual = (t_tabla_nivel*)actual->entradas[idx];
    }

    int final_idx = entradas_niveles[configMEMORIA.cantidad_niveles - 1];
    if (final_idx < 0 || final_idx >= configMEMORIA.entradas_por_tabla || actual->entradas[final_idx] == NULL) {
        log_error(logger, "Error al marcar pagina modificada: entrada final invalida (idx=%d)", final_idx);
        return;
    }

    t_entrada_pagina* entrada = actual->entradas[final_idx];
    entrada->modificado = true;
}




bool hay_espacio_para(uint32_t paginas_requeridas) {
    uint32_t disponibles = 0;

    for (int i = 0; i < cantidad_frames; i++) {
        if (!bitarray_test_bit(bitmap_frames, i))
            disponibles++;
    }

    return disponibles >= paginas_requeridas;
}


t_proceso_en_memoria* buscar_proceso_por_pid(uint32_t pid) {
    for (int i = 0; i < list_size(procesos_en_memoria); i++) {
        t_proceso_en_memoria* p = list_get(procesos_en_memoria, i);
        if (p->pid == pid)
            return p;
    }
    return NULL;
}

