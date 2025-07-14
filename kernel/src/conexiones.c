#include "conexiones.h"

// --- Funciones auxiliares para cada puerto ---
void comprobacionModulo(t_modulo modulo_origen, t_modulo esperado, char *modulo, void (*operacion)(int), int socket_cliente)
{

    if (modulo_origen == esperado)
    {
        log_debug(logger, "Se conecto %s", modulo);
        pthread_t hilo_operacion;
        pthread_create(&hilo_operacion, NULL, operacion, (void *)(int)socket_cliente);
        pthread_detach(hilo_operacion); // Operaciones de modulos
    }
    else
    {
        log_warning(logger, "No es %s", modulo);
        close(socket_cliente);
    }
}

void *escuchar_dispatch(void *socket_servidor_void)
{
    int socket_servidor = (intptr_t)socket_servidor_void;
    log_debug(logger, "Servidor KERNEL_DISPATCH escuchando conexiones.");

    while (1)
    {
        log_debug(logger, "Esperando CPU");
        int socket_cliente = esperarCliente(socket_servidor, logger);
        int id_cpu;
        recv(socket_cliente, &id_cpu, sizeof(int), 0);
        log_debug(logger, "CPU ID recibido: %d en socket FD: %d", id_cpu, socket_cliente);
        agregarNuevaCpuInc(socket_cliente, id_cpu); // agregamos el primer SOCKET a nuestra nueva CPU inicializando esta misma a la vez
        log_debug(logger, "CPUs incompletas: %d, CPUs completas: %d", list_size(cpus_incompletas), list_size(cpus));

        // estoy en duda con esto si hacer recv o recibir handshake
        t_modulo modulo_origen;
        recv(socket_cliente, &modulo_origen, sizeof(t_modulo), 0);
        comprobacionModulo(modulo_origen, MODULO_CPU_DISPATCH, "CPU_DISPATCH", operarDispatch, socket_cliente);
        sem_post(&sem_agregar_cpu);
    }
    return NULL;
}

void *escuchar_interrupt(void *socket_servidor_void)
{
    int socket_servidor = (intptr_t)socket_servidor_void;
    log_debug(logger, "Servidor KERNEL_INTERRUPT escuchando conexiones.");

    while (1)
    {
        sem_wait(&sem_agregar_cpu);
        int socket_cliente = esperarCliente(socket_servidor, logger);
        int id_cpu;
        recv(socket_cliente, &id_cpu, sizeof(int), 0);
        log_debug(logger, "CPU ID recibido: %d en socket FD: %d", id_cpu, socket_cliente);
        agregarNuevaCpu(socket_cliente, id_cpu); // Terminamos de completar la nueva CPU
        log_debug(logger, "CPUs incompletas: %d, CPUs completas: %d", list_size(cpus_incompletas), list_size(cpus));
        sem_post(&sem_cpu_disponible);
        t_modulo modulo_origen;
        recv(socket_cliente, &modulo_origen, sizeof(t_modulo), 0);
        comprobacionModulo(modulo_origen, MODULO_CPU_INTERRUPT, "CPU_INTERRUPT", operarInterrupt, socket_cliente);
    }
    return NULL;
}

void *escuchar_io(void *socket_servidor_void)
{
    int socket_servidor = (intptr_t)socket_servidor_void;
    log_debug(logger, "Servidor KERNEL_IO escuchando conexiones.");

    while (1)
    {
        int socket_cliente = esperarCliente(socket_servidor, logger);

        t_modulo modulo_origen;
        recv(socket_cliente, &modulo_origen, sizeof(t_modulo), 0);

        comprobacionModulo(modulo_origen, MODULO_IO, "IO", operarIo, socket_cliente);
    }
    return NULL;
}

// --- Funcion principal ---

void establecerConexiones()
{
    // Iniciar 3 servidores TCP (en teoria como entendi deberia ser asi...)
    int socket_dispatch = iniciarServidor(string_itoa(configKERNEL.puerto_escucha_dispatch), logger, "KERNEL_DISPATCH");
    int socket_interrupt = iniciarServidor(string_itoa(configKERNEL.puerto_escucha_interrupt), logger, "KERNEL_INTERRUPT");
    int socket_io = iniciarServidor(string_itoa(configKERNEL.puerto_escucha_io), logger, "KERNEL_IO");

    // Crear 3 hilos para escuchar en paralelo
    pthread_t hilo_dispatch, hilo_interrupt, hilo_io;
    pthread_create(&hilo_dispatch, NULL, escuchar_dispatch, (void *)(intptr_t)socket_dispatch);
    pthread_create(&hilo_interrupt, NULL, escuchar_interrupt, (void *)(intptr_t)socket_interrupt);
    pthread_create(&hilo_io, NULL, escuchar_io, (void *)(intptr_t)socket_io);

    pthread_detach(hilo_dispatch);
    pthread_detach(hilo_interrupt);
    pthread_detach(hilo_io);

    // El Kernel sigue ejecutando otras cosas mientras estos hilos aceptan clientes, cada uno tiene un bucle esperando muchas instancias
}

void operarDispatch(int socket_cliente)
{
    log_debug(logger, "Manejando conexion DISPATCH");

    while (1)
    {
        t_opcode codigo_operacion;
        int status = recv(socket_cliente, &codigo_operacion, sizeof(t_opcode), MSG_WAITALL);

        if (status <= 0)
        {
            log_warning(logger, "Se cerro la conexion con CPU Dispatch (socket %d)", socket_cliente);
            close(socket_cliente);
            return;
        }

        switch (codigo_operacion)
        {
        case SYSCALL:
            log_debug(logger, "Se recibio una SYSCALL desde CPU (socket %d)", socket_cliente);

            t_paquete *paquete = recibir_paquete(socket_cliente);
            procesar_syscall(paquete, socket_cliente);
            eliminar_paquete(paquete);

            break;

        case CPU_LIBRE:
            log_debug(logger, "CPU en socket %d marco su disponibilidad", socket_cliente);
            marcar_cpu_como_libre(socket_cliente, true);
            sem_post(&sem_corto_plazo);
            sem_post(&sem_cpu_disponible);
            break;

        default:
            log_warning(logger, "Codigo de operacion inesperado en DISPATCH: %d", codigo_operacion);
            break;
        }
    }
}

void operarInterrupt(int socket_cliente)
{
    log_debug(logger, "Manejando conexion INTERRUPT");

    while (1)
    {
        t_opcode codigo_operacion;
        int status = recv(socket_cliente, &codigo_operacion, sizeof(t_opcode), MSG_WAITALL);

        if (status <= 0)
        {
            log_warning(logger, "Se cerro la conexion con CPU Interrupt (socket %d)", socket_cliente);
            close(socket_cliente);
            return;
        }

        switch (codigo_operacion)
        {
        case CPU_LIBRE:
            log_debug(logger, "CPU en socket %d marco su disponibilidad (por interrupt)", socket_cliente);
            marcar_cpu_como_libre(socket_cliente, false);
            log_debug(logger, "CPU en socket %d marcada como disponible sin replanificar", socket_cliente);
            break;
        case DESALOJAR_PROCESO:
            log_debug(logger, "Se recibio DESALOJAR_PROCESO desde CPU (socket %d)", socket_cliente);

            t_paquete *paquete = recibir_paquete(socket_cliente);
            uint32_t pid, pc, timer_exec;
            memcpy(&pid, paquete->buffer->stream, sizeof(uint32_t));
            memcpy(&pc, paquete->buffer->stream + sizeof(uint32_t), sizeof(uint32_t));
            memcpy(&timer_exec, paquete->buffer->stream + sizeof(uint32_t) + sizeof(uint32_t), sizeof(uint32_t));

            eliminar_paquete(paquete);

            t_pcb *pcb = buscar_pcb_por_pid(pid);
            if (pcb == NULL)
            {
                log_error(logger, "No se encontro el PCB para el PID %d", pid);
                break;
            }

            pcb->pc = pc;
            pcb->timer_exec = timer_exec;
            cambiar_estado(pcb, READY);
            log_info(logger, "## (%i) - Desalojado por algoritmo SRT", pid);
            marcar_cpu_como_libre(socket_cliente, false);
            sem_post(&sem_procesos_en_ready);

            break;
        default:
            log_warning(logger, "Codigo de operacion inesperado en INTERRUPT: %d", codigo_operacion);
            break;
        }
    }
}

void operarIo(int socket_cliente)
{
    log_debug(logger, "Registrando nuevo dispositivo IO...");

    t_opcode codOperacion;
    recv(socket_cliente, &codOperacion, sizeof(t_opcode), MSG_WAITALL);

    if (codOperacion != INICIAR_IO)
    {
        log_error(logger, "Operacion inesperada. Se esperaba INICIAR_IO.");
        close(socket_cliente);
        return;
    }

    t_paquete *paquete = recibir_paquete(socket_cliente);
    char *nombre_io = recibir_string_de_paquete(paquete);
    eliminar_paquete(paquete);

    agregarNuevaIo(nombre_io, socket_cliente);
    log_debug(logger, "IO %s registrado correctamente con socket %d", nombre_io, socket_cliente);
    free(nombre_io);

    // recepcion de solicitudes
    while (1)
    {
        t_opcode op;
        int recv_bytes = recv(socket_cliente, &op, sizeof(op), MSG_WAITALL);
        if (recv_bytes <= 0)
        {
            log_warning(logger, "Desconexion de IO (socket %d). Eliminando de lista IOs", socket_cliente);
            t_io *dispositivo = buscar_io_por_socket(socket_cliente);
            // Finalizo el proceso que estaba usando IO
            t_pcb *pcb = buscar_pcb_por_pid(dispositivo->pid_actual);
            if (pcb)
            {
                log_debug(logger, "Finalizando proceso en IO %d", dispositivo->pid_actual);
                pthread_mutex_lock(&pcb->mutex_pcb);
                cambiar_estado(pcb, EXIT_PROCESS);
                finalizar_proceso(pcb);
                pthread_mutex_unlock(&pcb->mutex_pcb);
            }
            // Finalizo los que estaban encolados
            while (!queue_is_empty(dispositivo->cola_procesos))
            {
                t_pcb *pcb_siguiente = queue_pop(dispositivo->cola_procesos);
                log_debug(logger, "Finalizando proceso encolado %d", pcb_siguiente->pid);
                pthread_mutex_lock(&pcb_siguiente->mutex_pcb);
                cambiar_estado(pcb_siguiente, EXIT_PROCESS);
                finalizar_proceso(pcb_siguiente);
                pthread_mutex_unlock(&pcb_siguiente->mutex_pcb);
            }

            int index_dispositivo = buscar_index_io_por_socket(socket_cliente);
            list_remove_and_destroy_element(ios, index_dispositivo, free);
            break;
        }

        switch (op)
        {
        case FIN_IO:
        {
            t_paquete *paquete = recibir_paquete(socket_cliente);
            int pid;
            memcpy(&pid, paquete->buffer->stream, sizeof(int));
            eliminar_paquete(paquete);

            t_io *dispositivo = buscar_io_por_socket(socket_cliente);
            if (!dispositivo)
            {
                log_error(logger, "No se pudo identificar el IO por socket %d", socket_cliente);
                break;
            }

            log_info(logger, "## (%i) finalizó IO y pasa a READY", pid);
            log_debug(logger, "Fin de IO: %s recibido del PID %d", dispositivo->nombre, pid);
            dispositivo->disponible = 1;
            dispositivo->pid_actual = -1;

            t_pcb *pcb = buscar_pcb_por_pid(pid);

            if (pcb)
            {
                pthread_mutex_lock(&pcb->mutex_pcb);
                if (pcb->estado_actual == BLOCKED)
                {
                    pcb->timer_flag = -1; // invalida el timer
                    // log_info(logger,"## (%d) El proceso ya se desbloqueo", pcb->pid);
                    cambiar_estado(pcb, READY); // ACA TENEMOS QUE RECORDAR QUE EL PROCESO PUDO PASAR A SUSP READY
                    // sem_post(&sem_procesos_en_ready); Este creo que estaba de mas ya que ya se hace el sem_post cuando se agrega pcb a lista_ready
                    // log_info(logger, "Se hizo un semPost de ready");
                }
                else
                {
                    cambiar_estado(pcb, SUSP_READY); // Pasamos el proceso a SUSP_READY
                }
                pcb->tiempoIO = -1;
                pthread_mutex_unlock(&pcb->mutex_pcb);
            }

            if (!queue_is_empty(dispositivo->cola_procesos))
            {
                t_pcb *siguiente = queue_pop(dispositivo->cola_procesos);
                usar_o_encolar_io(dispositivo, siguiente, siguiente->tiempoIO);
            }
            break;
        }

        default:
            log_warning(logger, "IO recibio opcode inesperado: %d", op);
            break;
        }
    }

    close(socket_cliente);
}
