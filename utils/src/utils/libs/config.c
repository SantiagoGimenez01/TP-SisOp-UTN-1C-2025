#include "config.h"

t_config* iniciar_config(char* path) {
    t_config* config = config_create(path);
    if (config == NULL) {
        printf("No se pudo leer el archivo de configuracion: %s\n", path);
        exit(EXIT_FAILURE);
    }
    return config;
}
