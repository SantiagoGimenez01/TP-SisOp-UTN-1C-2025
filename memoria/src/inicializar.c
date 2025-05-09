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

void crear_estructuras_para_proceso(uint32_t pid, char* nombre_archivo, int tamanio) {
    t_proceso_en_memoria* nuevo = malloc(sizeof(t_proceso_en_memoria));
    nuevo->pid = pid;
    nuevo->tamanio = tamanio;
    nuevo->nombre_archivo = strdup(nombre_archivo); 

    nuevo->tabla_nivel_1 = crear_tabla_nivel(1, configMEMORIA.cantidad_niveles);

    // Inicializar y cargar las instrucciones directamente
    nuevo->instrucciones = list_create();
    cargar_instrucciones(nuevo->instrucciones, nombre_archivo);  

    list_add(procesos_en_memoria, nuevo);

    log_info(logger, "Se inicializaron estructuras y cargaron instrucciones para el proceso %d", pid);
}


void cargar_instrucciones(t_list* lista_instrucciones, char* nombre_archivo) {
    char ruta_archivo[256];
    snprintf(ruta_archivo, sizeof(ruta_archivo), "%s/%s.txt", configMEMORIA.pseudocodigoPath, nombre_archivo);

    FILE* archivo = fopen(ruta_archivo, "r");
    if (!archivo) {
        log_error(logger, "No se pudo abrir el archivo de pseudocodigo: %s", ruta_archivo);
        return;
    }

    char* linea = NULL;
    size_t len = 0;
    ssize_t read;
    int numero_linea = 0;

    while ((read = getline(&linea, &len, archivo)) != -1) {
        linea[strcspn(linea, "\n")] = '\0';  // remover \n
        list_add(lista_instrucciones, strdup(linea));
        log_trace(logger, "Instruccion [%d]: %s", numero_linea++, linea);
    }

    free(linea);
    fclose(archivo);
}



