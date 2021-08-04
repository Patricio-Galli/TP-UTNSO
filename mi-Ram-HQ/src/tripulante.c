#include "tripulante.h"

uint32_t creo_segmento_tcb(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

int iniciar_tripulante(uint32_t id_trip, uint32_t id_patota, uint32_t pos_x, uint32_t pos_y) {
	int tamanio_tcb = TAMANIO_TRIPULANTE;
	patota_data* patota = (patota_data *)list_get(lista_patotas, id_patota - 1);

	#define espacio_ocupado_ultimo_frame (patota->memoria_ocupada % memoria_ram.tamanio_pagina)
	// log_info(logger, "Entro a iniciar_tripulante. Espacio ocupado %d", espacio_ocupado_ultimo_frame);

	uint32_t inicio_tcb;

	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		inicio_tcb = creo_segmento_tcb(tamanio_tcb, id_patota, id_trip, pos_x, pos_y, patota->inicio_elementos[0]);
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		// log_info(logger, "Frames necesarios para tripulante %d: %d", id_trip, frames_necesarios(TAMANIO_PAGINA - espacio_ocupado_ultimo_frame, TAMANIO_TRIPULANTE));
		asignar_frames_p(id_patota, frames_necesarios(TAMANIO_PAGINA - espacio_ocupado_ultimo_frame, TAMANIO_TRIPULANTE));
		patota->inicio_elementos[id_trip + 1] = patota->memoria_ocupada;
		inicio_tcb = patota->memoria_ocupada;
		// log_info(logger, "Actualizo valores del tcb. Trip %d, memoria_ocupada: %d", id_trip, patota->memoria_ocupada);
		actualizar_entero_paginacion(id_patota, patota->memoria_ocupada, id_trip);
		patota->memoria_ocupada += sizeof(uint32_t);
		// log_info(logger, "Estado");
		actualizar_estado(id_patota, id_trip, 'N');
		patota->memoria_ocupada += sizeof(char);
		// log_info(logger, "Posx");
		actualizar_entero_paginacion(id_patota, patota->memoria_ocupada, pos_x);
		patota->memoria_ocupada += sizeof(uint32_t);
		// log_info(logger, "Posx");
		actualizar_entero_paginacion(id_patota, patota->memoria_ocupada, pos_y);
		patota->memoria_ocupada += sizeof(uint32_t);
		// log_info(logger, "IP");
		actualizar_entero_paginacion(id_patota, patota->memoria_ocupada, 0);
		patota->memoria_ocupada += sizeof(uint32_t);
		// log_info(logger, "pcb pointer");
		actualizar_entero_paginacion(id_patota, patota->memoria_ocupada, inicio_marco_logico(patota->frames[0]));
		patota->memoria_ocupada += sizeof(uint32_t);
	}
	log_info(logger, "CREO ESTRUCTURA TRIPULANTE");
	// CREO ESTRUCTURA TRIPULANTE PARA GUARDAR EN TABLA
	trip_data* nuevo_trip = malloc(sizeof(trip_data));
	nuevo_trip->PID = id_patota;
	nuevo_trip->TID = id_trip;
	// log_info(logger, "Realloc");
	// if(patota->cantidad_elementos - 2 < id_trip) {
	// 	patota->inicio_elementos = realloc(patota->inicio_elementos, sizeof(uint32_t *) * (id_trip + 2));
	// 	patota->cantidad_elementos = id_trip + 2;
	// }
	patota->inicio_elementos[id_trip + 1] = inicio_tcb;
	nuevo_trip->inicio = inicio_tcb;
	// patota->cantidad_elementos++;
	sem_wait(&mutex_lista_tripulantes);
	list_add(lista_tripulantes, nuevo_trip);
	sem_post(&mutex_lista_tripulantes);

	int socket_nuevo = crear_conexion_servidor(IP_RAM, 0, 1);
	pthread_t* hilo_nuevo = malloc(sizeof(pthread_t));
	nuevo_trip->socket = socket_nuevo;
	nuevo_trip->hilo = hilo_nuevo;
	
	log_info(logger, "CREO ESTRUCTURA TRIPULANTE PARA GUARDAR EN TABLA");
	pthread_create(hilo_nuevo, NULL, rutina_hilos, (void *)nuevo_trip);
	log_info(logger, "CREO HILO");
	// pthread_detach(*hilo_nuevo);
	if(CONSOLA_ACTIVA) {
		log_info(logger, "Entro a crear movimiento");
		t_movimiento* nuevo_movimiento = malloc(sizeof(t_movimiento));
		nuevo_movimiento->PID = id_patota;
		nuevo_movimiento->TID = id_trip;
		nuevo_movimiento->pos_x = pos_x;
		nuevo_movimiento->pos_y = pos_y;
		nuevo_movimiento->seguir = true;
		sem_wait(&mutex_movimiento);
		list_add(movimientos_pendientes, nuevo_movimiento);
		sem_post(&mutex_movimiento);
		sem_post(&semaforo_consola);
	}

	return puerto_desde_socket(socket_nuevo);
}

void eliminar_tripulante(uint32_t id_patota, uint32_t id_tripulante) {
	int posicion_de_lista = posicion_trip(id_patota, id_tripulante);
	if(posicion_de_lista == -1) {
		// printf("Posicion de lista -1\n");
		return;
	}
	log_info(logger, "Entro a eliminar_tripulante %d de la patota %d", id_tripulante, id_patota);
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	log_info(logger, "Mi patota");
	uint32_t inicio_tripulante = mi_patota->inicio_elementos[id_tripulante + 1];
	
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		log_info(logger, "Nro segmento tripulante: %d", nro_segmento_tripulante(inicio_tripulante));
		eliminar_segmento(nro_segmento_tripulante(inicio_tripulante));
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		
	}

	trip_data* trip_to_kill = (trip_data *)list_get(lista_tripulantes, posicion_de_lista);
	log_info(logger, "Entro a quitar de la lista");

	sem_wait(&mutex_lista_tripulantes);
	list_remove(lista_tripulantes, posicion_de_lista);
	sem_post(&mutex_lista_tripulantes);

	pthread_cancel(*trip_to_kill->hilo);
	pthread_join(*trip_to_kill->hilo, NULL);
	log_info(logger, "Pthread cancel");
	// printf("Recibi los restos del tripulante\n");
	liberar_tripulante(trip_to_kill);
	log_info(logger, "Libero tripulante");

	if(CONSOLA_ACTIVA) {
		t_movimiento* nuevo_movimiento = malloc(sizeof(t_movimiento));
		nuevo_movimiento->PID = id_patota;
		nuevo_movimiento->TID = id_tripulante;
		nuevo_movimiento->seguir = false;
		sem_wait(&mutex_movimiento);
		list_add(movimientos_pendientes, nuevo_movimiento);
		sem_post(&mutex_movimiento);
		sem_post(&semaforo_consola);
	}
}

uint32_t obtener_valor_tripulante(uint32_t id_patota, uint32_t id_trip, uint32_t nro_parametro) {
	uint32_t valor_tripulante;
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	uint32_t inicio_tripulante = mi_patota->inicio_elementos[id_trip + 1];
	// log_info(logger, "Obtener valor tripulante");
	switch(nro_parametro) {
		case PCB_POINTER:	inicio_tripulante += sizeof(uint32_t);
		case INS_POINTER:	inicio_tripulante += sizeof(uint32_t);
		case POS_Y:			inicio_tripulante += sizeof(uint32_t);
		case POS_X:			inicio_tripulante += sizeof(uint32_t);
		case ESTADO:		inicio_tripulante += sizeof(char);
		case TRIP_IP:
			break;
	}
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		memcpy(&valor_tripulante, memoria_ram.inicio + inicio_tripulante, sizeof(uint32_t));
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		valor_tripulante = obtener_entero_paginacion(id_patota, inicio_tripulante);
	}
	return valor_tripulante;
}

char obtener_estado(uint32_t id_patota, uint32_t id_tripulante) {
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	uint32_t inicio_logico_tripulante = mi_patota->inicio_elementos[id_tripulante + 1];
	char valor_char;
	void* inicio_tripulante;
	t_marco* marco_auxiliar;
	
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		// inicio_tripulante = inicio_logico_tripulante;
		inicio_tripulante = memoria_ram.inicio + inicio_logico_tripulante;
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		div_t posicion_compuesta = div(inicio_logico_tripulante, memoria_ram.tamanio_pagina);
		uint32_t pagina_actual = posicion_compuesta.quot;
		marco_auxiliar = memoria_ram.mapa_logico[mi_patota->frames[pagina_actual]];
		sem_wait(&marco_auxiliar->semaforo_mutex);
		if(memoria_ram.mapa_logico[mi_patota->frames[posicion_compuesta.quot]]->presencia == false) {
			incorporar_marco(mi_patota->frames[posicion_compuesta.quot]);
		}
		inicio_tripulante = inicio_marco(posicion_compuesta.quot) + posicion_compuesta.rem;
	}
	memcpy(&valor_char, inicio_tripulante + sizeof(uint32_t), sizeof(char));
	if(memoria_ram.esquema_memoria == PAGINACION)	sem_post(&marco_auxiliar->semaforo_mutex);
	return valor_char;
}

void actualizar_valor_tripulante(uint32_t id_patota, uint32_t id_trip, para_trip nro_parametro, uint32_t nuevo_valor) {
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	uint32_t inicio_tripulante = mi_patota->inicio_elementos[id_trip + 1];
	switch(nro_parametro) {
		case PCB_POINTER:	inicio_tripulante += sizeof(uint32_t);
		case INS_POINTER:	inicio_tripulante += sizeof(uint32_t);
		case POS_Y:			inicio_tripulante += sizeof(uint32_t);
		case POS_X:			inicio_tripulante += sizeof(uint32_t);
		case ESTADO:		inicio_tripulante += sizeof(char);
		case TRIP_IP:
			break;
	}
	
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		uint32_t valor_tripulante = nuevo_valor;
		memcpy(memoria_ram.inicio + inicio_tripulante, &valor_tripulante, sizeof(uint32_t));
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		// div_t posicion_compuesta = div(inicio_tripulante, memoria_ram.tamanio_pagina);
		// uint32_t bytes_de_valor = memoria_ram.tamanio_pagina - posicion_compuesta.rem;
		// if(bytes_de_valor > 4) { bytes_de_valor = 4;}
		// // semaforo
		// memcpy(inicio_marco(mi_patota->frames[posicion_compuesta.quot]) + posicion_compuesta.rem, &valor_tripulante, bytes_de_valor);
		// // memoria_ram.
		// if(bytes_de_valor < 4) {
		// 	memcpy(inicio_marco(mi_patota->frames[posicion_compuesta.quot + 1]), &valor_tripulante + bytes_de_valor, 4 - bytes_de_valor);
		// }
		// // semaforo
		actualizar_entero_paginacion(id_patota, inicio_tripulante, nuevo_valor);
	}
}

void actualizar_estado(uint32_t id_patota, uint32_t id_tripulante, char nuevo_valor) {
	if(nuevo_valor != 0) {
		patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
		uint32_t inicio_logico_tripulante = mi_patota->inicio_elementos[id_tripulante + 1];
		t_marco* marco_auxiliar;

		void* inicio_tripulante;
		if(memoria_ram.esquema_memoria == SEGMENTACION) {
			inicio_tripulante = memoria_ram.inicio + inicio_logico_tripulante;
		}
		if(memoria_ram.esquema_memoria == PAGINACION) {
			div_t posicion_compuesta = div(inicio_logico_tripulante, memoria_ram.tamanio_pagina);
			uint32_t pagina_actual = posicion_compuesta.quot;
			marco_auxiliar = memoria_ram.mapa_logico[mi_patota->frames[pagina_actual]];
			sem_wait(&marco_auxiliar->semaforo_mutex);
			if(memoria_ram.mapa_logico[mi_patota->frames[pagina_actual]]->presencia == false) {
				incorporar_marco(mi_patota->frames[posicion_compuesta.quot]);
			}
			inicio_tripulante = inicio_marco(posicion_compuesta.quot) + posicion_compuesta.rem;
		}
		char valor = nuevo_valor;
		memcpy(inicio_tripulante + sizeof(uint32_t), &valor, sizeof(char));
		if(memoria_ram.esquema_memoria == PAGINACION)	sem_post(&marco_auxiliar->semaforo_mutex);
	}
}

trip_data* tripulante_de_lista(uint32_t id_patota, uint32_t id_trip) {
	return (trip_data *)list_get(lista_tripulantes, posicion_trip(id_patota, id_trip));
}

int posicion_trip(uint32_t id_patota, uint32_t id_trip) {
	int posicion = -1;
	bool encontre = false;
	t_link_element* iterador_tripulante = lista_tripulantes->head;
	trip_data* trip_auxiliar;
	while(iterador_tripulante) {
		posicion++;
		trip_auxiliar = (trip_data *)iterador_tripulante->data;
		if(trip_auxiliar->PID == id_patota && trip_auxiliar->TID == id_trip) {
			encontre = true;
			break;
		}
		iterador_tripulante = iterador_tripulante->next;
	}
	
	if(encontre)
		return posicion;
	else
		return -1;
}

void liberar_tripulante(trip_data* trip_to_kill) {
    close(trip_to_kill->socket);
	sem_close(trip_to_kill->semaforo_hilo);
	free(trip_to_kill->semaforo_hilo);
	free(trip_to_kill->hilo);
	free(trip_to_kill);
}

uint32_t nro_segmento_tripulante(uint32_t inicio_tripulante) {
	t_link_element* iterador = memoria_ram.mapa_segmentos->head;
	while(((t_segmento *)iterador->data)->inicio != inicio_tripulante) {
		iterador = iterador->next;
	}
	return ((t_segmento *)iterador->data)->n_segmento;
}

uint32_t creo_segmento_tcb(uint32_t tamanio_tcb, uint32_t id_patota, uint32_t id_trip, uint32_t pos_x, uint32_t pos_y, uint32_t inicio_patota) {
	log_info(logger, "Creo segmento tcb");
	t_segmento* segmento_tcb = crear_segmento(TAMANIO_TRIPULANTE);
	segmento_tcb->duenio = id_patota;
	segmento_tcb->indice = id_trip + 1;
	log_info(logger, "Entro a segmentar");
	uint32_t desplazamiento = 0;
	segmentar_entero(segmento_tcb->inicio + desplazamiento, id_trip);
	desplazamiento += sizeof(uint32_t);
	segmentar_caracter(memoria_ram.inicio, segmento_tcb->inicio + desplazamiento, 'N');
	desplazamiento += sizeof(char);
	segmentar_entero(segmento_tcb->inicio + desplazamiento, pos_x);
	desplazamiento += sizeof(uint32_t);
	segmentar_entero(segmento_tcb->inicio + desplazamiento, pos_y);
	desplazamiento += sizeof(uint32_t);
	segmentar_entero(segmento_tcb->inicio + desplazamiento, 0);
	desplazamiento += sizeof(uint32_t);
	segmentar_entero(segmento_tcb->inicio + desplazamiento, inicio_patota);
	log_info(logger, "SEGMENTO TCB");

	return segmento_tcb->inicio;
}