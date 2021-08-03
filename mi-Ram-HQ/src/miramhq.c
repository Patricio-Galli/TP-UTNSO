#include "miramhq.h"

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
		for (int i = 0; i < (uint32_t)(memoria_ram.tamanio_swap / TAMANIO_PAGINA); i++) {
			// log_info(logger, "Pagina %d/Fisico: %d/Duenio: %d/Presencia: %d/Modificado: %d",
			// 	memoria_ram.mapa_fisico[i]->nro_virtual,
			// 	memoria_ram.mapa_fisico[i]->nro_real,
			// 	memoria_ram.mapa_fisico[i]->duenio,
			// 	memoria_ram.mapa_fisico[i]->presencia,
			// 	memoria_ram.mapa_fisico[i]->modificado
			// 	);
		}
	}
	
	log_info(logger, "Lista de patotas: %d", list_size(lista_patotas));
	
	uint32_t inicio;
	uint32_t pid;
	uint32_t tid;
	uint32_t pnt_tareas;
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		for(int i = 0; i < list_size(lista_patotas); i++) {
			log_info(logger, "Patota %d. PID: %d; Puntero a PCB: %d; Puntero a tareas: %d", i + 1,
				((patota_data *)(uint32_t)list_get(lista_patotas, i))->PID,
				((patota_data *)(uint32_t)list_get(lista_patotas, i))->inicio_elementos[0],
				((patota_data *)(uint32_t)list_get(lista_patotas, i))->inicio_elementos[1]);

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
			
			log_info(logger, "Marco: %d. Inicio marco %p", mi_patota->frames[0], inicio_marco(mi_patota->frames[0]));
			memcpy(&pid, inicio_marco(mi_patota->frames[0]), sizeof(uint32_t));
			memcpy(&pnt_tareas, inicio_marco(mi_patota->frames[0]) + sizeof(uint32_t), sizeof(uint32_t));
			log_info(logger, "PID: %d; Puntero a tareas: %d", pid, pnt_tareas);
			log_info(logger, "PID: %d; Puntero a tareas: %d", obtener_entero_paginacion(i + 1, 0), obtener_entero_paginacion(i + 1, 4));
		}
	}
	
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		log_info(logger, "Lista de tripulantes activos: %d", list_size(lista_tripulantes));
		for(int i = 0; i < lista_tripulantes->elements_count; i++) {
			inicio = ((trip_data *)(uint32_t)list_get(lista_tripulantes, i))->inicio;
			pid = ((trip_data *)(uint32_t)list_get(lista_tripulantes, i))->PID;
			tid = ((trip_data *)(uint32_t)list_get(lista_tripulantes, i))->TID;
			log_info(logger, "TID: %d; inicio: %d; estado: %c; pos_x: %d; pos_y: %d; IP: %d; Punt PCB: %d",
				obtener_valor_tripulante(pid, tid, TRIP_IP),
				inicio,
				obtener_estado(pid, tid),
				obtener_valor_tripulante(pid, tid, POS_X),
				obtener_valor_tripulante(pid, tid, POS_Y),
				obtener_valor_tripulante(pid, tid, INS_POINTER),
				obtener_valor_tripulante(pid, tid, PCB_POINTER)
				);
		}
	}
}

int main(void) {
	if(CONSOLA_ACTIVA)
		logger = log_create("miramhq.log", "Mi-RAM-HQ", 0, LOG_LEVEL_INFO);
	else
		logger = log_create("miramhq.log", "Mi-RAM-HQ", 1, LOG_LEVEL_INFO);
	
	t_config* config = config_create("miramhq.config");
	lista_patotas = list_create();
	lista_tareas = list_create();
	lista_tripulantes = list_create();
	movimientos_pendientes = list_create();

	if(!iniciar_memoria(config)) {
		log_info(logger, "FALLO EN EL ARCHIVO DE CONFIGURACIÓN");
		return 0;
	}
	
	sem_init(&semaforo_consola, 0, 0);
	sem_init(&mutex_movimiento, 0, 1);
	sem_init(&mutex_lista_tripulantes, 0, 1);

	bool* continuar_consola = malloc(sizeof(bool));
	pthread_t* hilo_consola;
	if(CONSOLA_ACTIVA) {
		log_info(logger, "CONSOLA ACTIVA");
		*continuar_consola = true;
		hilo_consola = iniciar_mapa(continuar_consola);
	}

	int server_fd = crear_conexion_servidor(IP_RAM,	config_get_int_value(config, "PUERTO"), 1);

	if(!validar_socket(server_fd, logger)) {
		close(server_fd);
		log_destroy(logger);
		// liberar_memoria(config, socket_discord, hilo_consola);
		return ERROR_CONEXION;
	}
	log_info(logger, "Servidor listo");
	int socket_discord = esperar_cliente(server_fd);
	close(server_fd);
	log_info(logger, "Conexión establecida con el discordiador");
	
	t_list* mensaje_in;
	t_mensaje* mensaje_out;
	
	bool inicio_correcto;
	bool conexion_activa_discord = true;

	uint32_t patota_actual = 0;
	uint32_t nro_tripulante;

	uint32_t id_trip;	// Maldito c
	uint32_t id_patota;	// Maldito c

	while(conexion_activa_discord == true) {
		log_info(logger, "Esperando información del discordiador");
		log_info(logger, "Rompe recibir mensaje?");
		mensaje_in = recibir_mensaje(socket_discord);
		log_info(logger, "Recibir mensaje");
		if (!validar_mensaje(mensaje_in, logger)) {
			liberar_mensaje_in(mensaje_in);
			log_info(logger, "Cliente desconectado dentro del while");
			close(server_fd);
			log_destroy(logger);
			// liberar_memoria(config, socket_discord, hilo_consola);
			return ERROR_CONEXION;
		}
		
		switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
		case INIT_P:
			log_info(logger, "Discordiador solicitó iniciar_patota");
			patota_actual++;
			inicio_correcto = iniciar_patota(patota_actual, mensaje_in);
			// log_info(logger, "Patota %d. Valores: %d-%d", id_patota, obtener_entero_paginacion(id_patota, 0), obtener_entero_paginacion(id_patota, 4));
			if(!inicio_correcto) {
				mensaje_out = crear_mensaje(NO_SPC);
				if(memoria_ram.esquema_memoria == PAGINACION) {
					liberar_memoria(config, socket_discord);
					return ERROR_CONEXION;
				}
				patota_actual--;
			}
				
			else {
				mensaje_out = crear_mensaje(TODOOK);
			}

			enviar_mensaje(socket_discord, mensaje_out);
			liberar_mensaje_out(mensaje_out);		// debe estar fuera del switch
			nro_tripulante = 1;
			// for(int i = 0; i < ((tareas_data *)list_get(lista_tareas, patota_actual - 1))->cant_tareas; i++) {
			// 	// log_info(logger, "Tarea %d: '%s'", i, obtener_tarea(memoria_ram.inicio + ((patota_data *)list_get(lista_patotas, patota_actual - 1))->inicio_elementos[1], (tareas_data *)list_get(lista_tareas, patota_actual - 1), i));
			// 	log_info(logger, "Tarea %d: '%s'", i, obtener_tarea(patota_actual, i));
			// 	log_info(logger, "Inicio tarea: %d, Tamanio tarea: %d", ((tareas_data *)list_get(lista_tareas, patota_actual - 1))->inicio_tareas[i], ((tareas_data *)list_get(lista_tareas, patota_actual - 1))->tamanio_tareas[i]);
			// }
			// patota_data * patota_log = (patota_data *)list_get(lista_patotas, patota_actual - 1);
			// log_info(logger, "Frames de patota: %d", patota_log->cant_frames);
			// log_info(logger, "Elementos de patota: %d", patota_log->cantidad_elementos);
			// log_info(logger, "Ocupada de patota: %d", patota_log->memoria_ocupada);
			// if(memoria_ram.esquema_memoria == PAGINACION) {
			// 	for(int i = 0; i < patota_log->cant_frames; i++) {
			// 		char* marco = malloc(TAMANIO_PAGINA + 1);
			// 		log_info(logger, "1");
			// 		memcpy(marco, inicio_marco(patota_log->frames[i]), TAMANIO_PAGINA);
			// 		log_info(logger, "1");
			// 		char final = '\0';
			// 		log_info(logger, "1");
			// 		memcpy(marco + TAMANIO_PAGINA, &final, 1);
			// 		log_info(logger, "Marco completo:");
			// 		log_info(logger, "%s", marco);
			// 	}
			// }
			// printf("Tarea 1: '%s' no me la container\n", memoria_ram.inicio + ((patota_data *)list_get(lista_patotas, patota_actual - 1))->inicio_elementos[1]);
			break;
		case INIT_T:
			log_info(logger, "Discordiador solicitó iniciar_tripulante");
			uint32_t posicion_x = (uint32_t)list_get(mensaje_in, 1);
			uint32_t posicion_y = (uint32_t)list_get(mensaje_in, 2);
			log_info(logger, "Entro a iniciar_tripulante");
			int puerto = iniciar_tripulante(nro_tripulante, patota_actual, posicion_x, posicion_y);
			log_info(logger, "Resolvi iniciar_tripulante");
			if(puerto == 0) {
				mensaje_out = crear_mensaje(NO_SPC);
			}
			else {
				mensaje_out = crear_mensaje(SND_PO);
				agregar_parametro_a_mensaje(mensaje_out, (void *)puerto, ENTERO);
			}

			enviar_mensaje(socket_discord, mensaje_out);
			liberar_mensaje_out(mensaje_out);
			nro_tripulante++;
			break;
		case ELIM_T:
			log_info(logger, "Discordiador solicitó expulsar_tripulante");
			id_trip = (uint32_t)list_get(mensaje_in, 1);
			id_patota = (uint32_t)list_get(mensaje_in, 2);
			eliminar_tripulante(id_patota, id_trip);
			mensaje_out = crear_mensaje(TODOOK);
			enviar_mensaje(socket_discord, mensaje_out);
			liberar_mensaje_out(mensaje_out);
			break;
		case 64:
			log_info(logger, "Cliente desconectado 64");
			conexion_activa_discord = false;
			break;
		default:
			log_info(logger, "Cliente desconectado default");
			conexion_activa_discord = false;
			break;
		}
		liberar_mensaje_in(mensaje_in);
		loggear_data(/*logger*/);
	}
    log_info(logger, "Paso a cambiar continuar_consola");
	*continuar_consola = false;
	sem_post(&semaforo_consola);

	liberar_memoria(config, socket_discord);
	// log_info(logger, "Segmentos");
	// liberar_segmentos();
	// log_info(logger, "Patotas");
	// liberar_patotas();
	// log_info(logger, "Tareas");    // Rompe
	// // liberar_tareas();
	// log_info(logger, "Tripulantes");
	// liberar_tripulantes();
	// log_info(logger, "Consola");
	if(CONSOLA_ACTIVA) {
		pthread_join(*hilo_consola, 0);
		free(hilo_consola);
	}
	// log_info(logger, "Recibi la consola");
	// config_destroy(config);
	// log_destroy(logger);
	// close(socket_discord);
	
	return EXIT_SUCCESS;
}

void liberar_segmentos() {
	list_destroy(memoria_ram.mapa_segmentos);
	// void destruir_segmento(void* segmento) {
	// 	free(segmento);
	// }
	
	// list_destroy_and_destroy_elements(lista_patotas, destruir_segmento);
}

void liberar_patotas() {
	void destruir_patota(void* patota) {
		free(((patota_data *)patota)->inicio_elementos);
		free(patota);
	}
	
	list_destroy_and_destroy_elements(lista_patotas, destruir_patota);
}

void liberar_tareas() {
	void destruir_tarea(void* tarea) {
		free(((tareas_data *)tarea)->inicio_tareas);
		free(((tareas_data *)tarea)->tamanio_tareas);
		free(tarea);
	}

	list_destroy_and_destroy_elements(lista_patotas, destruir_tarea);
}

void liberar_tripulantes() {
	trip_data* trip_aux;
	for(int i = 0; i < list_size(lista_tripulantes); i++) {
		sem_wait(&mutex_lista_tripulantes);
		trip_aux = list_remove(lista_tripulantes, 0);
		sem_post(&mutex_lista_tripulantes);
		
		// trip_aux = list_remove(lista_tripulantes, 0);
		pthread_cancel(*trip_aux->hilo);
		pthread_join(*trip_aux->hilo, NULL);
	}
	list_destroy(lista_tripulantes);
	// void destruir_tripulante(void* tripulante) {
	// 	free(((trip_data *)tripulante)->semaforo_hilo);
	// 	free(((trip_data *)tripulante)->hilo);
	// 	free(tripulante);
	// }
	// list_destroy_and_destroy_elements(lista_patotas, destruir_tripulante);
}

pthread_t* iniciar_mapa(bool* continuar_consola) {
	pthread_t* hilo_consola = malloc(sizeof(pthread_t));
	pthread_create(hilo_consola, NULL, dibujar_mapa, (void *)continuar_consola);
	return hilo_consola;
}

bool iniciar_memoria_segmentada(t_config* config) {
	// sem_init(&mutex_segmentacion, 0, 1);
	memoria_ram.esquema_memoria = SEGMENTACION;
	memoria_ram.mapa_segmentos = list_create();
	t_segmento* segmento_memoria = malloc(sizeof(t_segmento));
	segmento_memoria->n_segmento = 0;
	segmento_memoria->duenio = 0;
	segmento_memoria->inicio = 0;
	segmento_memoria->tamanio = memoria_ram.tamanio_memoria;
	list_add(memoria_ram.mapa_segmentos, segmento_memoria);

	if(!strcmp(config_get_string_value(config, "CRITERIO_SELECCION"), "FF"))
		memoria_ram.criterio_seleccion = FIRST_FIT;
	if(!strcmp(config_get_string_value(config, "CRITERIO_SELECCION"), "BF"))
		memoria_ram.criterio_seleccion = BEST_FIT;
	return true;
}

bool iniciar_memoria_paginada(t_config* config) {
	memoria_ram.esquema_memoria = PAGINACION;
	memoria_ram.tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");
	memoria_ram.tamanio_swap = config_get_int_value(config, "TAMANIO_SWAP");
	uint32_t frames_en_memoria = memoria_ram.tamanio_memoria / TAMANIO_PAGINA;
	uint32_t frames_totales = (/*memoria_ram.tamanio_memoria + */memoria_ram.tamanio_swap) / TAMANIO_PAGINA;
	if(memoria_ram.tamanio_memoria % TAMANIO_PAGINA + memoria_ram.tamanio_swap % TAMANIO_PAGINA > 0)
		return false;
	
	log_info(logger, "Estoy en paginación, con entrada valida. Nro frames: %d:%d", frames_en_memoria, frames_totales);
	memoria_ram.mapa_fisico = calloc(frames_en_memoria, memoria_ram.tamanio_pagina);
	// memset(memoria_ram.mapa_fisico, 0, sizeof(memoria_ram.mapa_fisico));
	memoria_ram.mapa_logico = calloc(frames_totales, memoria_ram.tamanio_pagina);
	// memset(memoria_ram.mapa_logico, 0, sizeof(memoria_ram.mapa_logico));
	for(int i = 0; i < frames_totales; i++) {
		t_marco* marco_auxiliar = malloc(sizeof(t_marco));
		memoria_ram.mapa_logico[i] = marco_auxiliar;
		marco_auxiliar->nro_virtual = i;
		if(i < frames_en_memoria) {
			memoria_ram.mapa_fisico[i] = marco_auxiliar;
			marco_auxiliar->nro_real = i;
		}
		else
			marco_auxiliar->nro_real = 0;
		marco_auxiliar->duenio = 0;
		marco_auxiliar->presencia = true;
		marco_auxiliar->modificado = false;
	}
	if(!strcmp(config_get_string_value(config, "ALGORITMO_REEMPLAZO"), "LRU"))
		memoria_ram.algoritmo_reemplazo = LRU;
	if(!strcmp(config_get_string_value(config, "ALGORITMO_REEMPLAZO"), "CLOCK"))
		memoria_ram.algoritmo_reemplazo = CLOCK;     
	memoria_ram.inicio_swap = fopen(config_get_string_value(config, "PATH_SWAP"), "wb");
	char memoria_vacia[memoria_ram.tamanio_swap];
	memset(memoria_vacia, 0, memoria_ram.tamanio_swap);
	fwrite(memoria_vacia, 1, memoria_ram.tamanio_swap, memoria_ram.inicio_swap);
	return true;
}

bool iniciar_memoria(t_config* config) {
	memoria_ram.tamanio_memoria = config_get_int_value(config, "TAMANIO_MEMORIA");
	memoria_ram.inicio = malloc(memoria_ram.tamanio_memoria);
	char *esquema_memoria = config_get_string_value(config, "ESQUEMA_MEMORIA");
	bool inicio_correcto = false;
	if(!strcmp(esquema_memoria, "SEGMENTACION")) {
		inicio_correcto = iniciar_memoria_segmentada(config);
	}
	if(!strcmp(esquema_memoria, "PAGINACION")) {
		inicio_correcto = iniciar_memoria_paginada(config);
	}
	free(esquema_memoria);
	return inicio_correcto;
}

void liberar_memoria(t_config* config, int socket_discord) {
	log_info(logger, "Segmentos");
	liberar_segmentos();
	log_info(logger, "Patotas");
	liberar_patotas();
	log_info(logger, "Tareas");    // Rompe
	// liberar_tareas();
	log_info(logger, "Tripulantes");
	liberar_tripulantes();
	log_info(logger, "Consola");
	log_info(logger, "Recibi la consola");
	config_destroy(config);
	log_destroy(logger);
	close(socket_discord);
}