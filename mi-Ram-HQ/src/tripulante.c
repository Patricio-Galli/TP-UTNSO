#include "tripulante.h"

uint32_t id_filtro_patota;

int iniciar_tripulante(uint32_t id_trip, uint32_t id_patota, uint32_t pos_x, uint32_t pos_y, algoritmo_segmento algoritmo) {
	int tamanio_tcb = TAMANIO_TRIPULANTE;
	if (tamanio_tcb > memoria_libre()) {
		return 0;
	}
	
	// CREO SEGMENTO PCB
	t_segmento* segmento_tcb = crear_segmento(mapa_segmentos, TAMANIO_TRIPULANTE, algoritmo);
	if(segmento_tcb == NULL) {
		realizar_compactacion();
		segmento_tcb = crear_segmento(mapa_segmentos, TAMANIO_TRIPULANTE, algoritmo);
	}
	segmento_tcb->duenio = id_patota;
	segmento_tcb->indice = id_trip + 1;
	patota_data* patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	
	// SEGMENTO TCB
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
	
	// CREO ESTRUCTURA TRIPULANTE PARA GUARDAR EN TABLA
	trip_data* nuevo_trip = malloc(sizeof(trip_data));
	nuevo_trip->PID = id_patota;
	nuevo_trip->TID = id_trip;
	nuevo_trip->seguir = true;
	if(patota->tamanio_tabla - 2 < id_trip) {
		patota->tabla_segmentos = realloc(patota->tabla_segmentos, sizeof(uint32_t *) * (id_trip + 2));
		patota->tamanio_tabla = id_trip;
	}
	patota->tabla_segmentos[id_trip + 1] = segmento_tcb->inicio;
	nuevo_trip->inicio = segmento_tcb->inicio;
	patota->tamanio_tabla++;
	list_add(lista_tripulantes, nuevo_trip);

	int socket_nuevo = crear_conexion_servidor(IP_RAM, 0, 1);
	pthread_t* hilo_nuevo = malloc(sizeof(pthread_t));
	sem_t* semaforo = malloc(sizeof(sem_t));
	sem_init(semaforo, 0, 1);

	nuevo_trip->semaforo_hilo = semaforo;
	nuevo_trip->socket = socket_nuevo;
	nuevo_trip->hilo = hilo_nuevo;
	
	pthread_create(hilo_nuevo, NULL, rutina_hilos, (void *)nuevo_trip);
	pthread_detach(*hilo_nuevo);

	return puerto_desde_socket(socket_nuevo);
}

uint32_t obtener_valor_tripulante(void* segmento, uint32_t nro_parametro) {
	uint32_t valor_int;
	switch(nro_parametro) {
		case TRIP_IP:
			memcpy(&valor_int, segmento, sizeof(uint32_t));
			break;
		case POS_X:
			memcpy(&valor_int, segmento + sizeof(uint32_t) + sizeof(char), sizeof(uint32_t));
			break;
		case POS_Y:
			memcpy(&valor_int, segmento + 2 * sizeof(uint32_t) + sizeof(char), sizeof(uint32_t));
			break;
		case INS_POINTER:
			memcpy(&valor_int, segmento + 3 * sizeof(uint32_t) + sizeof(char), sizeof(uint32_t));
			break;
		case PCB_POINTER:
			memcpy(&valor_int, segmento + 4 * sizeof(uint32_t) + sizeof(char), sizeof(uint32_t));
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

void eliminar_tripulante(uint32_t id_patota, uint32_t id_tripulante) {
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	
	uint32_t inicio_tripulante = mi_patota->tabla_segmentos[id_tripulante + 1];
	t_link_element* iterador = mapa_segmentos->head;
	while(((t_segmento *)iterador->data)->inicio != inicio_tripulante) {
		iterador = iterador->next;
	}
	t_segmento* segmento_tripulante = (t_segmento *)iterador->data;
	eliminar_segmento(mapa_segmentos, segmento_tripulante);
	
	trip_data* mi_tripulante = tripulante_de_lista(id_patota, id_tripulante);
	mi_tripulante->seguir = false;
	
	// Falta actualizar trip_data, detener el hilo y
}

bool seg_trip_de_patota(void* segmento) {
	if(((t_segmento*)segmento)->duenio == id_filtro_patota && ((t_segmento*)segmento)->indice > 1) {
		return true;
	}
    else
        return false;
}

t_list* tripulantes_de_patota(uint32_t id_patota) {
	id_filtro_patota = id_patota;
	return list_filter(mapa_segmentos, (*seg_trip_de_patota));
}

trip_data* tripulante_de_lista(uint32_t id_patota, uint32_t id_trip) {
	t_link_element* iterador_patota = lista_patotas->head;
	t_link_element* iterador_tripulante = lista_tripulantes->head;

	patota_data* patota_aux;
	for(int iter1 = 1; iter1 < id_patota; iter1++) {
		patota_aux = (patota_data *)iterador_patota->data;
		for(int iter2 = 0; iter2 < patota_aux->tamanio_tabla - 1; iter2++) {
			iterador_tripulante = iterador_tripulante->next;
		}
		iterador_patota = iterador_patota->next;
	}
	for(int i = 1; i < id_trip; i++) { iterador_tripulante = iterador_tripulante->next; }
	return (trip_data *)iterador_tripulante->data;	
}

bool tripulante_activo(void * un_trip) {
	if(((trip_data *)un_trip)->seguir)
		return true;
	else
		return false;
}

uint32_t cantidad_tripulantes_activos() {
	return list_count_satisfying(lista_tripulantes, (*tripulante_activo));
}

t_list* tripulantes_activos() {
	return list_filter(lista_tripulantes, (*tripulante_activo));
}