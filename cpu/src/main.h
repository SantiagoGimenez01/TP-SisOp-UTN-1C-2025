#include "utils/libs/config.h"
#include "utils/libs/logger.h"

config_cpu_t configCPU;
t_log* logger;

void cargarConfiguracion(char* path) {
    load_config(path);
    configCPU = load_config_cpu();
    logger = iniciar_logger("cpu", configCPU.log_level);
}