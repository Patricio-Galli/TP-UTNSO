#include "tripulante.h"

uint32_t id_filtro_patota;

int iniciar_tripulante(uint32_t id_trip, uint32_t id_patota, uint32_t pos_x, uint32_t pos_y) {
	int tamanio_tcb = TAMANIO_TRIPULANTE;
	if (tamanio_tcb > memoria_libre()) {
		log_info(logger, "No hay memoria disponible");
		return 0;
	}
	log_info(logger, "Entro a iniciar_tripulante");
	// CREO SEGMENTO TCB. TO DO: agregar posibilidad de paginacion
	t_segmento* segmento_tcb = crear_segmento(TAMANIO_TRIPULANTE);
	segmento_tcb->duenio = id_patota;
	segmento_tcb->indice = id_trip + 1;

	patota_data* patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	log_info(logger, "CREO SEGMENTO TCB");
	// SEGMENTO TCB. TO DO: pasar funciones a segmentos.c y considerar la paginacion
	uint32_t desplazamiento = 0;
	segmentar_entero(memoria_ram, segmento_tcb->inicio + desplazamiento, id_trip);
	desplazamiento += sizeof(uint32_t);
	segmentar_caracter(memoria_ram, segmento_tcb->inicio + desplazamiento, 'N');
	desplazamiento += sizeof(char);
	segmentar_entero(memoria_ram, segmento_tcb->inicio + desplazamiento, pos_x);
	desplazamiento += sizeof(uint32_t);
	segmentar_entero(memoria_ram, segmento_tcb->inicio + desplazamiento, pos_y);
	desplazamiento += sizeof(uint32_t);
	segmentar_entero(memoria_ram, segmento_tcb->inicio + desplazamiento, 0);
	desplazamiento += sizeof(uint32_t);
	segmentar_entero(memoria_ram, segmento_tcb->inicio + desplazamiento, patota->tabla_segmentos[0]);
	log_info(logger, "SEGMENTO TCB");

	// CREO ESTRUCTURA TRIPULANTE PARA GUARDAR EN TABLA
	trip_data* nuevo_trip = malloc(sizeof(trip_data));
	nuevo_trip->PID = id_patota;
	nuevo_trip->TID = id_trip;

	if(patota->tamanio_tabla - 2 < id_trip) {
		patota->tabla_segmentos = realloc(patota->tabla_segmentos, sizeof(uint32_t *) * (id_trip + 2));
		patota->tamanio_tabla = id_trip;
	}
	patota->tabla_segmentos[id_trip + 1] = segmento_tcb->inicio;
	nuevo_trip->inicio = segmento_tcb->inicio;
	patota->tamanio_tabla++;
	
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
	log_info(logger, "Entro a eliminar_tripulante");
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	
	uint32_t inicio_tripulante = mi_patota->tabla_segmentos[id_tripulante + 1];
	log_info(logger, "Nro segmento tripulante: %d", nro_segmento_tripulante(inicio_tripulante));
	// sem_wait(&mutex_segmentacion);
	eliminar_segmento(nro_segmento_tripulante(inicio_tripulante));
    // sem_post(&mutex_segmentacion);

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

uint32_t obtener_valor_tripulante(void* inicio, uint32_t nro_parametro) {
	uint32_t valor_int;
	switch(nro_parametro) {
		case TRIP_IP:
			memcpy(&valor_int, inicio, sizeof(uint32_t));
			break;
		case POS_X:
			memcpy(&valor_int, inicio + sizeof(uint32_t) + sizeof(char), sizeof(uint32_t));
			break;
		case POS_Y:
			memcpy(&valor_int, inicio + 2 * sizeof(uint32_t) + sizeof(char), sizeof(uint32_t));
			break;
		case INS_POINTER:
			memcpy(&valor_int, inicio + 3 * sizeof(uint32_t) + sizeof(char), sizeof(uint32_t));
			break;
		case PCB_POINTER:
			memcpy(&valor_int, inicio + 4 * sizeof(uint32_t) + sizeof(char), sizeof(uint32_t));
			break;
	}
	return valor_int;
}

char obtener_estado(void* segmento) {
	char valor_char;
	memcpy(&valor_char, segmento + sizeof(uint32_t), sizeof(char));
	return valor_char;
}

void actualizar_valor_tripulante(void* segmento, uint32_t nro_parametro, uint32_t nuevo_valor) {
	uint32_t valor_int = nuevo_valor;
	switch(nro_parametro) {
	case TRIP_IP:
		memcpy(segmento, &valor_int, sizeof(uint32_t));
		break;
	case POS_X:
		memcpy(segmento + sizeof(uint32_t) + sizeof(char), &valor_int, sizeof(uint32_t));
		break;
	case POS_Y:
		memcpy(segmento + 2 * sizeof(uint32_t) + sizeof(char), &valor_int, sizeof(uint32_t));
		break;
	case INS_POINTER:
		memcpy(segmento + 3 * sizeof(uint32_t) + sizeof(char), &valor_int, sizeof(uint32_t));
		break;
	case PCB_POINTER:
		memcpy(segmento + 4 * sizeof(uint32_t) + sizeof(char), &valor_int, sizeof(uint32_t));
		break;
	}
}

void actualizar_estado(void* segmento, char nuevo_valor) {
	char valor = nuevo_valor;
	memcpy(segmento + sizeof(uint32_t), &valor, sizeof(char));
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
	t_link_element* iterador = mapa_segmentos->head;
	while(((t_segmento *)iterador->data)->inicio != inicio_tripulante) {
		iterador = iterador->next;
	}
	return ((t_segmento *)iterador->data)->n_segmento;
}