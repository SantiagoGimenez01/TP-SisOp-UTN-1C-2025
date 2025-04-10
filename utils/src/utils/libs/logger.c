// logger.c
#include "logger.h"

t_log* iniciar_logger(char* nombre_modulo, t_log_level nivel) {
    char* path = string_from_format("./logs/%s.log", nombre_modulo);
    remove(path); 
    t_log* nuevo_logger = log_create(path, nombre_modulo, true, nivel);
    free(path);
    return nuevo_logger;
}
