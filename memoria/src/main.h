#include "utils/libs/config.h"
#include "utils/libs/logger.h"

config_memoria_t configMEMORIA;
t_log* logger;

void cargarConfiguracion(char* path) {
    load_config(path);
    configMEMORIA = load_config_memoria();
    logger = iniciar_logger("memoria", configMEMORIA.log_level);
}