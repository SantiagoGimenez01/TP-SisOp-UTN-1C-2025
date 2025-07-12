#include "swapDump.h"
static int slot_a_verificar;

bool generar_dump(t_proceso_en_memoria* proceso) {
    // obtener timestamp
    time_t timestamp = time(NULL);
    char nombre_archivo[256];
    snprintf(nombre_archivo, sizeof(nombre_archivo), "%s/%d-%ld.dmp", configMEMORIA.dump_path, proceso->pid, timestamp);

    FILE* archivo = fopen(nombre_archivo, "wb");
    if (!archivo) {
        log_error(logger, "No se pudo crear archivo de dump: %s", nombre_archivo);
        return false; 
    }

    // contenido de memoria del proceso
    int paginas_escritas = 0;
    escribir_paginas_recursivamente(proceso->tabla_nivel_1, 1, archivo, &paginas_escritas);

    fclose(archivo);
    log_debug(logger, "Dump completado para PID %d: %d paginas copiadas a %s", proceso->pid, paginas_escritas, nombre_archivo);
    return true;
}


void escribir_paginas_recursivamente(t_tabla_nivel* tabla, int nivel_actual, FILE* archivo, int* paginas_escritas) {
    if (!tabla) {
        log_error(logger, "Tabla NULL en nivel %d", nivel_actual);
        return;
    }

    for (int i = 0; i < configMEMORIA.entradas_por_tabla; i++) {
        if (nivel_actual == configMEMORIA.cantidad_niveles) {
            t_entrada_pagina* entrada = tabla->entradas[i];

            if (entrada == NULL) {
                log_trace(logger, "Entrada NULL en nivel %d, indice %d", nivel_actual, i);
                continue;
            }

            if (entrada->presencia) {
                void* origen = memoria_fisica + (entrada->marco * configMEMORIA.tam_pagina);
                fwrite(origen, 1, configMEMORIA.tam_pagina, archivo);
                (*paginas_escritas)++;
            }
        } else {
            if (tabla->entradas[i] == NULL) {
                log_trace(logger, "Entrada NULL en nivel %d, indice %d", nivel_actual, i);
                continue;
            }
            escribir_paginas_recursivamente((t_tabla_nivel*) tabla->entradas[i], nivel_actual + 1, archivo, paginas_escritas);
        }
    }
}



bool slot_ocupado(void* elemento) {
    t_registro_swap* reg = (t_registro_swap*) elemento;
    return reg->slot == slot_a_verificar;
}

int obtener_slot_libre() {
    for (int i = 0;; i++) {
        slot_a_verificar = i;
        if (!list_any_satisfy(paginas_en_swap, slot_ocupado)) {
            return i;
        }
    }
}


void suspender_proceso(t_proceso_en_memoria* proceso) {
    FILE* swap = fopen(configMEMORIA.path_swapfile, "r+b");
    if (!swap) {
        log_error(logger, "No se pudo abrir el archivo swapfile.bin");
        return;
    }

    log_debug(logger, "Iniciando suspension de PID %d", proceso->pid);

    suspender_paginas_recursivamente(proceso->tabla_nivel_1, 1, proceso, swap, 0);

    fclose(swap);

    usleep(configMEMORIA.retardo_swap * 1000);  // retardo en milisegundos
    log_debug(logger, "Proceso %d suspendido correctamente", proceso->pid);
}

void suspender_paginas_recursivamente(t_tabla_nivel* tabla, int nivel_actual, t_proceso_en_memoria* proceso, FILE* swap, int base_logica) {
    for (int i = 0; i < configMEMORIA.entradas_por_tabla; i++) {
        if (!tabla || !tabla->entradas[i]) {
            log_trace(logger, "Entrada NULL en nivel %d, indice %d", nivel_actual, i);
            continue;
        }

        // Cálculo de número de página lógica global
        int nro_pagina = base_logica + i * pow(configMEMORIA.entradas_por_tabla, configMEMORIA.cantidad_niveles - nivel_actual);

        if (nivel_actual == configMEMORIA.cantidad_niveles) {
            t_entrada_pagina* entrada = tabla->entradas[i];

            if (entrada->presencia) {
                if (entrada->marco < 0 || entrada->marco >= cantidad_frames) {
                    log_error(logger, "ERROR: Marco inválido (%d) en entrada [%d] del nivel %d", entrada->marco, i, nivel_actual);
                    continue;
                }

                int slot = obtener_slot_libre();
                if (slot == -1) {
                    log_error(logger, "ERROR: No hay slots libres en SWAP para PID %d", proceso->pid);
                    continue;
                }

                int offset = slot * configMEMORIA.tam_pagina;
                void* origen = memoria_fisica + (entrada->marco * configMEMORIA.tam_pagina);

                log_trace(logger, "Copiando página %d del PID %d al slot %d (offset %d)", nro_pagina, proceso->pid, slot, offset);
                fseek(swap, offset, SEEK_SET);
                fwrite(origen, 1, configMEMORIA.tam_pagina, swap);

                t_registro_swap* reg = malloc(sizeof(t_registro_swap));
                reg->pid = proceso->pid;
                reg->nro_pagina = nro_pagina;
                reg->slot = slot;
                list_add(paginas_en_swap, reg);

                bitarray_clean_bit(bitmap_frames, entrada->marco);
                entrada->presencia = false;
                entrada->marco = -1;

                proceso->metricas.bajadas_a_swap++;
            }

        } else {
            suspender_paginas_recursivamente((t_tabla_nivel*) tabla->entradas[i], nivel_actual + 1, proceso, swap, nro_pagina);
        }
    }
}





int contar_paginas_en_swap(int pid) {
    int contador = 0;
    for (int i = 0; i < list_size(paginas_en_swap); i++) {
        t_registro_swap* r = list_get(paginas_en_swap, i);
        if (r->pid == pid) contador++;
    }
    return contador;
}

int desuspender_proceso(t_proceso_en_memoria* proceso) {
    int paginas_a_cargar = contar_paginas_en_swap(proceso->pid);
    int marcos_libres = contar_marcos_libres();

    if (marcos_libres < paginas_a_cargar) {
        return -1; // No hay espacio suficiente
    }

    FILE* swap = fopen(configMEMORIA.path_swapfile, "r+b");
    if (!swap) {
        log_error(logger, "No se pudo abrir el archivo swapfile.bin");
        return -1;
    }

    for (int i = 0; i < list_size(paginas_en_swap);) {
        t_registro_swap* r = list_get(paginas_en_swap, i);

        if (r->pid == proceso->pid) {
            
            int marco = buscar_frame_libre();
            log_trace(logger, "Restaurando página %d del PID %d desde slot %d al marco %d",
          r->nro_pagina, r->pid, r->slot, marco);

            int offset = r->slot * configMEMORIA.tam_pagina;

            void* destino = memoria_fisica + (marco * configMEMORIA.tam_pagina);
            fseek(swap, offset, SEEK_SET);
            fread(destino, 1, configMEMORIA.tam_pagina, swap);
            
            t_entrada_pagina* entrada = buscar_entrada_pagina(proceso->tabla_nivel_1, r->nro_pagina);
            entrada->marco = marco;
            entrada->presencia = true;
            entrada->uso = false;
            entrada->modificado = false;

            proceso->metricas.subidas_de_swap++;
            
            log_trace(logger, "Entrada actualizada: nro_pagina=%d marco=%d presencia=%d",
          r->nro_pagina, entrada->marco, entrada->presencia);

            list_remove_and_destroy_element(paginas_en_swap, i, free);
        } else {
            i++;
        }
    }

    fclose(swap);
    usleep(configMEMORIA.retardo_swap * 1000);
    log_trace(logger, "PID %d - Total de páginas restauradas desde swap: %d",
          proceso->pid, paginas_a_cargar);

    return 0;
}

t_entrada_pagina* buscar_entrada_pagina(t_tabla_nivel* tabla, int nro_pagina) {
    int niveles = configMEMORIA.cantidad_niveles;
    int entradas = configMEMORIA.entradas_por_tabla;

    t_tabla_nivel* actual = tabla;

    for (int nivel = 1; nivel < niveles; nivel++) {
        int indice = (nro_pagina / (int)pow(entradas, niveles - nivel)) % entradas;
        actual = (t_tabla_nivel*) actual->entradas[indice];
    }

    int offset = nro_pagina % entradas;
    log_trace(logger, "Buscar entrada: - Página %d encontrada en nivel final. Retorna marco actual: %d",
           nro_pagina, ((t_entrada_pagina*) actual->entradas[offset])->marco);

    return (t_entrada_pagina*) actual->entradas[offset];
}


int contar_marcos_libres() {
    int disponibles = 0;
    for (int i = 0; i < bitarray_get_max_bit(bitmap_frames); i++) {
        if (!bitarray_test_bit(bitmap_frames, i))
            disponibles++;
    }
    return disponibles;
}
