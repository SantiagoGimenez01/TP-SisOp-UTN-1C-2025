#ifndef STRUCTS_H
#define STRUCTS_H
#include <commons/collections/list.h>
#include <stdint.h>


typedef enum {
    ESTADO_NEW,
    ESTADO_READY,
    ESTADO_EXEC,
    ESTADO_BLOCKED,
    ESTADO_SUSPENDED_READY,
    ESTADO_SUSPENDED_BLOCKED,
    ESTADO_EXIT
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
    SUSPENDER_PROCESO,
    SOLICITUD_IO,
    SOLICITUD_READ,
    SOLICITUD_WRITE,
    SOLICITUD_DUMP,
    INTERRUPCION_CPU,
    PAGE_FAULT,    
    WRITE_PAGE,  
    SYSCALL   
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
    uint32_t estRafagaAnt;       
    uint32_t proxRafaga;
    uint32_t realRafaga;     
    uint32_t tamanio;               
    t_list* metricas_estado;     // eso lo podemos tomar como una sola lista para tiempo y estado y jugar dentro de esto *   
    t_list* metricas_tiempo;        
} t_pcb;

typedef struct {
    t_instruccion_id id;
    uint32_t parametro1;
    uint32_t parametro2;
} t_instruccion;


#endif
