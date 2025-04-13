#include "main.h"
config_kernel_t configKERNEL;
t_log* logger;

int main(int argc, char* argv[]) {
    cargarConfiguracionKernel("kernel.config", &configKERNEL, &logger);
    
    establecerConexiones();
    
    conectar_con_memoria(); // DE PRUEBA

    while (1) { // simular haciendo algo hasta que se conecte cpu
        sleep(1);
    }
    return 0;
}
