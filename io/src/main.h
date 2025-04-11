#include "utils/libs/config.h"
#include "utils/libs/logger.h"

config_io_t config_IO;
t_log* logger;

void cargarConfiguracion(char* path) {
    load_config(path);
    config_IO = load_config_io();
    logger = iniciar_logger("io", config_IO.log_level);
}
