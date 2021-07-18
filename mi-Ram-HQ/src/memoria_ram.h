#ifndef _MEMORIA_RAM_H_
#define _MEMORIA_RAM_H_

#include <commons/collections/list.h>
#include <commons/log.h>
#include <stdint.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <semaphore.h>

#define IP_RAM "127.0.0.1"
#define CONSOLA_ACTIVA 1

uint32_t tamanio_memoria;
void* memoria_ram;

t_list* lista_patotas;
t_list* lista_tareas;
t_list* lista_tripulantes;
t_list* movimientos_pendientes;

t_list* mapa_segmentos; // segmentacion
t_list* mapa_paginas;   // paginacion

t_log* logger;

sem_t semaforo_consola;

typedef enum {
    FF,
    BF
} algoritmo_segmento;

typedef struct {
    uint32_t PID;
    uint32_t tamanio_tabla;
    int* tabla_segmentos;    // Su tamanio depende de la cantidad de tripulantes
} patota_data;

typedef struct {
    uint32_t PID;
    uint32_t TID;
    uint32_t pos_x;
    uint32_t pos_y;
    bool seguir;
} t_movimiento;

#endif /* _MEMORIA_RAM_H_ */