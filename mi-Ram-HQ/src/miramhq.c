#include "miramhq.h"

int variable = 1;

int main(void) {
	t_log* logger = log_create("miramhq.log", "Mi-RAM-HQ", 1, LOG_LEVEL_INFO);
	t_config* config = config_create("miramhq.config");
	
	int tamanio_memoria = config_get_int_value(config, "TAMANIO_MEMORIA");
	log_info(logger, "Iniciando memoria RAM de %d bytes", tamanio_memoria);
	
	algoritmo_segmento algoritmo;
	if(!strcmp(config_get_string_value(config, "CRITERIO_SELECCION"), "FF"))
		algoritmo = FF;
	if(!strcmp(config_get_string_value(config, "CRITERIO_SELECCION"), "BF"))
		algoritmo = BF;

	memoria_ram = malloc(tamanio_memoria);
	memoria_libre = tamanio_memoria;

	mapa_segmentos = list_create();
	t_segmento segmento_memoria;
	segmento_memoria.n_segmento = 0;
	segmento_memoria.duenio = 0;
	segmento_memoria.inicio = 0;
	segmento_memoria.tamanio = tamanio_memoria;
	list_add(mapa_segmentos, &segmento_memoria);

	// dibujar_mapa(); VACÍO

	int server_fd = crear_conexion_servidor(
		IP_RAM,	config_get_int_value(config, "PUERTO"), 1);
	
	if(!validar_socket(server_fd, logger)) {
		close(server_fd);
		log_destroy(logger);
		return ERROR_CONEXION;
	}
	log_info(logger, "Servidor listo");
	int socket_discord = esperar_cliente(server_fd);
	log_info(logger, "Conexión establecida con el discordiador");
	
	lista_patotas = list_create();
	lista_tareas = list_create();
	lista_tripulantes = list_create();
	
	t_list* mensaje_discor;
	t_mensaje* respuesta;
	
	bool inicio_correcto;
	bool conexion_activa_discord = true;

	uint32_t patota_actual = 0;
	uint32_t nro_tripulante;

	while(conexion_activa_discord == true) {
		log_info(logger, "Esperando información del discordiador");
		mensaje_discor = recibir_mensaje(socket_discord);
		if (!validar_mensaje(mensaje_discor, logger)) {
			log_info(logger, "Cliente desconectado dentro del while");
			close(server_fd);
			log_destroy(logger);
			return ERROR_CONEXION;
		}
		
		switch((int)list_get(mensaje_discor, 0)) { // protocolo del mensaje
		case INIT_P:
			log_info(logger, "Discordiador solicitó iniciar_patota");
			patota_actual++;
			inicio_correcto = iniciar_patota(patota_actual, mensaje_discor, algoritmo);
			
			if(!inicio_correcto)
				respuesta = crear_mensaje(NO_SPC);
			else
				respuesta = crear_mensaje(TODOOK);

			enviar_mensaje(socket_discord, respuesta);
			liberar_mensaje(respuesta);		// debe estar fuera del switch
			list_destroy(mensaje_discor);	// debe estar fuera del switch
			nro_tripulante = 1;
			break;
		case INIT_T:
			log_info(logger, "Discordiador solicitó iniciar_tripulante");
			uint32_t posicion_x = (uint32_t)list_get(mensaje_discor, 1);
			uint32_t posicion_y = (uint32_t)list_get(mensaje_discor, 2);
			inicio_correcto = iniciar_tripulante(nro_tripulante, patota_actual, posicion_x, posicion_y, algoritmo);
			
			if(inicio_correcto == false) {
				respuesta = crear_mensaje(NO_SPC);
			}
			else {
				int socket_nuevo = crear_conexion_servidor(IP_RAM, 0, 1);
				// nuevo_trip->hilo = hilo_nuevo;
				pthread_t* hilo_nuevo = malloc(sizeof(pthread_t));
				
				uint32_t patota = patota_actual;
				t_list* parametros_hilo = list_create();
				list_add(parametros_hilo, (void *)socket_nuevo);
				list_add(parametros_hilo, (void *)nro_tripulante);
				list_add(parametros_hilo, (void *)patota);
				pthread_create(hilo_nuevo, NULL, rutina_hilos, (void *)parametros_hilo);
				// close(socket_nuevo);
				respuesta = crear_mensaje(SND_PO);
				agregar_parametro_a_mensaje(respuesta, (void *)puerto_desde_socket(socket_nuevo), ENTERO);
			}
			
			enviar_mensaje(socket_discord, respuesta);
			liberar_mensaje(respuesta);
			list_destroy(mensaje_discor);
			nro_tripulante++;
			break;
		case ELIM_T:
			log_info(logger, "Discordiador solicitó eliminar_tripulante");
			uint32_t id_trip = (uint32_t)list_get(mensaje_discor, 1);
			uint32_t id_patota = (uint32_t)list_get(mensaje_discor, 2);
			
			log_info(logger, "Voy a eliminar_tripulante");
			eliminar_tripulante(id_patota, id_trip);
			respuesta = crear_mensaje(TODOOK);
			enviar_mensaje(socket_discord, respuesta);
			liberar_mensaje(respuesta);
			list_destroy(mensaje_discor);
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
		
		log_info(logger, "NUEVOS RESULTADOS");
		log_info(logger, "Cantidad de segmentos: %d", mapa_segmentos->elements_count);
		// log_info(logger, "Memoria libre: %d", memoria_libre);
		for (int i = 0; i < mapa_segmentos->elements_count; i++) {
			/*log_info(logger, "SEGMENTO %d", ((t_segmento *)list_get(mapa_segmentos, i))->n_segmento + 1);
			log_info(logger, "Duenio: %d", ((t_segmento *)list_get(mapa_segmentos, i))->duenio);
			log_info(logger, "Indice: %d", ((t_segmento *)list_get(mapa_segmentos, i))->indice);
			log_info(logger, "Inicio: %d", ((t_segmento *)list_get(mapa_segmentos, i))->inicio);
			log_info(logger, "Tamanio: %d", ((t_segmento *)list_get(mapa_segmentos, i))->tamanio);*/

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
			log_info(logger, "Info desde lista");
			log_info(logger, "Patota %d:\nPID: %d; Puntero a PCB: %d; Puntero a tareas: %d", i + 1,
				((patota_data *)(uint32_t)list_get(lista_patotas, i))->PID,
				((patota_data *)(uint32_t)list_get(lista_patotas, i))->tabla_segmentos[0],
				((patota_data *)(uint32_t)list_get(lista_patotas, i))->tabla_segmentos[1]);

			log_info(logger, "Info desde memoria");
			inicio = ((patota_data *)(uint32_t)list_get(lista_patotas, i))->tabla_segmentos[0];
			// obtener_valor(memoria_ram + inicio, )
			memcpy(&pid, memoria_ram + inicio, sizeof(uint32_t));
			memcpy(&pnt_tareas, memoria_ram + inicio + sizeof(uint32_t), sizeof(uint32_t));
			log_info(logger, "PID: %d; Puntero a tareas: %d", pid, pnt_tareas);
		}
		
		log_info(logger, "Lista de tripulantes: %d", lista_tripulantes->elements_count);
		for(int i = 0; i < lista_tripulantes->elements_count; i++) {
			inicio = ((trip_data *)(uint32_t)list_get(lista_tripulantes, i))->inicio;
			if(((trip_data *)(uint32_t)list_get(lista_tripulantes, i))->seguir == true)
			log_info(logger, "TID: %d; estado: %c; pos_x: %d; pos_y: %d; IP: %d; Punt PCB: %d",
				obtener_valor_tripulante(memoria_ram + inicio, TRIP_IP),
				obtener_estado(memoria_ram + inicio),
				obtener_valor_tripulante(memoria_ram + inicio, POS_X),
				obtener_valor_tripulante(memoria_ram + inicio, POS_Y),
				obtener_valor_tripulante(memoria_ram + inicio, INS_POINTER),
				obtener_valor_tripulante(memoria_ram + inicio, PCB_POINTER)
				);
		}
	}
	return EXIT_SUCCESS;
}

void* rutina_hilos(void* parametros) {
	// t_log* logger = log_create("miramhq.log", "HILOX", 1, LOG_LEVEL_INFO);
	// log_info(logger, "HOLA MUNDO, SOY UN HILO %d", variable);

	int socket = (uint32_t)list_get((t_list *)parametros, 0);
	uint32_t id_trip = (uint32_t)list_get((t_list *)parametros, 1);
	uint32_t id_patota = (uint32_t)list_get((t_list *)parametros, 2);
	list_destroy((t_list *)parametros);
	
	// log_info(logger, "Recibií parametros: %d, %d, %d", socket, id_trip, id_patota);
	patota_data* segmento_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	// log_info(logger, "0, %d/ %d", lista_tareas->elements_count, lista_patotas->elements_count);
	tareas_data* segmento_tareas = (tareas_data *)list_get(lista_tareas, id_patota - 1);
	// uint32_t inicio_segmento = segmento_patota->tabla_segmentos[id_trip + 1];
	// log_info(logger, "1");
	variable++;
	uint32_t ip;
	char* tarea_nueva;
	// log_info(logger, "2");
	t_mensaje* mensaje_out;
	t_list* mensaje_in;
	// log_info(logger, "Espero cliente");
	int socket_cliente = esperar_cliente((int)socket);

	int posicion_x;
	int posicion_y;
	while(1) {
		mensaje_in = recibir_mensaje((int)socket_cliente);
		// if(!validar_mensaje(mensaje_in, logger)) {
			// printf("FALLO EN MENSAJE CON HILO RAM\n");
		// }
		// else
			// printf("EL HILO DISCORD ME DIJO: %d\n", (int)list_get(mensaje_in, 0));
		switch ((int)list_get(mensaje_in, 0)) {
		case NEXT_T:
			// log_info(logger, "El hilo me pidio la proxima tarea");
			ip = obtener_valor_tripulante(memoria_ram + segmento_patota->tabla_segmentos[id_trip + 1], INS_POINTER);
			// log_info(logger, "Obtuve ip %d", ip);
			if(ip + 1 > segmento_tareas->cant_tareas) {
				// log_info(logger, "IP fuera de rango");
				mensaje_out = crear_mensaje(ER_MSJ);
			}
			else {
				// log_info(logger, "IP valido");
				tarea_nueva = obtener_tarea(memoria_ram + segmento_patota->tabla_segmentos[1], segmento_tareas, ip);
				// log_info(logger, "Obtuve tarea");
				actualizar_valor_tripulante(memoria_ram + segmento_patota->tabla_segmentos[id_trip + 1], INS_POINTER, ip + 1);
				// log_info(logger, "Actualice ip %d");
				mensaje_out = crear_mensaje(TASK_T);
				agregar_parametro_a_mensaje(mensaje_out, tarea_nueva, BUFFER);
			}
			break;
		case ACTU_T:
			// log_info(logger, "El hilo me pidio actualizar posicion");
			// int posicion_x;
			// int posicion_y;

			posicion_x = (int)list_get(mensaje_in, 1);
			posicion_y = (int)list_get(mensaje_in, 2);

			actualizar_valor_tripulante(memoria_ram + segmento_patota->tabla_segmentos[id_trip + 1], POS_X, posicion_x);
			actualizar_valor_tripulante(memoria_ram + segmento_patota->tabla_segmentos[id_trip + 1], POS_Y, posicion_y);

			// log_info(logger, "Tripulante %d posicion actualizada %d|%d", id_trip, posicion_x, posicion_y);

			mensaje_out = crear_mensaje(TODOOK);
			break;
		case ELIM_T:
			break;
		case SHOW_T:
			break;
		}
		// log_info(logger, "Envío tarea");
		enviar_mensaje((int)socket_cliente, mensaje_out);
		liberar_mensaje(mensaje_out);
	}
	return 0;
}