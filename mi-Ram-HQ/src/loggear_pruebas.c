#include "loggear_pruebas.h"

void loggear_patotas() {
    log_info(logger, "Lista de patotas: %d", list_size(lista_patotas));
	
	uint32_t inicio;
	uint32_t pid;
	uint32_t tid;
	uint32_t pnt_tareas;
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		for(int i = 0; i < list_size(lista_patotas); i++) {
			patota_data * mi_patota = (patota_data *)list_get(lista_patotas, i);
			log_info(logger, "Iteracion %d", i);
			log_info(logger, "Patota %d. PID: %d", i + 1, mi_patota->PID);
			log_info(logger, "Puntero a PCB: %d; Puntero a tareas: %d", mi_patota->inicio_elementos[0],	mi_patota->inicio_elementos[1]);

			inicio = ((patota_data *)(uint32_t)list_get(lista_patotas, i))->inicio_elementos[0];
			memcpy(&pid, memoria_ram.inicio + inicio, sizeof(uint32_t));
			memcpy(&pnt_tareas, memoria_ram.inicio + inicio + sizeof(uint32_t), sizeof(uint32_t));
			log_info(logger, "PID: %d; Puntero a tareas: %d", pid, pnt_tareas);
		}
	}

	if(memoria_ram.esquema_memoria == PAGINACION) {
		for(int i = 0; i < list_size(lista_patotas); i++) {
			patota_data * mi_patota = (patota_data *)list_get(lista_patotas, i);
			log_info(logger, "Patota %d. PID: %d; Puntero a PCB: %d; Puntero a tareas: %d", i + 1,
				mi_patota->PID,	mi_patota->inicio_elementos[0],	mi_patota->inicio_elementos[1]);
			
			// log_info(logger, "Marco: %d. Inicio marco %p", mi_patota->frames[0], inicio_marco(mi_patota->frames[0]));
			log_info(logger, "PID: %d; Puntero a tareas: %d", obtener_entero_paginacion(i + 1, 0), obtener_entero_paginacion(i + 1, 4));
		}
	}
}

void loggear_tripulantes() {
    uint32_t inicio;
	uint32_t pid;
	uint32_t tid;
    log_info(logger, "Lista de tripulantes activos: %d", list_size(lista_tripulantes));
	for(int i = 0; i < lista_tripulantes->elements_count; i++) {
		inicio = ((trip_data *)(uint32_t)list_get(lista_tripulantes, i))->inicio;
		pid = ((trip_data *)(uint32_t)list_get(lista_tripulantes, i))->PID;
		tid = ((trip_data *)(uint32_t)list_get(lista_tripulantes, i))->TID;
        // log_info(logger, "Loggero tripulante %d", i);
		log_info(logger, "TID: %d; inicio: %d; estado: %c; pos_x: %d; pos_y: %d; IP: %d; Punt PCB: %d",
			obtener_valor_tripulante(pid, tid, TRIP_IP),
			inicio,
			obtener_estado(pid, tid),
			// 'P',
			obtener_valor_tripulante(pid, tid, POS_X),
			obtener_valor_tripulante(pid, tid, POS_Y),
			obtener_valor_tripulante(pid, tid, INS_POINTER),
			obtener_valor_tripulante(pid, tid, PCB_POINTER)
			);
	}
}



void loggear_data() {
	log_info(logger, "NUEVOS RESULTADOS");
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		log_info(logger, "Cantidad de segmentos: %d. Memoria libre: %d", list_size(memoria_ram.mapa_segmentos), memoria_libre_segmentacion());
		for (int i = 0; i < list_size(memoria_ram.mapa_segmentos); i++) {
			log_info(logger, "SEGMENTO %d/Duenio: %d/Indice: %d/Inicio: %d/Tamanio: %d",
				((t_segmento *)list_get(memoria_ram.mapa_segmentos, i))->n_segmento + 1,
				((t_segmento *)list_get(memoria_ram.mapa_segmentos, i))->duenio,
				((t_segmento *)list_get(memoria_ram.mapa_segmentos, i))->indice,
				((t_segmento *)list_get(memoria_ram.mapa_segmentos, i))->inicio,
				((t_segmento *)list_get(memoria_ram.mapa_segmentos, i))->tamanio
				);
		}
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		log_info(logger, "Marcos libres fisicos: %d. Marcos libres logicos: %d", marcos_reales_disponibles(), marcos_logicos_disponibles());
		for (int i = 0; i < (uint32_t)(memoria_ram.tamanio_memoria / TAMANIO_PAGINA); i++) {
			log_info(logger, "Pagina %d/Fisico: %d/Duenio: %d/Presencia: %d/Modificado: %d",
				memoria_ram.mapa_fisico[i]->nro_virtual,
				memoria_ram.mapa_fisico[i]->nro_real,
				memoria_ram.mapa_fisico[i]->duenio,
				memoria_ram.mapa_fisico[i]->presencia,
				memoria_ram.mapa_fisico[i]->modificado
				);
		}
	}
    loggear_patotas();
    loggear_tripulantes();
}
