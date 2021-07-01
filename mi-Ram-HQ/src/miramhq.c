#include "miramhq.h"

int memoria_libre;
void* memoria_ram;
algoritmo_segmento algoritmo;
int variable = 1;
uint32_t patota_actual = 0;
t_list* lista_patotas;
t_list* lista_tareas;
t_list* lista_tripulantes;

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
			inicio_correcto = iniciar_patota(logger, mensaje_discor, mapa_segmentos, lista_patotas);
			if(!inicio_correcto) {
				log_error(logger, "No hay tamanio suficiente");
				respuesta = crear_mensaje(NO_SPC);
			}
			else {
				respuesta = crear_mensaje(TODOOK);
			}

			log_info(logger, "Envío respuesta al discordiador");
			enviar_mensaje(socket_discord, respuesta);
			liberar_mensaje(respuesta);		// debe estar fuera del switch
			list_destroy(mensaje_discor);	// debe estar fuera del switch
			nro_tripulante = 1;
			break;
		case INIT_T:
			log_info(logger, "Discordiador solicitó iniciar_tripulante");
			
			inicio_correcto = iniciar_tripulante(logger, mensaje_discor, mapa_segmentos, lista_tripulantes, lista_patotas);
			
			if(inicio_correcto == false) {
				log_error(logger, "No hay tamanio suficiente");
				respuesta = crear_mensaje(NO_SPC);
			}
			else {
				int socket_nuevo = crear_conexion_servidor(IP_RAM, 0, 1);
				// nuevo_trip->hilo = hilo_nuevo;
				pthread_t* hilo_nuevo = malloc(sizeof(pthread_t));
				
				uint32_t patota = patota_actual;
				t_list* parametros_hilo = list_create();
				list_add(parametros_hilo, socket_nuevo);
				list_add(parametros_hilo, nro_tripulante);
				list_add(parametros_hilo, patota);
				pthread_create(hilo_nuevo, NULL, rutina_hilos, (void *)parametros_hilo);
				// close(socket_nuevo);
				respuesta = crear_mensaje(SND_PO);
				agregar_parametro_a_mensaje(respuesta, (void *)puerto_desde_socket(socket_nuevo), ENTERO);
			}
			
			log_info(logger, "Envío respuesta al discordiador");
			enviar_mensaje(socket_discord, respuesta);
			liberar_mensaje(respuesta);
			list_destroy(mensaje_discor);
			nro_tripulante++;
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
	}
	log_info(logger, "Resultados");
	
	log_info(logger, "Lista segmentos: %d", mapa_segmentos->elements_count);
	log_info(logger, "Memoria libre: %d", memoria_libre);
	for (int i = 0; i < mapa_segmentos->elements_count; i++) {
		log_info(logger, "Segmento %d %d", i + 1, ((t_segmento *)list_get(mapa_segmentos, i))->n_segmento);
		log_info(logger, "Duenio: %d", ((t_segmento *)list_get(mapa_segmentos, i))->duenio);
		log_info(logger, "Inicio: %d", ((t_segmento *)list_get(mapa_segmentos, i))->inicio);
		log_info(logger, "Tamanio: %d", ((t_segmento *)list_get(mapa_segmentos, i))->tamanio);
		uint32_t primer_elemento;
		memcpy(&primer_elemento, memoria_ram + ((t_segmento *)list_get(mapa_segmentos, i))->inicio, sizeof(uint32_t));
		log_info(logger, "Primer elemento: %d", primer_elemento);
	}
	log_info(logger, "Lista de patotas: %d", lista_patotas->elements_count);
	
	uint32_t inicio;
	uint32_t pid;
	uint32_t pnt_tareas;
	
	for(int i = 0; i < lista_patotas->elements_count; i++) {
		log_info(logger, "Info desde lista");
		log_info(logger, "Patota i:\nPID: %d; Puntero a PCB: %d; Puntero a tareas: %d",
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
	uint32_t valor_trip;
	char estado;
	for(int i = 0; i < lista_tripulantes->elements_count; i++) {
		inicio = ((trip_data *)(uint32_t)list_get(lista_tripulantes, i))->inicio;
		memcpy(&valor_trip, memoria_ram + inicio, sizeof(uint32_t));
		memcpy(&estado, memoria_ram + inicio + sizeof(uint32_t), sizeof(char));
		log_info(logger, "TID: %d; estado: %c", valor_trip, estado);
		log_info(logger, "TID: %d; estado: %c; pos_x: %d; pos_y: %d; IP: %d; Punt PCB: %d",
			obtener_valor_tripulante(memoria_ram + inicio, 0),
			obtener_valor_tripulante(memoria_ram + inicio, 1),
			obtener_valor_tripulante(memoria_ram + inicio, 2),
			obtener_valor_tripulante(memoria_ram + inicio, 3),
			obtener_valor_tripulante(memoria_ram + inicio, 4),
			obtener_valor_tripulante(memoria_ram + inicio, 5)
			);
			actualizar_valor(memoria_ram + inicio, 4, i);
			actualizar_valor(memoria_ram + inicio, 1, 'R');
			log_info(logger, "Estado: %c; IP: %d",
				obtener_valor_tripulante(memoria_ram + inicio, 1),
				obtener_valor_tripulante(memoria_ram + inicio, 4));
	}
	return EXIT_SUCCESS;
}

bool iniciar_patota(t_log* logger, t_list* parametros, t_list* mapa_segmentos, t_list* lista_patotas) {
	int tamanio_pcb = sizeof(t_patota);
	int tamanio_tarea = 0;
	int tamanio_bloque_tareas = 0;
	int cantidad_tareas = (int)list_get(parametros, 3);

	for(int i = 0; i < cantidad_tareas; i++) {
		tamanio_bloque_tareas += strlen((char *)list_get(parametros, 4 + i)) + 1;
	}

	if (tamanio_pcb + tamanio_bloque_tareas > memoria_libre) {
		return false;
	}

	char** vtareas = malloc(sizeof(char *) * cantidad_tareas);
	uint32_t* vtareas_inicio = malloc(sizeof(uint32_t) * cantidad_tareas);
	uint32_t* vtareas_tamanio = malloc(sizeof(uint32_t) * cantidad_tareas);

	tamanio_bloque_tareas = 0;
	for(int i = 0; i < cantidad_tareas; i++) {
		char* tarea_i = (char *)list_get(parametros, 4 + i);
		tamanio_tarea = strlen(tarea_i) + 1;
		vtareas_inicio[i] = tamanio_bloque_tareas;
		tamanio_bloque_tareas += tamanio_tarea;
		vtareas[i] = tarea_i;
		vtareas_tamanio[i] = tamanio_tarea;
	}

	// CREO SEGMENTO PCB
	t_segmento* segmento_pcb = crear_segmento(mapa_segmentos, tamanio_pcb, algoritmo);
	if(segmento_pcb == NULL) {
		// uint32_t final_memoria = realizar_compactacion();
		segmento_pcb = crear_segmento(mapa_segmentos, tamanio_pcb, algoritmo);
	}
	segmento_pcb->duenio = patota_actual;

	// CREO SEGMENTO TAREAS
	t_segmento* segmento_tareas = crear_segmento(mapa_segmentos, tamanio_bloque_tareas, algoritmo);
	if(segmento_tareas == NULL) {
		// uint32_t final_memoria = realizar_compactacion();
		segmento_tareas = crear_segmento(mapa_segmentos, tamanio_bloque_tareas, algoritmo);
	}
	segmento_tareas->duenio = patota_actual;
	
	// SEGMENTO PCB
	segmentar_entero(memoria_ram, segmento_pcb->inicio, patota_actual);
	segmentar_entero(memoria_ram, segmento_pcb->inicio + sizeof(uint32_t), segmento_tareas->inicio);
	memoria_libre -= 2 * sizeof(uint32_t);

	// SEGMENTO TAREAS
	for(int i = 0; i < cantidad_tareas; i++) {
		segmentar_string(memoria_ram, segmento_tareas->inicio + vtareas_inicio[i], vtareas[i]);
		free(vtareas[i]);
	}
	memoria_libre -= tamanio_bloque_tareas;

	// CREO ESTRUCTURA PATOTA PARA GUARDAR EN TABLA
	patota_data* nueva_patota = malloc(sizeof(patota_data));
	nueva_patota->PID = patota_actual;
	nueva_patota->tabla_segmentos = malloc(2 * sizeof(uint32_t));
	nueva_patota->tabla_segmentos[0] = segmento_pcb->inicio;
	nueva_patota->tabla_segmentos[1] = segmento_tareas->inicio;
	nueva_patota->tamanio_tabla = 2;
	list_add(lista_patotas, nueva_patota);
	
	// CREO ESTRUCTURA TAREAS PARA GUARDAR EN TABLA
	tareas_data* nuevo_bloque_tareas = malloc(sizeof(tareas_data));
	nuevo_bloque_tareas->cant_tareas = cantidad_tareas;
	nuevo_bloque_tareas->inicio_tareas = vtareas_inicio;
	nuevo_bloque_tareas->tamanio_tareas = vtareas_tamanio;
	list_add(lista_tareas, nuevo_bloque_tareas);
	return true;
}

bool iniciar_tripulante(t_log* logger, t_list* parametros, t_list* mapa_segmentos, t_list* lista_tripulantes, t_list* lista_patotas) {
	int tamanio_tcb = sizeof(t_tripulante);
	if (tamanio_tcb > memoria_libre) {
		return false;
	}
	
	// CREO SEGMENTO PCB
	t_segmento* segmento_tcb = crear_segmento(mapa_segmentos, sizeof(t_tripulante), algoritmo);
	if(segmento_tcb == NULL) {
		// uint32_t final_memoria = realizar_compactacion();
		segmento_tcb = crear_segmento(mapa_segmentos, sizeof(t_tripulante), algoritmo);
	}
	segmento_tcb->duenio = patota_actual;
	
	int id_patota = (int)list_get(parametros, 1);
	patota_data* patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	
	// SEGMENTO TCB
	uint32_t desplazamiento = 0;
	segmentar_entero(memoria_ram, segmento_tcb->inicio + desplazamiento, (int)list_get(parametros, 2));
	desplazamiento += sizeof(uint32_t);
	segmentar_caracter(memoria_ram, segmento_tcb->inicio + desplazamiento, 'N');
	desplazamiento += sizeof(char);
	segmentar_entero(memoria_ram, segmento_tcb->inicio + desplazamiento, (int)list_get(parametros, 3));
	desplazamiento += sizeof(uint32_t);
	segmentar_entero(memoria_ram, segmento_tcb->inicio + desplazamiento, (int)list_get(parametros, 4));
	desplazamiento += sizeof(uint32_t);
	segmentar_entero(memoria_ram, segmento_tcb->inicio + desplazamiento, 0);
	desplazamiento += sizeof(uint32_t);
	segmentar_entero(memoria_ram, segmento_tcb->inicio + desplazamiento, patota->tabla_segmentos[0]);
	desplazamiento += sizeof(uint32_t);
	memoria_libre -= desplazamiento;
	
	// CREO ESTRUCTURA TRIPULANTE PARA GUARDAR EN TABLA
	uint32_t TID = (int)list_get(parametros, 2);
	trip_data* nuevo_trip = malloc(sizeof(trip_data));
	nuevo_trip->PID = (uint32_t)list_get(parametros, 1);
	nuevo_trip->TID = (uint32_t)list_get(parametros, 2);
	if(patota->tamanio_tabla - 2 < TID) {
		patota->tabla_segmentos = realloc(patota->tabla_segmentos, sizeof(uint32_t *) * (TID + 2));
		patota->tamanio_tabla = TID;
	}
	patota->tabla_segmentos[TID + 1] = segmento_tcb->inicio;
	nuevo_trip->inicio = segmento_tcb->inicio;
	patota->tamanio_tabla++;
	list_add(lista_tripulantes, nuevo_trip);
	return true;
}

void* rutina_hilos(void* parametros) {
	t_log* logger = log_create("miramhq.log", "HILOX", 1, LOG_LEVEL_INFO);
	log_info(logger, "HOLA MUNDO, SOY UN HILO %d", variable);

	int socket = list_get((t_list *)parametros, 0);
	uint32_t id_trip = list_get((t_list *)parametros, 1);
	uint32_t id_patota = list_get((t_list *)parametros, 2);
	log_info(logger, "Recibií parametros: %d, %d, %d", socket, id_trip, id_patota);
	patota_data* segmento_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	tareas_data* segmento_tareas = (tareas_data *)list_get(lista_tareas, id_patota - 1);
	// uint32_t inicio_segmento = segmento_patota->tabla_segmentos[id_trip + 1];

	variable++;
	uint32_t ip;
	char* tarea_nueva;

	t_mensaje* mensaje_out;
	t_list* mensaje_in;
	log_info(logger, "Espero cliente");
	int socket_cliente = esperar_cliente((int)socket);
	while(1) {
		mensaje_in = recibir_mensaje((int)socket_cliente);
		if(!validar_mensaje(mensaje_in, logger)) {
			printf("FALLO EN MENSAJE CON HILO RAM\n");
		}
		else
			printf("EL HILO DISCORD ME DIJO: %d\n", (int)list_get(mensaje_in, 0));
		switch ((int)list_get(mensaje_in, 0)) {
		case NEXT_T:
			log_info(logger, "El hilo me pidio la proxima tarea");
			ip = obtener_valor_tripulante(memoria_ram + segmento_patota->tabla_segmentos[id_trip + 1], 4);
			log_info(logger, "Obtuve ip %d", ip);
			if(ip + 1 > segmento_tareas->cant_tareas) {
				log_info(logger, "IP fuera de rango");
				mensaje_out = crear_mensaje(ER_MSJ);
			}
			else {
				log_info(logger, "IP valido");
				tarea_nueva = obtener_tarea(memoria_ram + segmento_patota->tabla_segmentos[1], segmento_tareas, ip);
				log_info(logger, "Obtuve tarea");
				actualizar_valor(memoria_ram + segmento_patota->tabla_segmentos[id_trip + 1], 4, ip + 1);
				log_info(logger, "Actualice ip %d");
				mensaje_out = crear_mensaje(TASK_T);
				agregar_parametro_a_mensaje(mensaje_out, tarea_nueva, BUFFER);
			}
			break;
		case ACTU_T:
			break;
		case ELIM_T:
			break;
		case SHOW_T:
			break;
		}
		log_info(logger, "Envío tarea");
		enviar_mensaje((int)socket_cliente, mensaje_out);
		liberar_mensaje(mensaje_out);
	}
	return 0;
}

char* obtener_tarea(void* segmento, tareas_data* tareas, uint32_t nro_tarea) {
	char * tarea = malloc(tareas->tamanio_tareas[nro_tarea]);
	memcpy(tarea, segmento + tareas->inicio_tareas[nro_tarea], tareas->tamanio_tareas[nro_tarea]);
	return tarea;
}

void* obtener_valor_tripulante(void* segmento, uint32_t nro_parametro) {
	uint32_t valor_int;
	char valor_char;
	switch(nro_parametro) {
		case 0:
			memcpy(&valor_int, segmento, sizeof(uint32_t));
			return (void *)valor_int;
		case 1:
			memcpy(&valor_char, segmento + sizeof(uint32_t), sizeof(char));
			return (void *)valor_char;
		case 2:
			memcpy(&valor_int, segmento + sizeof(uint32_t) + sizeof(char), sizeof(uint32_t));
			return (void *)valor_int;
		case 3:
			memcpy(&valor_int, segmento + 2 * sizeof(uint32_t) + sizeof(char), sizeof(uint32_t));
			return (void *)valor_int;
		case 4:
			memcpy(&valor_int, segmento + 3 * sizeof(uint32_t) + sizeof(char), sizeof(uint32_t));
			return (void *)valor_int;
		case 5:
			memcpy(&valor_int, segmento + 4 * sizeof(uint32_t) + sizeof(char), sizeof(uint32_t));
			return (void *)valor_int;
		default:
			return NULL;
	}
}

void actualizar_valor(void* segmento, uint32_t nro_parametro, void* nuevo_valor) {
	uint32_t valor_int;
	char valor_char;
	switch(nro_parametro) {
		case 0:
			valor_int = (uint32_t)nuevo_valor;
			memcpy(segmento, &valor_int, sizeof(uint32_t));
		case 1:
			valor_char = (char)nuevo_valor;
			memcpy(segmento + sizeof(uint32_t), &valor_char, sizeof(char));
		case 2:
			valor_int = (uint32_t)nuevo_valor;
			memcpy(segmento + sizeof(uint32_t) + sizeof(char), &valor_int, sizeof(uint32_t));
		case 3:
			valor_int = (uint32_t)nuevo_valor;
			memcpy(segmento + 2 * sizeof(uint32_t) + sizeof(char), &valor_int, sizeof(uint32_t));
		case 4:
			valor_int = (uint32_t)nuevo_valor;
			memcpy(segmento + 3 * sizeof(uint32_t) + sizeof(char), &valor_int, sizeof(uint32_t));
		case 5:
			valor_int = (uint32_t)nuevo_valor;
			memcpy(segmento + 4 * sizeof(uint32_t) + sizeof(char), &valor_int, sizeof(uint32_t));
	}
}