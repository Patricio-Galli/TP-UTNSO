#ifndef _SEGMENTOS_H_
#define _SEGMENTOS_H_

#include <stdint.h>
#include <stdlib.h>

#include <commons/collections/list.h>

#include <string.h> // memcpy
#include <stdio.h>  // a quitar, solo pruebas

typedef enum {
    FF,
    BF
} algoritmo_segmento;

typedef enum {
    SEG_INT,
    SEG_CHAR,
    SEG_STRING
} tipo_dato;

typedef struct {
    void* data;
    tipo_dato tipo;
    uint32_t tamanio;
} nodo_seg;

typedef enum {
    PCB,
    TAREAS,
    TCB
} tipo_segmento;

typedef struct {
    uint32_t n_segmento;
    uint32_t inicio;
    uint32_t tamanio;
    uint32_t duenio;    // proceso, 0 si está libre.
} t_segmento;

t_segmento* crear_segmento(t_list* mapa_segmentos, uint32_t tamanio_segmento, algoritmo_segmento algoritmo);
void eliminar_segmento(t_list* mapa_segmentos, int segmento);

void segmentar(void* memoria, t_segmento* segmento, t_list* parametros_seg);
void agregar_parametro_a_segmento(t_list* parametros, void* data, tipo_dato tipo);

void nuevo_segmentar(void* memoria, uint32_t posicion, void* data, tipo_dato tipo, uint32_t tamanio);

void segmentar_tcb(void* memoria, t_segmento* segmento, t_list* parametros);
void segmentar_tareas(void* memoria, t_segmento* segmento, t_list* parametros, int* vector_tareas_tam);
void segmentar_pcb(void* memoria, t_segmento* segmento, t_list* parametros);

void segmentar_caracter(void* memoria, uint32_t posicion, char data);
void segmentar_entero(void* memoria, uint32_t posicion, uint32_t data);
void segmentar_string(void* memoria, uint32_t posicion, char* data);

// void nuevo_segmentar_cadena(void* memoria, uint32_t posicion, char* data, uint32_t tamanio);
#endif /* _SEGMENTOS_H_ */