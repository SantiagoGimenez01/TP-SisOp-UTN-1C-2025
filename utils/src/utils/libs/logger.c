// logger.c
#include "logger.h"

t_log* iniciar_logger(char* nombre_modulo, t_log_level nivel) {
    char* log_ext = ".log";
    char* result = malloc(strlen(nombre_modulo) + strlen(log_ext) + 1);
    strcpy(result, nombre_modulo);

    if (!string_ends_with(nombre_modulo, log_ext))
        string_append(&result, log_ext);

    t_log* nuevo_logger = log_create(result, nombre_modulo, true, nivel);
    free(result);
    return nuevo_logger;
}