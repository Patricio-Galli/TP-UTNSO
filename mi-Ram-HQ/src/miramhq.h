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

#include "segmentos.h"


#include <errno.h>

#define ERROR_CONEXION -1

#define IP_RAM "127.0.0.1"

typedef struct {
    uint32_t PID;
    uint32_t tareas;
} t_patota;

typedef struct {
    uint32_t TID;
    char estado;
    uint32_t posicion_x;
    uint32_t posicion_y;
    uint32_t proxima_tarea;
    uint32_t pcb;
} t_tripulante;

typedef struct {
    uint32_t PID;
    uint32_t TID;
    pthread_t* hilo;
    // puntero a espacio de memoria, o a tabla de segmentos
} trip_data;

typedef struct {
    uint32_t PID;
    uint32_t cant_tareas;
    uint32_t* inicio_tareas;
    uint32_t* tamanio_tareas;
} tareas_data;

typedef struct {
    uint32_t PID;
    uint32_t tamanio_tabla;
    int* tabla_segmentos;    // puede ser un vector variable
} patota_data;

void* rutina_hilos(void* socket, t_tripulante* mi_tripulante);
bool iniciar_patota(t_log* logger, t_list* parametros, t_list* mapa_segmentos, t_list* lista_patotas, int patota_actual);
bool iniciar_tripulante(t_log* logger, t_list* parametros, t_list* mapa_segmentos, t_list* lista_tripulantes, t_list* lista_patotas);
// uint32_t obtener_pcb(int id_patota, int id_tripulante);

void agregar_parametro_a_segmento(t_list* parametros, void* data, uint32_t tamanio);
#endif /* _MIRAMHQ_H_ */