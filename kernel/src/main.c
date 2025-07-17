#include "main.h"

config_kernel_t configKERNEL;
t_log *logger;
uint32_t pid_global = 0;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Ingrese los 2 parametros del primer proceso: %s <archivo_pseudocodigo> <tamanio_proceso>\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *archivo_pseudocodigo = argv[1];
    int tamanio_proceso = atoi(argv[2]);

    cargarConfiguracionKernel("kernel.config", &configKERNEL, &logger);

    log_debug(logger, "Archivo pseudocodigo INICIAL: %s", archivo_pseudocodigo);
    log_debug(logger, "Tamanio del proceso INICIAL: %d", tamanio_proceso);

    establecerConexiones();

    // conectar_con_memoria(); // DE PRUEBA

    inicializarEstados(); // aca inicializo las listas de todos los estados

    inicializar_proceso(archivo_pseudocodigo, tamanio_proceso);

    iniciar_planificadores();

    pause();

    return 0;
}
