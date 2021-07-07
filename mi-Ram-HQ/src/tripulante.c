#include "tripulante.h"

uint32_t id_filtro_patota;

bool iniciar_tripulante(uint32_t id_trip, uint32_t id_patota, uint32_t pos_x, uint32_t pos_y, algoritmo_segmento algoritmo) {
	printf("Entro a iniciar_tripulante\n");
	int tamanio_tcb = TAMANIO_TRIPULANTE;
	if (tamanio_tcb > memoria_libre()) {
		printf("Memoria insuficiente\n");
		return false;
	}
	printf("Voy a crear_segmento()\n");
	// CREO SEGMENTO PCB
	t_segmento* segmento_tcb = crear_segmento(mapa_segmentos, TAMANIO_TRIPULANTE, algoritmo);
	if(segmento_tcb == NULL) {
		printf("segmento_tcb == NULL\n");
		realizar_compactacion();
		segmento_tcb = crear_segmento(mapa_segmentos, TAMANIO_TRIPULANTE, algoritmo);
	}
	printf("Sobrevivi a crear_segmento()\n");
	segmento_tcb->duenio = id_patota;
	printf("1\n");
	segmento_tcb->indice = id_trip + 1;
	printf("2\n");
	patota_data* patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	printf("Sobrevivi a crear_segmento()\n");
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
	printf("Sobrevivi a segmento_pcb()\n");
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
	patota->trip_activos++;
	nuevo_trip->inicio = segmento_tcb->inicio;
	patota->tamanio_tabla++;
	list_add(lista_tripulantes, nuevo_trip);
	return true;
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
	printf("Entro a eliminar_tripulante\n");
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	// mi_patota->trip_activos--;
	// trip_data* mi_tripulante = list_filter(lista_tripulantes, (*soy))

	uint32_t inicio_tripulante = mi_patota->tabla_segmentos[id_tripulante + 1];
	t_link_element* iterador = mapa_segmentos->head;
	while(((t_segmento *)iterador->data)->inicio != inicio_tripulante) {
		iterador = iterador->next;
	}
	printf("4\n");
	t_segmento* segmento_tripulante = (t_segmento *)iterador->data;
	printf("Entro a eliminar segmento\n");
	eliminar_segmento(mapa_segmentos, segmento_tripulante);
	printf("Sobrevivi a eliminar segmento\n");
	
	t_link_element* iterador_patota = lista_patotas->head;
	t_link_element* iterador_tripulante = lista_tripulantes->head;

	patota_data* patota_aux;
	for(int iter1 = 1; iter1 < id_patota; iter1++) {
		patota_aux = (patota_data *)iterador_patota->data;
		for(int iter2 = 0; iter2 < patota_aux->trip_activos; iter2++) {
			iterador_tripulante = iterador_tripulante->next;
		}
		iterador_patota = iterador_patota->next;
	}
	for(int i = 1; i < id_tripulante; i++) { iterador_tripulante = iterador_tripulante->next; }
	trip_data* mi_tripulante = ((trip_data *)iterador_tripulante->data);
	mi_tripulante->seguir = false;
	
	// Falta actualizar trip_data, detener el hilo y
}
/*
1 - 1 - 1 - 2 - 2 - 3 - 3 - 3 - 3 - 4 - 5 - 5
*/

/*
uint32_t id_global_tripulante(uint32_t id_patota, uint32_t id_tripulante) {
	t_link_element* aux_trip = lista_tripulantes->head;
	for(int iterador = 0; iterador < id_patota; iterador++) {

	}
	
}*/

bool seg_trip_de_patota(void* segmento) {
	if(((t_segmento*)segmento)->duenio == id_filtro_patota && ((t_segmento*)segmento)->indice > 1)
        return true;
    else
        return false;
}

t_list* tripulantes_de_patota(uint32_t id_patota) {
	uint32_t id_filtro_patota = id_patota;
	return list_filter(mapa_segmentos, (*seg_trip_de_patota));
}