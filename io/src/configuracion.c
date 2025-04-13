#include "configuracion.h"
#include "globales.h"
#include "conexiones.h"

void cargarConfiguracionIO(char* path, config_io_t* configIO, t_log** logger) {
    t_config* config = iniciar_config(path);
    *configIO = load_config_io(config);
    *logger = iniciar_logger("io", configIO->log_level);
} // PARA TODOS LOS MODULOS NO OLVIDAR EL CONFIG DESTROY AL FINAL DE TODO