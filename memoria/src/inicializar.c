#include "inicializar.h"


void inicializar_memoria() {
    cantidad_frames = configMEMORIA.tam_memoria / configMEMORIA.tam_pagina;

    int tamanio_bitmap_bytes = cantidad_frames / 8;
    char* bitmap_memoria = calloc(tamanio_bitmap_bytes, sizeof(char));  // todos los bits en 0

    bitmap_frames = bitarray_create_with_mode(bitmap_memoria, tamanio_bitmap_bytes, LSB_FIRST);

    memoria_fisica = calloc(configMEMORIA.tam_memoria, sizeof(char)); // simula la memoria

    log_info(logger, "Memoria inicializada con %d frames de %d bytes", cantidad_frames, configMEMORIA.tam_pagina);

    procesos_en_memoria = list_create();

}

t_tabla_nivel* crear_tabla_nivel(int nivel_actual, int nivel_maximo) {
    t_tabla_nivel* tabla = malloc(sizeof(t_tabla_nivel));
    tabla->entradas = malloc(sizeof(t_entrada_pagina*) * configMEMORIA.entradas_por_tabla);

    for (int i = 0; i < configMEMORIA.entradas_por_tabla; i++) {
        if (nivel_actual == nivel_maximo) {
            // ultimo nivel apuntan a marcos (inicialmente no asignados. TENGO QUE REPASAR)
            tabla->entradas[i] = malloc(sizeof(t_entrada_pagina));
            tabla->entradas[i]->marco = 0;       // No tiene marco aún
            tabla->entradas[i]->presencia = false;
            tabla->entradas[i]->uso = false;
            tabla->entradas[i]->modificado = false;
        } else {
            // Apuntan a otra tabla
            tabla->entradas[i] = (t_entrada_pagina*)crear_tabla_nivel(nivel_actual + 1, nivel_maximo);
        }
    }

    return tabla;
}

void crear_estructuras_para_proceso(uint32_t pid) {
    t_proceso_en_memoria* nuevo = malloc(sizeof(t_proceso_en_memoria));
    nuevo->pid = pid;
    nuevo->tabla_nivel_1 = crear_tabla_nivel(1, configMEMORIA.cantidad_niveles);
    list_add(procesos_en_memoria, nuevo);

    log_info(logger, "Se inicializaron las estructuras de tablas para el proceso %d", pid);
}

