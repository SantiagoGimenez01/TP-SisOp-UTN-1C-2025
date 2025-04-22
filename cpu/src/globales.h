#ifndef GLOBALES_H
#define GLOBALES_H

#include "utils/libs/config.h"
#include "utils/libs/logger.h"

extern config_cpu_t configCPU;
extern t_log* logger;
// Sockets globales
extern int socket_memoria;
extern int socket_dispatch;
extern int socket_interrupt;
#endif
