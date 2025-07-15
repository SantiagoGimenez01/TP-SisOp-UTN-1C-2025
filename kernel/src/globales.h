#ifndef GLOBALES_H
#define GLOBALES_H

#include "utils/libs/config.h"
#include "utils/libs/logger.h"
#include "../../utils/src/utils/paquete.h"


extern config_kernel_t configKERNEL;  //Esto por ahora lo pongo asi porque los tengo que usar en otro archivo, como conexiones.c pero creo que hay algo mejor
extern t_log* logger;
extern t_list* cpus;  
extern t_list* ios;
extern t_list* cpus_incompletas;
extern t_list* pcbs;
extern pthread_mutex_t mutex_ios;

typedef struct {
    int id;     
    t_pcb* pcb_exec;
    int socket_dispatch;      
    int socket_interrupt;    
    int disponible;          
} t_cpu;

typedef struct {
    char* nombre;             
    int socket;                
    bool disponible;           
    int pid_actual; 
    t_queue* cola_procesos; 
    pthread_mutex_t mutex;          
} t_io;
typedef enum {
    CORTO_PLAZO,
    MEDIANO_PLAZO,
    LARGO_PLAZO,
} t_planificador;

extern uint32_t pid_global; // Este se va incrementando


#endif
