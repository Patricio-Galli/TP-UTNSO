#ifndef _MEMORIA_RAM_H_
#define _MEMORIA_RAM_H_

#include <commons/collections/list.h>
#include <stdint.h>
#include <pthread.h>

#define IP_RAM "127.0.0.1"

uint32_t tamanio_memoria;
void* memoria_ram;

t_list* lista_patotas;
t_list* lista_tareas;
t_list* lista_tripulantes;

t_list* mapa_segmentos; // segmentacion
t_list* mapa_paginas;   // paginacion

typedef enum {
    FF,
    BF
} algoritmo_segmento;

typedef struct {
    uint32_t PID;
    uint32_t tamanio_tabla;
    int* tabla_segmentos;    // Su tamanio depende de la cantidad de tripulantes
} patota_data;

#endif /* _MEMORIA_RAM_H_ */