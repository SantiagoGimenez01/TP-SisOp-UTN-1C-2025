#ifndef INICIALIZAR_H
#define INICIALIZAR_H

#include "inicializar.h"
#include <stdint.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <stdbool.h>
#include "configuracion.h"

// Declaración de estructuras si es necesario usar tipos

void inicializar_memoria();
void crear_estructuras_para_proceso(uint32_t pid, char* nombre_archivo, int tamanio, uint32_t paginas_necesarias);
bool hay_espacio_para(uint32_t paginas_requeridas);
t_proceso_en_memoria* buscar_proceso_por_pid(uint32_t pid);
t_tabla_nivel* crear_tabla_nivel(int nivel_actual, int nivel_maximo);
void cargar_instrucciones(t_list* lista_instrucciones, char* nombre_archivo);
void log_metricas_proceso(t_proceso_en_memoria* proceso);
void liberar_marcos_de_proceso(t_tabla_nivel* tabla, int nivel_actual);
void liberar_proceso_en_memoria(t_proceso_en_memoria* proceso);

#endif
