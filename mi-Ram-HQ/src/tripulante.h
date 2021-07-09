#ifndef _TRIPULANTE_H_
#define _TRIPULANTE_H_

#include <commons/collections/list.h>
#include <stdint.h>
#include <commons/log.h>

#include "memoria_ram.h"
#include "segmentos.h"

#define TAMANIO_TRIPULANTE (5 * sizeof(uint32_t) + sizeof(char))

typedef enum {
    TRIP_IP,
    ESTADO,
    POS_X,
    POS_Y,
    INS_POINTER,
    PCB_POINTER
} para_trip;

typedef struct {
    uint32_t PID;
    uint32_t TID;
    uint32_t inicio;
    int socket;
    pthread_t* hilo;
    bool seguir;
} trip_data;

bool iniciar_tripulante(uint32_t id_trip, uint32_t id_patota, uint32_t pos_x, uint32_t pos_y, algoritmo_segmento algoritmo);
void eliminar_tripulante(uint32_t id_patota, uint32_t id_tripulante);

uint32_t obtener_valor_tripulante(void* segmento, para_trip nro_parametro);
char obtener_estado(void* segmento);

void actualizar_valor_tripulante(void* segmento, para_trip nro_parametro, uint32_t nuevo_valor);
void actualizar_estado(void* segmento, char nuevo_valor);

t_list* tripulantes_de_patota(uint32_t id_patota);
trip_data* tripulante_de_lista(uint32_t id_patota, uint32_t id_trip);

#endif /* _TRIPULANTE_H_ */