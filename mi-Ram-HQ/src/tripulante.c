#include "tripulante.h"

uint32_t id_filtro_patota;

int iniciar_tripulante(uint32_t id_trip, uint32_t id_patota, uint32_t pos_x, uint32_t pos_y, algoritmo_segmento algoritmo) {
	int tamanio_tcb = TAMANIO_TRIPULANTE;
	if (tamanio_tcb > memoria_libre()) {
		return 0;
	}
	
	// CREO SEGMENTO PCB. TO DO: agregar posibilidad de pagincion
	t_segmento* segmento_tcb = crear_segmento(mapa_segmentos, TAMANIO_TRIPULANTE, algoritmo);
	if(segmento_tcb == NULL) {
		realizar_compactacion();
		segmento_tcb = crear_segmento(mapa_segmentos, TAMANIO_TRIPULANTE, algoritmo);
	}
	segmento_tcb->duenio = id_patota;
	segmento_tcb->indice = id_trip + 1;
	patota_data* patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	
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
	
	// CREO ESTRUCTURA TRIPULANTE PARA GUARDAR EN TABLA
	trip_data* nuevo_trip = malloc(sizeof(trip_data));
	nuevo_trip->PID = id_patota;
	nuevo_trip->TID = id_trip;
	nuevo_trip->seguir = true;
	nuevo_trip->posicion_x = pos_x;
	nuevo_trip->posicion_y = pos_y;

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
	// pthread_detach(*hilo_nuevo);

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
	int posicion_de_lista = posicion_trip(id_patota, id_tripulante);
	if(posicion_de_lista == -1) {
		printf("Posicion de lista -1\n");
		return;
	}
	printf("Posicion de lista %d\n", posicion_de_lista);
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	
	uint32_t inicio_tripulante = mi_patota->tabla_segmentos[id_tripulante + 1];
	// TO DO: hacer funcion auxiliar y hacer que eliminar_segmento tome la posicion y no el segmento en sí
	t_link_element* iterador = mapa_segmentos->head;
	while(((t_segmento *)iterador->data)->inicio != inicio_tripulante) {
		iterador = iterador->next;
	}
	t_segmento* segmento_tripulante = (t_segmento *)iterador->data;
	eliminar_segmento(mapa_segmentos, segmento_tripulante);

	trip_data* trip_to_kill = (trip_data *)list_get(lista_tripulantes, posicion_de_lista);
	
	if(CONSOLA_ACTIVA) {
		sem_init(trip_to_kill->eliminar_consola, 0, 0);
		trip_to_kill->modificado = true;
		trip_to_kill->seguir = false;
		sem_wait(trip_to_kill->eliminar_consola);
	}
	
	printf("Entro a remover de la lista\n");
	list_remove(lista_tripulantes, posicion_de_lista);
	printf("Removi trip de la lista y espero que finalize el trip\n");
	pthread_cancel(*trip_to_kill->hilo);
	pthread_join(*trip_to_kill->hilo, NULL);
	printf("Recibi los restos del tripulante\n");
	liberar_tripulante(trip_to_kill);
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

t_list* tripulantes_modificados() {
	bool tripulante_modificado(void* tripulante) {
		if(((trip_data *)tripulante)->modificado)
			return true;
		else
			return false;
	}

	return list_filter(lista_tripulantes, (*tripulante_modificado));
}

void liberar_tripulante(trip_data* trip_to_kill) {
	printf("Voy a liberar tripulante\n");
	sem_close(trip_to_kill->semaforo_hilo);
	free(trip_to_kill->semaforo_hilo);
	printf("kill sem\n");
	sem_close(trip_to_kill->eliminar_consola);
	// free(trip_to_kill->eliminar_consola);
	printf("kill sem\n");
	free(trip_to_kill->hilo);
	printf("free 1\n");
	free(trip_to_kill);
	printf("free 2\n");
}