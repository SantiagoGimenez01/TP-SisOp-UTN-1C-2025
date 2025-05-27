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

    return tabla;
}

void asignar_marcos_a_paginas(t_tabla_nivel* tabla, int nivel_actual, int nivel_maximo, int* paginas_asignadas, int total_paginas) {
    for (int i = 0; i < configMEMORIA.entradas_por_tabla && *paginas_asignadas < total_paginas; i++) {
        if (nivel_actual == nivel_maximo) {
            tabla->entradas[i] = malloc(sizeof(t_entrada_pagina));
            int frame = buscar_frame_libre();
            tabla->entradas[i]->marco = frame;
            tabla->entradas[i]->presencia = true;
            tabla->entradas[i]->uso = false;
            tabla->entradas[i]->modificado = false;
            (*paginas_asignadas)++;
        } else {
            tabla->entradas[i] = (t_entrada_pagina*) crear_tabla_nivel(nivel_actual + 1, nivel_maximo);
            asignar_marcos_a_paginas((t_tabla_nivel*)tabla->entradas[i], nivel_actual + 1, nivel_maximo, paginas_asignadas, total_paginas);
        }
    }
}

int buscar_frame_libre() {
    for (int i = 0; i < bitarray_get_max_bit(bitmap_frames); i++) {
        if (!bitarray_test_bit(bitmap_frames, i)) {
            bitarray_set_bit(bitmap_frames, i);
            return i;
        }
    }
    return -1; // No deberia pasar si ya se verifico antes
}

void crear_estructuras_para_proceso(uint32_t pid, char* nombre_archivo, int tamanio, uint32_t paginas_necesarias) {
    t_proceso_en_memoria* nuevo = malloc(sizeof(t_proceso_en_memoria));
    nuevo->pid = pid;
    nuevo->tamanio = tamanio;
    nuevo->nombre_archivo = strdup(nombre_archivo); 
    nuevo->paginas_necesarias = paginas_necesarias;
    


    nuevo->metricas.accesos_tablas_paginas = 0;
    nuevo->metricas.instrucciones_solicitadas = 0;
    nuevo->metricas.bajadas_a_swap = 0;
    nuevo->metricas.subidas_de_swap = 0;
    nuevo->metricas.lecturas_memoria = 0;
    nuevo->metricas.escrituras_memoria = 0;

    nuevo->tabla_nivel_1 = crear_tabla_nivel(1, configMEMORIA.cantidad_niveles);

    int paginas_asignadas = 0;
    asignar_marcos_a_paginas(nuevo->tabla_nivel_1, 1, configMEMORIA.cantidad_niveles, &paginas_asignadas, paginas_necesarias);

    // Inicializar y cargar las instrucciones directamente
    nuevo->instrucciones = list_create();
    cargar_instrucciones(nuevo->instrucciones, nombre_archivo);  

    list_add(procesos_en_memoria, nuevo);

    log_info(logger, "Se inicializaron estructuras, se asignaron marcos y se cargaron instrucciones para el proceso %d", pid);
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



void log_metricas_proceso(t_proceso_en_memoria* proceso) {
    log_info(logger, "## PID: %d - Proceso Destruido - Metricas - Acc.T.Pag: %d; Inst.Sol.: %d; SWAP: %d; Mem.Prin.: %d; Lec.Mem.: %d; Esc.Mem.: %d",
        proceso->pid,
        proceso->metricas.accesos_tablas_paginas,
        proceso->metricas.instrucciones_solicitadas,
        proceso->metricas.bajadas_a_swap,
        proceso->metricas.subidas_de_swap,
        proceso->metricas.lecturas_memoria,
        proceso->metricas.escrituras_memoria);
}

void liberar_marcos_de_proceso(t_tabla_nivel* tabla, int nivel_actual) {
    for (int i = 0; i < configMEMORIA.entradas_por_tabla; i++) {
        if (nivel_actual == configMEMORIA.cantidad_niveles) {
            t_entrada_pagina* entrada = tabla->entradas[i];
            if (entrada->presencia) {
                bitarray_clean_bit(bitmap_frames, entrada->marco);
            }
            free(entrada);
        } else {
            liberar_marcos_de_proceso((t_tabla_nivel*) tabla->entradas[i], nivel_actual + 1);
        }
    }
    free(tabla->entradas);
    free(tabla);
}

void liberar_proceso_en_memoria(t_proceso_en_memoria* proceso) {
    log_info(logger, "----Todavia no libero nada----");
    liberar_marcos_de_proceso(proceso->tabla_nivel_1, 1);
    log_info(logger, "----Libero los marcos del proceso----");
    list_destroy_and_destroy_elements(proceso->instrucciones, free);
    free(proceso->nombre_archivo);
    list_remove_element(procesos_en_memoria, proceso); // elimina de la lista global
    free(proceso);

    log_info(logger, "----Libero todo----");
}
