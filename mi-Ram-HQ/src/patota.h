#ifndef _INICIO_PATOTA_H_
#define _INICIO_PATOTA_H_

#include <commons/collections/list.h>
#include <stdint.h>
#include <commons/log.h>

#include "memoria_ram.h"
#include "segmentos.h"
#include "tareas.h"

#define TAMANIO_PATOTA (2 * sizeof(uint32_t))

bool iniciar_patota(uint32_t id_patota, t_list* parametros, algoritmo_segmento algoritmo);

#endif /* _INICIO_PATOTA_H_ */