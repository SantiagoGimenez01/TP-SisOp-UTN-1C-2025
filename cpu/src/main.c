#include "main.h"

config_cpu_t configCPU;
t_log* logger;

int main(int argc, char* argv[]) {
    cargarConfiguracionCPU("cpu.config", &configCPU, &logger);
    establecerConexiones();
    return 0;
}
