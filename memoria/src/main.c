#include "main.h"

config_memoria_t configMEMORIA;
t_log* logger;
t_bitarray* bitmap_frames;
int cantidad_frames;
int frames_libres;
void* memoria_fisica;
t_list* procesos_en_memoria;
t_list* paginas_en_swap;

int main(int argc, char* argv[]) {
    cargarConfiguracionMemoria("memoria.config", &configMEMORIA, &logger);
    inicializar_memoria();
    establecerConexiones();
 
    return 0;
}




