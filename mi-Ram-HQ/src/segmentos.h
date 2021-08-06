#ifndef _SEGMENTOS_H_
#define _SEGMENTOS_H_

#include <stdint.h>
#include <stdlib.h>

#include <commons/collections/list.h>
#include <commons/config.h>
#include "memoria_ram.h"
#include "patota.h"
#include "tripulante.h"

#include <string.h> // memcpy
#include <stdio.h>  // a quitar, solo pruebas

typedef struct {
    uint32_t n_segmento;
    uint32_t inicio;
    uint32_t tamanio;
    uint32_t duenio;    // proceso, 0 si está libre.
    uint32_t indice;
    sem_t mutex;
} t_segmento;

bool iniciar_memoria_segmentada(t_config* config);

t_segmento* crear_segmento(uint32_t tamanio_segmento);
void eliminar_segmento(uint32_t nro_segmento);

void segmentar_caracter(void* memoria, uint32_t posicion, char data);
void segmentar_entero(uint32_t posicion, uint32_t data);
void segmentar_string(void* memoria, uint32_t posicion, char* data);
void segmentar_bloque(void* memoria, uint32_t posicion, void* data, uint32_t tamanio);

t_list* seg_tripulantes_de_patota(uint32_t id_patota);
t_list* seg_ordenados_de_patota(uint32_t id_patota);

void* obtener_bloque_memoria(void* memoria, t_segmento* segmento);

void realizar_compactacion();

uint32_t memoria_libre_segmentacion();
t_segmento* segmento_desde_inicio(uint32_t inicio_segmento);
uint32_t nro_segmento_desde_inicio(uint32_t inicio_segmento);

uint32_t trip_de_segmento(uint32_t inicio_segmento);
void actualizar_ubicacion_tareas(void* segmento, uint32_t nueva_ubicacion);

#endif /* _SEGMENTOS_H_ */