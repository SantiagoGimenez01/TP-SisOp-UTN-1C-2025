#include "utils/libs/config.h" 
#include "utils/libs/logger.h" 

config_kernel_t configKERNEL;
t_log* logger;

void cargarConfiguracion(char* path) {
    load_config(path);                        
    configKERNEL = load_config_kernel();         
    logger = iniciar_logger("kernel", configKERNEL.log_level); 
}
typedef enum{
    CORTO_PLAZO,
    MEDIANO_PLAZO,
    LARGO_PLAZO,
}t_planificador;

