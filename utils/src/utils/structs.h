#ifndef STRUCTS_H
#define STRUCTS_H
#include <commons/collections/list.h>
#include <stdint.h>
#include <pthread.h>


typedef enum {
    NEW,
    READY,
    EXEC,
    BLOCKED,
    SUSP_READY,
    SUSP_BLOCKED,
    EXIT_PROCESS  
} t_estado_proceso;

typedef enum{
    NOOP,
    WRITE,
    READ,
    GOTO,
    IO,
    INIT_PROC,
    DUMP_MEMORY,
    EXIT
}t_instruccion_id;


typedef enum {
    HANDSHAKE,
    INICIAR_PROCESO,
    ENVIAR_PCB,
    FINALIZAR_PROCESO,
    SOLICITUD_IO,
    SOLICITUD_READ,
    SOLICITUD_WRITE,
    SOLICITUD_DUMP,
    INTERRUPCION,
    PAGE_FAULT,    
    LEER_PAGINA,
    ESCRIBIR_PAGINA,  
    SYSCALL,
    INICIAR_IO,
    EJECUTAR_PROCESO,
    PEDIR_INSTRUCCION,
    PEDIR_CONFIGURACION, 
    CONTINUAR_PROCESO, 
    DESALOJAR_PROCESO, 
    CPU_LIBRE, 
    FIN_IO, 
    PEDIR_PAGINA_COMPLETA,
    PEDIR_MARCO,
    ACTUALIZAR_PAGINA_COMPLETA,
    SUSPENDER_PROCESO,
    DESUSPENDER_PROCESO,
    RESPUESTA_ESTIMACION,
    CONSULTAR_ESTIMACION_RESTANTE
} t_opcode;

typedef enum {
    MODULO_KERNEL,
    MODULO_CPU_DISPATCH,
    MODULO_CPU_INTERRUPT,
    MODULO_MEMORIA,
    MODULO_IO
} t_modulo;

typedef enum {
    RESPUESTA_OK,
    RESPUESTA_ERROR,
    RESPUESTA_INTERRUPCION,    
    RESPUESTA_END_OF_LINE,     
    RESPUESTA_DESALOJO        
} t_respuesta;

typedef enum {
    FIFO,
    SJF,
    SRT,
} planificador_t;

typedef struct {
    t_estado_proceso estado;
    uint32_t cantVeces;  
    uint64_t tiempoTotal;     
} t_metricas_estado;  // * dentro de esto

typedef struct {
    uint32_t pid;                  
    uint32_t pc;                   
    uint32_t tablaPaginas;            
    uint32_t tamanio;               
    t_estado_proceso estado_actual;
    uint64_t momento_entrada_estado;
    t_list* metricas; // Lista de t_metricas_estado*
    uint32_t estimacion_rafaga;   // Est(n+1)
    uint32_t estimacion_anterior; // Est(n)         
    uint32_t rafaga_anterior;     // R(n)  
    uint64_t timer_exec;     // R(n)  
    char* archivo_pseudocodigo;
    int tiempoIO;
    int timer_flag;
    pthread_mutex_t mutex_pcb;
} t_pcb;



#endif
