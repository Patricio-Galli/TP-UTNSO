#ifndef _INICIO_PATOTA_H_
#define _INICIO_PATOTA_H_

#include <commons/collections/list.h>
#include <stdint.h>
#include <commons/log.h>

#include "memoria_ram.h"
#include "segmentos.h"
#include "paginas.h"
#include "tareas.h"

#define TAMANIO_PATOTA (2 * sizeof(uint32_t))

bool iniciar_patota(uint32_t id_patota, t_list* parametros);

void actualizar_ubicacion_tareas(void* segmento, uint32_t nueva_ubicacion);

void segmentar_pcb_p(uint32_t id_patota, uint32_t cant_tareas, char**);

#endif /* _INICIO_PATOTA_H_ */