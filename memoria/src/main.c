#include "main.h"
#include "configuracion.h"

config_memoria_t configMEMORIA;
t_log* logger;

int main(int argc, char* argv[]) {
    cargarConfiguracionMemoria("memoria.config", &configMEMORIA, &logger);
    establecerConexiones();
 
    return 0;
}
