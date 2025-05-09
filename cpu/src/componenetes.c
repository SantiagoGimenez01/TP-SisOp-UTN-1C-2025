#include "componentes.h"

t_list* tlb = NULL;
t_list* cache_paginas = NULL;

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
        //reemplazar_entrada_tlb(nro_pagina, marco); // desarrollo
    } else {
        t_entrada_tlb* nueva = malloc(sizeof(t_entrada_tlb));
        nueva->nro_pagina = nro_pagina;
        nueva->marco = marco;
        list_add(tlb, nueva);
    }
}

char* pedir_contenido_de_pagina(uint32_t pid, uint32_t marco) {
    enviar_opcode(PEDIR_PAGINA_COMPLETA, socket_memoria);
    t_paquete* paquete = crear_paquete();
    agregar_int_a_paquete(paquete, pid);
    agregar_int_a_paquete(paquete, marco);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    t_paquete* respuesta = recibir_paquete(socket_memoria);
    char* contenido = recibir_string_de_paquete(respuesta);
    eliminar_paquete(respuesta);
    return contenido;
}

void escribir_en_memoria(uint32_t pid, uint32_t nro_pagina, uint32_t desplazamiento, char* nuevo_contenido) {
    enviar_opcode(ESCRIBIR_PAGINA, socket_memoria);
    t_paquete* paquete = crear_paquete();
    agregar_int_a_paquete(paquete, pid);
    agregar_int_a_paquete(paquete, nro_pagina);
    agregar_int_a_paquete(paquete, desplazamiento);
    agregar_string_a_paquete(paquete, nuevo_contenido);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
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


void agregar_a_cache(uint32_t nro_pagina, char* contenido) {
    
    if (list_size(cache_paginas) == configCPU.entradas_cache) {
        //reemplazar_en_cache();  // desarollo
    }

    t_entrada_cache* nueva = malloc(sizeof(t_entrada_cache));
    nueva->nro_pagina = nro_pagina;
    nueva->contenido = strdup(contenido);
    nueva->uso = true;
    nueva->modificado = false;

    list_add(cache_paginas, nueva);
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


uint32_t pedir_marco_a_memoria(int pid, t_direccion_fisica* dir) {
    enviar_opcode(PEDIR_MARCO, socket_memoria);

    t_paquete* paquete = crear_paquete();
    agregar_int_a_paquete(paquete, pid);

    for (int i = 0; i < cantidad_niveles; i++) {
        agregar_int_a_paquete(paquete, dir->entradas_niveles[i]);
    }

    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    uint32_t marco;
    recv(socket_memoria, &marco, sizeof(int), MSG_WAITALL);
    return marco;
}

char* pedir_fragmento_de_memoria(int pid, int marco, int desplazamiento, int tamanio) {
    enviar_opcode(LEER_PAGINA, socket_memoria);

    t_paquete* paquete = crear_paquete();
    agregar_int_a_paquete(paquete, pid);
    agregar_int_a_paquete(paquete, marco);
    agregar_int_a_paquete(paquete, desplazamiento);
    agregar_int_a_paquete(paquete, tamanio);

    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    t_paquete* recibido = recibir_paquete(socket_memoria);
    char* resultado = recibir_string_de_paquete(recibido);
    eliminar_paquete(recibido);
    return resultado;
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
