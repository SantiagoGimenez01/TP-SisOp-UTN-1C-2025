#include "configuracion.h"

void cargarConfiguracionMemoria(char* path, config_memoria_t* configMemoria, t_log** logger){
    config_global = iniciar_config(path);
    *configMemoria = load_config_memoria(config_global);
    *logger = iniciar_logger("memoria", configMemoria->log_level);

}