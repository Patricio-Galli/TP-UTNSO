#include "patota.h"

uint32_t creo_segmento_pcb(uint32_t, uint32_t);
uint32_t creo_segmento_tareas(uint32_t, uint32_t, char**, uint32_t, uint32_t*);
void segmentar_pcb_p(uint32_t, uint32_t, char**);

bool iniciar_patota(uint32_t id_patota, t_list* parametros) {
	uint32_t tamanio_pcb = TAMANIO_PATOTA;	// TO CLEAN
	uint32_t tamanio_tarea = 0;
	uint32_t tamanio_bloque_tareas = 0;
	uint32_t cantidad_tareas = (int)list_get(parametros, 2);

	for(int i = 0; i < cantidad_tareas; i++) {
		tamanio_bloque_tareas += strlen((char *)list_get(parametros, 3 + i)) + 1;
	}

	// Valido que haya memoria desponible en la memoria ram
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		log_info(logger, "Inicio patota con SEGMENTACION");
		log_info(logger, "Memoria libre: %d", memoria_libre_segmentacion());
		if(TAMANIO_PATOTA + tamanio_bloque_tareas + (int)list_get(parametros, 1) * TAMANIO_TRIPULANTE > memoria_libre_segmentacion())
			return false;
	}

	if(memoria_ram.esquema_memoria == PAGINACION) {
		log_info(logger, "Inicio patota con PAGINACION");
		log_info(logger, "Frames necesarios: %d", frames_necesarios(0, TAMANIO_PATOTA + tamanio_bloque_tareas));
		log_info(logger, "Marcos logicos disponibles: %d", marcos_logicos_disponibles());
		if(frames_necesarios(0, TAMANIO_PATOTA + tamanio_bloque_tareas) > marcos_logicos_disponibles())
			return false;
	}
	log_info(logger, "Hay memoria disponible");

	// Valido que haya frames disponibles. Si no los hay, tengo que hacer un reemplazo.
	// if(memoria_ram.esquema_memoria == PAGINACION && bloques_disponibles(tamanio_pcb + tamanio_bloque_tareas >)

	// Preparo los datos para almacenar las tareas
	char** vtareas = malloc(sizeof(char *) * cantidad_tareas);
	uint32_t* vtareas_inicio = malloc(sizeof(uint32_t) * cantidad_tareas);
	uint32_t* vtareas_tamanio = malloc(sizeof(uint32_t) * cantidad_tareas);

	tamanio_bloque_tareas = 0;
	for(int i = 0; i < cantidad_tareas; i++) {
		char* tarea_i = (char *)list_get(parametros, 3 + i);
		tamanio_tarea = strlen(tarea_i) + 1;
		vtareas_inicio[i] = tamanio_bloque_tareas;
		tamanio_bloque_tareas += tamanio_tarea;
		vtareas[i] = tarea_i;
		vtareas_tamanio[i] = tamanio_tarea;
		log_info(logger, "Recibo tarea: %s. Inicio = %d/%d. Tamanio = %d/%d", tarea_i, tamanio_bloque_tareas, vtareas_inicio[i], tamanio_tarea, vtareas_tamanio[i]);
	}

	// CREO ESTRUCTURA PATOTA
	patota_data* nueva_patota = malloc(sizeof(patota_data));
	nueva_patota->PID = id_patota;
	nueva_patota->inicio_elementos = malloc(2 * sizeof(uint32_t));
	nueva_patota->cantidad_elementos = 2;
	nueva_patota->memoria_ocupada = TAMANIO_PATOTA + tamanio_bloque_tareas;
	list_add(lista_patotas, nueva_patota);

	// CREO ESTRUCTURA TAREAS
	tareas_data* nuevo_bloque_tareas = malloc(sizeof(tareas_data));
	nuevo_bloque_tareas->cant_tareas = cantidad_tareas;
	nuevo_bloque_tareas->inicio_tareas = vtareas_inicio;
	nuevo_bloque_tareas->tamanio_tareas = vtareas_tamanio;
	list_add(lista_tareas, nuevo_bloque_tareas);
	
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		nueva_patota->inicio_elementos[0] = creo_segmento_pcb(tamanio_pcb, id_patota);
		nueva_patota->inicio_elementos[1] = creo_segmento_tareas(tamanio_bloque_tareas, id_patota, vtareas, cantidad_tareas, &nueva_patota->inicio_elementos[0]);
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		asignar_frames(frames_necesarios(0, TAMANIO_PATOTA + tamanio_bloque_tareas), id_patota);
		nueva_patota->inicio_elementos[0] = 0;
		nueva_patota->inicio_elementos[1] = TAMANIO_PATOTA;
		segmentar_pcb_p(id_patota, cantidad_tareas, vtareas);
	}
	
	return true;
}

void actualizar_ubicacion_tareas(void* segmento, uint32_t nueva_ubicacion) {
	uint32_t valor_int = nueva_ubicacion;
	memcpy(segmento + sizeof(int), &valor_int, sizeof(uint32_t));
}

uint32_t creo_segmento_pcb(uint32_t tamanio_pcb, uint32_t id_patota) {
	t_segmento* segmento_pcb = crear_segmento(tamanio_pcb);
	segmento_pcb->duenio = id_patota;
	segmento_pcb->indice = 0;
	segmentar_entero(segmento_pcb->inicio, id_patota);
	return segmento_pcb->inicio;
}

uint32_t creo_segmento_tareas(uint32_t tamanio_bloque_tareas, uint32_t id_patota, char** vector_tareas, uint32_t cant_tareas, uint32_t* inicio_pcb) {
	t_segmento* segmento_pcb = segmento_desde_inicio(*inicio_pcb);
	t_segmento* segmento_tareas = crear_segmento(tamanio_bloque_tareas);
	segmento_tareas->duenio = id_patota;
	segmento_tareas->indice = 1;

	uint32_t desplazamiento = 0;
	for(int i = 0; i < cant_tareas; i++) {
		segmentar_string(memoria_ram.inicio, segmento_tareas->inicio + desplazamiento, vector_tareas[i]);
		desplazamiento += strlen(vector_tareas[i]/* + 1*/);
		free(vector_tareas[i]);
	}
	free(vector_tareas);
	
	*inicio_pcb = segmento_pcb->inicio;
	segmentar_entero(*inicio_pcb + sizeof(uint32_t), segmento_tareas->inicio);
	return segmento_tareas->inicio;
}

void segmentar_pcb_p(uint32_t id_patota, uint32_t cant_tareas, char** tareas) {
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	uint32_t pagina_actual = 0;
	uint32_t inicio_marco = mi_patota->frames[pagina_actual] * memoria_ram.tamanio_pagina;	// typedef
	segmentar_entero(inicio_marco, id_patota);
	segmentar_entero(inicio_marco + sizeof(uint32_t), inicio_marco + TAMANIO_PATOTA);
	signed int espacio_faltante = memoria_ram.tamanio_pagina - TAMANIO_PATOTA;
	mi_patota->memoria_ocupada = TAMANIO_PATOTA;
	for(int i = 0; i < cant_tareas; i++) {
		uint32_t largo_tarea = strlen(tareas[i]) + 1;
		espacio_faltante -= largo_tarea;
		if(espacio_faltante > 0) {
			memcpy(memoria_ram.inicio + inicio_marco + mi_patota->memoria_ocupada, tareas[i], largo_tarea);
		}
		else {
			memcpy(memoria_ram.inicio + inicio_marco + mi_patota->memoria_ocupada, tareas[i], largo_tarea + espacio_faltante);
			pagina_actual++;
			inicio_marco = mi_patota->frames[pagina_actual] * memoria_ram.tamanio_pagina;
			if (espacio_faltante != 0) {
				memcpy(memoria_ram.inicio + inicio_marco, tareas[i] - espacio_faltante, espacio_faltante);
			}
		}
		mi_patota->memoria_ocupada += largo_tarea;
		mi_patota->memoria_ocupada %= memoria_ram.tamanio_pagina;
		free(tareas[i]);
	}
	free(tareas);
}