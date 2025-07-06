#include "configuracion.h"

void cargarConfiguracionMemoria(char* path, config_memoria_t* configMemoria, t_log** logger){
    t_config* config = iniciar_config(path);
    *configMemoria = load_config_memoria(config);
    *logger = iniciar_logger("memoria", configMemoria->log_level);

}