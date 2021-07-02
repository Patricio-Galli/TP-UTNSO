#ifndef _TAREAS_H_
#define _TAREAS_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t PID;
    uint32_t cant_tareas;
    uint32_t* inicio_tareas;
    uint32_t* tamanio_tareas;
} tareas_data;

char* obtener_tarea(void* segmento, tareas_data* tareas, uint32_t nro_tarea);

#endif /* _TAREAS_H_ */