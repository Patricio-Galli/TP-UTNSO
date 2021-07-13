#include "patota.h"

bool iniciar_patota(uint32_t id_patota, t_list* parametros, algoritmo_segmento algoritmo) {
	uint32_t tamanio_pcb = TAMANIO_PATOTA;
	uint32_t tamanio_tarea = 0;
	uint32_t tamanio_bloque_tareas = 0;
	uint32_t cantidad_tareas = (int)list_get(parametros, 1);

	for(int i = 0; i < cantidad_tareas; i++) {
		tamanio_bloque_tareas += strlen((char *)list_get(parametros, 2 + i)) + 1;
	}

	if (tamanio_pcb + tamanio_bloque_tareas > memoria_libre()) {
		return false;
	}

	char** vtareas = malloc(sizeof(char *) * cantidad_tareas);
	uint32_t* vtareas_inicio = malloc(sizeof(uint32_t) * cantidad_tareas);
	uint32_t* vtareas_tamanio = malloc(sizeof(uint32_t) * cantidad_tareas);

	tamanio_bloque_tareas = 0;
	for(int i = 0; i < cantidad_tareas; i++) {
		char* tarea_i = (char *)list_get(parametros, 2 + i);
		tamanio_tarea = strlen(tarea_i) + 1;
		vtareas_inicio[i] = tamanio_bloque_tareas;
		tamanio_bloque_tareas += tamanio_tarea;
		vtareas[i] = tarea_i;
		vtareas_tamanio[i] = tamanio_tarea;
	}

	// CREO SEGMENTO PCB
	t_segmento* segmento_pcb = crear_segmento(mapa_segmentos, tamanio_pcb, algoritmo);
	if(segmento_pcb == NULL) {
		realizar_compactacion();
		segmento_pcb = crear_segmento(mapa_segmentos, tamanio_pcb, algoritmo);
	}
	segmento_pcb->duenio = id_patota;
	segmento_pcb->indice = 0;
	segmentar_entero(memoria_ram, segmento_pcb->inicio, id_patota);
	
	// CREO ESTRUCTURA PATOTA PARA GUARDAR EN TABLA
	patota_data* nueva_patota = malloc(sizeof(patota_data));
	nueva_patota->PID = id_patota;
	nueva_patota->tabla_segmentos = malloc(2 * sizeof(uint32_t));
	nueva_patota->tamanio_tabla = 2;
	nueva_patota->tabla_segmentos[0] = segmento_pcb->inicio;
	list_add(lista_patotas, nueva_patota);

	// CREO SEGMENTO TAREAS
	t_segmento* segmento_tareas = crear_segmento(mapa_segmentos, tamanio_bloque_tareas, algoritmo);
	if(segmento_tareas == NULL) {
		realizar_compactacion();
		segmento_tareas = crear_segmento(mapa_segmentos, tamanio_bloque_tareas, algoritmo);
	}
	segmento_tareas->duenio = id_patota;
	segmento_tareas->indice = 1;

	for(int i = 0; i < cantidad_tareas; i++) {
		segmentar_string(memoria_ram, segmento_tareas->inicio + vtareas_inicio[i], vtareas[i]);
		free(vtareas[i]);
	}
	free(vtareas);
	nueva_patota->tabla_segmentos[1] = segmento_tareas->inicio;
	segmentar_entero(memoria_ram, segmento_pcb->inicio + sizeof(uint32_t), segmento_tareas->inicio);

	// CREO ESTRUCTURA TAREAS PARA GUARDAR EN TABLA
	tareas_data* nuevo_bloque_tareas = malloc(sizeof(tareas_data));
	nuevo_bloque_tareas->cant_tareas = cantidad_tareas;
	nuevo_bloque_tareas->inicio_tareas = vtareas_inicio;
	nuevo_bloque_tareas->tamanio_tareas = vtareas_tamanio;
	list_add(lista_tareas, nuevo_bloque_tareas);
	return true;
}

void actualizar_ubicacion_tareas(void* segmento, uint32_t nueva_ubicacion) {
	uint32_t valor_int = nueva_ubicacion;
	memcpy(segmento + sizeof(int), &valor_int, sizeof(uint32_t));
}