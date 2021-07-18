#ifndef _MIRAMHQ_H_
#define _MIRAMHQ_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<utils/utils-server.h>
#include<utils/utils-sockets.h>
#include<utils/utils-mensajes.h>

#include<commons/collections/list.h>
#include<commons/config.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "memoria_ram.h"
#include "segmentos.h"
#include "patota.h"
#include "tareas.h"
#include "tripulante.h"
#include "consola.h"

#include <errno.h>

#define ERROR_CONEXION -1

pthread_t* iniciar_mapa(bool*);
void iniciar_memoria(t_config*);

void liberar_segmentos();
void liberar_patotas();
void liberar_tareas();
void liberar_tripulantes();

#endif /* _MIRAMHQ_H_ */