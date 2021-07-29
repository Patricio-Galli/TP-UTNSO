#ifndef _SEGMENTOS_H_
#define _SEGMENTOS_H_

#include <stdint.h>
#include <stdlib.h>

#include <commons/collections/list.h>
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
} t_segmento;

t_segmento* crear_segmento(uint32_t tamanio_segmento);
void eliminar_segmento(uint32_t nro_segmento);

void segmentar_caracter(void* memoria, uint32_t posicion, char data);
void segmentar_entero(uint32_t posicion, uint32_t data);
void segmentar_string(void* memoria, uint32_t posicion, char* data);
void segmentar_bloque(void* memoria, uint32_t posicion, void* data, uint32_t tamanio);
t_list* seg_tripulantes_de_patota(uint32_t id_patota);

void* obtener_bloque_memoria(void* memoria, t_segmento* segmento);

void realizar_compactacion();

uint32_t memoria_libre_segmentacion();
t_segmento* segmento_desde_inicio(uint32_t inicio_segmento);

#endif /* _SEGMENTOS_H_ */