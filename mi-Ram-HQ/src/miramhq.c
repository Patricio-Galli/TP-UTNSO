#include "miramhq.h"

int memoria_libre;
void* memoria_ram;
algoritmo_segmento algoritmo;

int main(void) {
	t_log* logger = log_create("miramhq.log", "Mi-RAM-HQ", 1, LOG_LEVEL_DEBUG);
	t_config* config = config_create("miramhq.config");
	
	int tamanio_memoria = config_get_int_value(config, "TAMANIO_MEMORIA");
	log_info(logger, "Iniciando memoria RAM de %d bytes", tamanio_memoria);
	
	if(!strcmp(config_get_string_value(config, "CRITERIO_SELECCION"), "FF"))
		algoritmo = FF;
	if(!strcmp(config_get_string_value(config, "CRITERIO_SELECCION"), "BF"))
		algoritmo = BF;

	memoria_ram = malloc(tamanio_memoria);
	memoria_libre = tamanio_memoria;

	t_list* mapa_segmentos = list_create();
	t_segmento segmento_memoria;
	segmento_memoria.n_segmento = 0;
	segmento_memoria.duenio = 0;
	segmento_memoria.inicio = 0;
	segmento_memoria.tamanio = tamanio_memoria;
	list_add(mapa_segmentos, &segmento_memoria);

	// dibujar_mapa(); VACÍO

	int server_fd = crear_conexion_servidor(
		IP_RAM,	config_get_int_value(config, "PUERTO"), 1);
	
	if(server_fd < 0) {
		log_info(logger, "Fallo en la conexión del servidor");
		close(server_fd);
		return ERROR_CONEXION;
	}

	log_info(logger, "Servidor listo");
	int socket_discord = esperar_cliente(server_fd);
	log_info(logger, "Conexión establecida con el discordiador");
	
	
	t_list* lista_patotas = list_create();
	t_list* lista_tripulantes = list_create();
	
	t_list* mensaje_discor;
	t_mensaje* respuesta;

	uint32_t patota_actual = 1;
	
	bool inicio_correcto;
	bool conexion_activa_discord = true;
	
	while(conexion_activa_discord == true) {
		log_info(logger, "Esperando información del discordiador");
		mensaje_discor = recibir_mensaje(socket_discord);
		
		switch((int)list_get(mensaje_discor, 0)) { // protocolo del mensaje
		case INIT_P:
			log_info(logger, "Discordiador solicitó iniciar_patota");
			
			inicio_correcto = iniciar_patota(logger, mensaje_discor, mapa_segmentos, lista_patotas, patota_actual);

			if(inicio_correcto == false) {
				log_error(logger, "No hay tamanio suficiente");
				respuesta = crear_mensaje(NO_SPC);
			}
			else {
				log_info(logger, "Patota creada correctamente");
				respuesta = crear_mensaje(TODOOK);
			}

			enviar_mensaje(socket_discord, respuesta);
			patota_actual++;
			log_info(logger, "Envío respuesta al discordiador");
			free(respuesta);				// debe estar fuera del switch
			list_destroy(mensaje_discor);	// debe estar fuera del switch
			break;
		case INIT_T:
			log_info(logger, "Discordiador solicitó iniciar_tripulante");
			
			inicio_correcto = iniciar_tripulante(logger, mensaje_discor, mapa_segmentos, lista_tripulantes);

			if(inicio_correcto == false) {
				log_error(logger, "No hay tamanio suficiente");
				respuesta = crear_mensaje(NO_SPC);
			}
			else {
				log_info(logger, "Patota creada correctamente");
				int socket_nuevo = crear_conexion_servidor(IP_RAM, 0, 1);
				
				pthread_t* hilo_nuevo = malloc(sizeof(pthread_t));
				pthread_create(hilo_nuevo, NULL, rutina_hilos, (void *)socket_nuevo);
				
				int nuevo_puerto = puerto_desde_socket(socket_nuevo);
				respuesta = crear_mensaje(SND_PO);
				agregar_parametro_a_mensaje(respuesta, (void *)nuevo_puerto, ENTERO);
			}
			enviar_mensaje(socket_discord, respuesta);
			liberar_mensaje(respuesta);
			log_info(logger, "Envío respuesta al discordiador");
			list_destroy(mensaje_discor);
			
			break;
		case ELIM_T:
			// elimino el tcb del tripulante correspondiente
			return 0;
			break;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			conexion_activa_discord = false;
			break;
		}
		log_info(logger, "Resultados");
		log_info(logger, "Lista de patotas: %d", lista_patotas->elements_count);
		log_info(logger, "Lista segmentos: %d", mapa_segmentos->elements_count);
		for (int i = 0; i < mapa_segmentos->elements_count; i++) {
			log_info(logger, "Segmento %d", i + 1);
			log_info(logger, "Duenio: %d", ((t_segmento *)list_get(mapa_segmentos, i))->duenio);
			log_info(logger, "Inicio: %d", ((t_segmento *)list_get(mapa_segmentos, i))->inicio);
			log_info(logger, "N segmento: %d", ((t_segmento *)list_get(mapa_segmentos, i))->n_segmento);
			log_info(logger, "Tamanio: %d", ((t_segmento *)list_get(mapa_segmentos, i))->tamanio);
		}
	}
	return EXIT_SUCCESS;
}

void* rutina_hilos(void* socket, t_tripulante* mi_tripulante) {
	t_log* logger = log_create("miramhq.log", "HILOX", 1, LOG_LEVEL_INFO);
	log_info(logger, "HOLA MUNDO, SOY UN HILO");
	
	int socket_cliente = esperar_cliente((int)socket);

	// t_mensaje* mensaje;
	// t_list* mensaje_discor = list_create();
	// mensaje_discor = recibir_mensaje(socket_cliente);
	
	/*char* tarea;
	// while(tripulante.estado != EXIT) {
	while(1) {
		mensaje_discor = recibir_mensaje(socket_cliente);
		switch((int)list_get(mensaje_discor, 0)) {
		case ACTU_T:

			break;
		case NEXT_T:
			char* tarea;
			int n_tarea;
			obtener_proxima_tarea(p_pcb, n_tarea);
			mensaje = crear_mensaje(TASK_T);
			agregar_parametro_a_mensaje(mensaje, tarea, BUFFER, logger);
			enviar_mensaje(socket_cliente, mensaje);
			free(mensaje);
			break;
		
		}
	}*/

	return 0;
}

void segmentar_pcb(t_segmento* segmento_pcb, uint32_t patota, t_segmento* segmento_tareas) {
	t_patota* nueva_patota = malloc(sizeof(t_patota));
	nueva_patota->PID = patota;
	nueva_patota->tareas = segmento_tareas->inicio;
	segmento_pcb->duenio = patota;
	memcpy(memoria_ram + segmento_pcb->inicio, nueva_patota, sizeof(t_patota));
	free(nueva_patota);
}

void segmentar_tareas(t_segmento* segmento_tareas, uint32_t patota, char** vector_tareas) {
	segmento_tareas->duenio = patota;
	memcpy(memoria_ram + segmento_tareas->inicio, vector_tareas, segmento_tareas->tamanio);
}

void segmentar_tcb(t_segmento* segmento_tcb, uint32_t patota, t_tripulante* nuevo_tripulante) {
	segmento_tcb->duenio = patota;
	memcpy(memoria_ram + segmento_tcb->inicio, nuevo_tripulante, sizeof(t_tripulante));
}

bool iniciar_patota(t_log* logger, t_list* parametros, t_list* mapa_segmentos, t_list* lista_patotas, int patota) {
	int tamanio_pcb = sizeof(t_patota);
	int tamanio_tareas = 0;
	
	int cantidad_tareas = (int)list_get(parametros, 3);
	char** vector_tareas = malloc(sizeof(char *) * cantidad_tareas);
	for(int i = 0; i < cantidad_tareas; i++) {
		char* tarea_i = (char *)list_get(parametros, 4 + i);
		tamanio_tareas += strlen(tarea_i) + 1;
		vector_tareas[i] = tarea_i;
	}

	log_info(logger, "Entro a encontrar bloque, memoria libre: %d", memoria_libre);
	log_info(logger, "tamanio_pcb = %d, tamanio_tareas = %d", tamanio_pcb, tamanio_tareas);
	if (tamanio_pcb + tamanio_tareas > memoria_libre) {
		return false;
	}
	log_info(logger, "Segmento PCB");
	t_segmento* segmento_pcb = crear_segmento(mapa_segmentos, tamanio_pcb, algoritmo);
	if(segmento_pcb == NULL) {
		log_info(logger, "Entro a realizar compactacion");
		// uint32_t final_memoria = realizar_compactacion();
		segmento_pcb = crear_segmento(mapa_segmentos, tamanio_pcb, algoritmo);
	}
	
	log_info(logger, "Segmento tareas");
	t_segmento* segmento_tareas = crear_segmento(mapa_segmentos, tamanio_tareas, algoritmo);
	if(segmento_tareas == NULL) {	
		log_info(logger, "Entro a realizar compactacion");
		// uint32_t final_memoria = realizar_compactacion();
		segmento_tareas = crear_segmento(mapa_segmentos, tamanio_tareas, algoritmo);
	}
	log_info(logger, "Segmentar pcb");

	segmentar_pcb(segmento_pcb, patota, segmento_tareas);
	log_info(logger, "Segmentar tareas");
	segmentar_tareas(segmento_tareas, patota, vector_tareas);
	log_info(logger, "Agrego nueva_patota");
	patota_data* nueva_patota = malloc(sizeof(patota_data));
	nueva_patota->PID = patota;
	nueva_patota->tabla_segmentos = malloc(2 * sizeof(uint32_t));
	nueva_patota->tabla_segmentos[0] = segmento_pcb->inicio;
	nueva_patota->tabla_segmentos[1] = segmento_tareas->inicio;
	list_add(lista_patotas, nueva_patota);
	return true;
}

bool iniciar_tripulante(t_log* logger, t_list* parametros, t_list* mapa_segmentos, t_list* lista_tripulantes) {
	int tamanio_tcb = sizeof(t_tripulante);
	if (tamanio_tcb > memoria_libre) {
		return false;
	}
	t_segmento* segmento_tcb = crear_segmento(mapa_segmentos, sizeof(t_tripulante), algoritmo);
	if(segmento_tcb == NULL) {
		// uint32_t final_memoria = realizar_compactacion();
		segmento_tcb = crear_segmento(mapa_segmentos, sizeof(t_tripulante), algoritmo);
	}
	
	t_tripulante* nuevo_tripulante = malloc(sizeof(t_tripulante));
	/*
	t_patota* patota = (t_patota *)list_get(patotas);*/
	nuevo_tripulante->estado = 'N';
	nuevo_tripulante->TID = (int)list_get(parametros, 2);
	nuevo_tripulante->posicion_x = (int)list_get(parametros, 3);
	nuevo_tripulante->posicion_y = (int)list_get(parametros, 4);
	nuevo_tripulante->proxima_tarea = 0;
	// INIT_T | id_patota [int] | id_trip [int] | posicion_x [int] | posicion_y [int]
	// segmentar_tcb(segmento_tcb, TID, estado, posicion_x, posicion_y, proxima_tarea, pcb);
	segmentar_tcb(segmento_tcb, (int)list_get(parametros, 1), nuevo_tripulante);

	pthread_t* hilo_nuevo = malloc(sizeof(pthread_t));
	trip_data* nuevo_trip = malloc(sizeof(trip_data));
	nuevo_trip->PID = (uint32_t)list_get(parametros, 1);
	nuevo_trip->hilo = hilo_nuevo;
	nuevo_trip->TID = (uint32_t)list_get(parametros, 2);
	// TODO realloc() nueva_patota->PID = patota_actual;
	// nueva_patota->tabla_segmentos[TID + 1] = segmento_tcb->inicio;
	list_add(lista_tripulantes, nuevo_trip);

	return true;
}