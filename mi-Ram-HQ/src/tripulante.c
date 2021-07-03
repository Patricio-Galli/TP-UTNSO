#include "tripulante.h"

bool iniciar_tripulante(uint32_t id_trip, uint32_t id_patota, uint32_t pos_x, uint32_t pos_y, algoritmo_segmento algoritmo) {
	int tamanio_tcb = TAMANIO_TRIPULANTE;
	if (tamanio_tcb > memoria_libre) {
		return false;
	}
	
	// CREO SEGMENTO PCB
	t_segmento* segmento_tcb = crear_segmento(mapa_segmentos, TAMANIO_TRIPULANTE, algoritmo);
	if(segmento_tcb == NULL) {
		// uint32_t final_memoria = realizar_compactacion();
		segmento_tcb = crear_segmento(mapa_segmentos, TAMANIO_TRIPULANTE, algoritmo);
	}
	segmento_tcb->duenio = id_patota;
	
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
	desplazamiento += sizeof(uint32_t);
	memoria_libre -= desplazamiento;
	
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