#include "miramhq.h"

// int variable = 1;

void loggear_data(/*t_log* logger*/) {
	log_info(logger, "NUEVOS RESULTADOS");
	log_info(logger, "Cantidad de segmentos: %d. Memoria libre: %d", mapa_segmentos->elements_count, memoria_libre());
	for (int i = 0; i < mapa_segmentos->elements_count; i++) {
		log_info(logger, "SEGMENTO %d/Duenio: %d/Indice: %d/Inicio: %d/Tamanio: %d",
			((t_segmento *)list_get(mapa_segmentos, i))->n_segmento + 1,
			((t_segmento *)list_get(mapa_segmentos, i))->duenio,
			((t_segmento *)list_get(mapa_segmentos, i))->indice,
			((t_segmento *)list_get(mapa_segmentos, i))->inicio,
			((t_segmento *)list_get(mapa_segmentos, i))->tamanio
			);
	}
	log_info(logger, "Lista de patotas: %d", lista_patotas->elements_count);
	
	uint32_t inicio;
	uint32_t pid;
	uint32_t pnt_tareas;
	
	for(int i = 0; i < lista_patotas->elements_count; i++) {
		log_info(logger, "Patota %d. PID: %d; Puntero a PCB: %d; Puntero a tareas: %d", i + 1,
			((patota_data *)(uint32_t)list_get(lista_patotas, i))->PID,
			((patota_data *)(uint32_t)list_get(lista_patotas, i))->tabla_segmentos[0],
			((patota_data *)(uint32_t)list_get(lista_patotas, i))->tabla_segmentos[1]);

		inicio = ((patota_data *)(uint32_t)list_get(lista_patotas, i))->tabla_segmentos[0];
		memcpy(&pid, memoria_ram + inicio, sizeof(uint32_t));
		memcpy(&pnt_tareas, memoria_ram + inicio + sizeof(uint32_t), sizeof(uint32_t));
		log_info(logger, "PID: %d; Puntero a tareas: %d", pid, pnt_tareas);
	}

	log_info(logger, "Lista de tripulantes activos: %d", list_size(lista_tripulantes));
	for(int i = 0; i < lista_tripulantes->elements_count; i++) {
		inicio = ((trip_data *)(uint32_t)list_get(lista_tripulantes, i))->inicio;
		log_info(logger, "TID: %d; inicio: %d; estado: %c; pos_x: %d; pos_y: %d; IP: %d; Punt PCB: %d",
			obtener_valor_tripulante(memoria_ram + inicio, TRIP_IP),
			inicio,
			obtener_estado(memoria_ram + inicio),
			obtener_valor_tripulante(memoria_ram + inicio, POS_X),
			obtener_valor_tripulante(memoria_ram + inicio, POS_Y),
			obtener_valor_tripulante(memoria_ram + inicio, INS_POINTER),
			obtener_valor_tripulante(memoria_ram + inicio, PCB_POINTER)
			);
	}
}

int main(void) {
	// t_log* logger;
	if(CONSOLA_ACTIVA)
		logger = log_create("miramhq.log", "Mi-RAM-HQ", 0, LOG_LEVEL_INFO);
	else
		logger = log_create("miramhq.log", "Mi-RAM-HQ", 1, LOG_LEVEL_INFO);
	
	t_config* config = config_create("miramhq.config");
	lista_patotas = list_create();
	lista_tareas = list_create();
	lista_tripulantes = list_create();
	movimientos_pendientes = list_create();

	iniciar_memoria(config);

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
	data_socket(server_fd, logger);

	if(!validar_socket(server_fd, logger)) {
		close(server_fd);
		log_destroy(logger);
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
		mensaje_in = recibir_mensaje(socket_discord);
		if (!validar_mensaje(mensaje_in, logger)) {
			liberar_mensaje_in(mensaje_in);
			log_info(logger, "Cliente desconectado dentro del while");
			close(server_fd);
			log_destroy(logger);
			return ERROR_CONEXION;
		}
		
		switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
		case INIT_P:
			log_info(logger, "Discordiador solicitó iniciar_patota");
			patota_actual++;
			inicio_correcto = iniciar_patota(patota_actual, mensaje_in);
			
			if(!inicio_correcto) {
				mensaje_out = crear_mensaje(NO_SPC);
				patota_actual--;
			}
				
			else {
				mensaje_out = crear_mensaje(TODOOK);
			}

			enviar_mensaje(socket_discord, mensaje_out);
			liberar_mensaje_out(mensaje_out);		// debe estar fuera del switch
			nro_tripulante = 1;
			break;
		case INIT_T:
			log_info(logger, "Discordiador solicitó iniciar_tripulante");
			uint32_t posicion_x = (uint32_t)list_get(mensaje_in, 1);
			uint32_t posicion_y = (uint32_t)list_get(mensaje_in, 2);
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
		// loggear_data(/*logger*/);
	}
    log_info(logger, "Paso a cambiar continuar_consola");
	*continuar_consola = false;
	sem_post(&semaforo_consola);

	log_info(logger, "Segmentos");
	liberar_segmentos();
	log_info(logger, "Patotas");
	liberar_patotas();
	log_info(logger, "Tareas");    // Rompe
	// liberar_tareas();
	log_info(logger, "Tripulantes");
	liberar_tripulantes();
	log_info(logger, "Consola");
	if(CONSOLA_ACTIVA) {
		pthread_join(*hilo_consola, 0);
		free(hilo_consola);
	}
	log_info(logger, "Recibi la consola");
	config_destroy(config);
	log_destroy(logger);
	close(socket_discord);
	
	return EXIT_SUCCESS;
}

void liberar_segmentos() {
	list_destroy(mapa_segmentos);
	// void destruir_segmento(void* segmento) {
	// 	free(segmento);
	// }
	
	// list_destroy_and_destroy_elements(lista_patotas, destruir_segmento);
}

void liberar_patotas() {
	void destruir_patota(void* patota) {
		free(((patota_data *)patota)->tabla_segmentos);
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

void iniciar_memoria(t_config* config) {
	tamanio_memoria = config_get_int_value(config, "TAMANIO_MEMORIA");
	memoria_ram = malloc(tamanio_memoria);
	esquema_memoria = config_get_string_value(config, "ESQUEMA_MEMORIA");
	// sem_init(&mutex_segmentacion, 0, 1);
	if(!strcmp(esquema_memoria, "SEGMENTACION")) {
		mapa_segmentos = list_create();
		t_segmento* segmento_memoria = malloc(sizeof(t_segmento));
		segmento_memoria->n_segmento = 0;
		segmento_memoria->duenio = 0;
		segmento_memoria->inicio = 0;
		segmento_memoria->tamanio = tamanio_memoria;
		list_add(mapa_segmentos, segmento_memoria);

		if(!strcmp(config_get_string_value(config, "CRITERIO_SELECCION"), "FF"))
			algoritmo_seg_memoria = FF;
		if(!strcmp(config_get_string_value(config, "CRITERIO_SELECCION"), "BF"))
			algoritmo_seg_memoria = BF;
	}
	if(!strcmp(esquema_memoria, "PAGINACION")) {
		/* TO DO */
	}
	free(esquema_memoria);
}