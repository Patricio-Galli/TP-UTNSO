#include "tareas.h"

char* obtener_tarea(void* segmento, tareas_data* tareas, uint32_t nro_tarea) {
	char * tarea = malloc(tareas->tamanio_tareas[nro_tarea]);
	memcpy(tarea, segmento + tareas->inicio_tareas[nro_tarea], tareas->tamanio_tareas[nro_tarea]);
	return tarea;
}