#include "tareas.h"

char* obtener_tarea(void* segmento, tareas_data* tareas, uint32_t nro_tarea) {
	char * tarea = malloc(tareas->tamanio_tareas[nro_tarea] + 1);
	memcpy(tarea, segmento + tareas->inicio_tareas[nro_tarea], tareas->tamanio_tareas[nro_tarea]);
	char final = '\0';
	memcpy(tarea + tareas->tamanio_tareas[nro_tarea], &final, 1);
	return tarea;
}