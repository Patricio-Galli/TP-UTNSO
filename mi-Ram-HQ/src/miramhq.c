#include "miramhq.h"

int memoria_libre;
void* memoria_ram;
algoritmo_segmento algoritmo;
int variable = 1;

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
		if (!validar_mensaje(mensaje_discor, logger)) {
			close(server_fd);
			log_destroy(logger);
			return ERROR_CONEXION;
		}
		
		switch((int)list_get(mensaje_discor, 0)) { // protocolo del mensaje
		case INIT_P:
			log_info(logger, "Discordiador solicitó iniciar_patota");
			
			inicio_correcto = iniciar_patota(logger, mensaje_discor, mapa_segmentos, lista_patotas, patota_actual);
			if(!inicio_correcto) {
				log_error(logger, "No hay tamanio suficiente");
				respuesta = crear_mensaje(NO_SPC);
			}
			else {
				respuesta = crear_mensaje(TODOOK);
			}

			enviar_mensaje(socket_discord, respuesta);
			patota_actual++;
			liberar_mensaje(respuesta);				// debe estar fuera del switch
			list_destroy(mensaje_discor);	// debe estar fuera del switch
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

				pthread_create(hilo_nuevo, NULL, rutina_hilos, (void *)socket_nuevo);
				// close(socket_nuevo);
				respuesta = crear_mensaje(SND_PO);
				agregar_parametro_a_mensaje(respuesta, (void *)puerto_desde_socket(socket_nuevo), ENTERO);
			}
			
			log_info(logger, "Envío respuesta al discordiador");
			enviar_mensaje(socket_discord, respuesta);
			liberar_mensaje(respuesta);
			list_destroy(mensaje_discor);
			
			break;
		case 64:
			log_info(logger, "Cliente desconectado");
			conexion_activa_discord = false;
			break;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			conexion_activa_discord = false;
			break;
		}
		
	}
	log_info(logger, "Resultados");
	
	log_info(logger, "Lista segmentos: %d", mapa_segmentos->elements_count);
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
	for (int i = 0; i < lista_patotas->elements_count; i++) {
		log_info(logger, "Patota %d, %d", i + 1, ((patota_data *)list_get(lista_patotas, i))->PID);
		log_info(logger, "Tamanio tabla: %d", ((patota_data *)list_get(lista_patotas, i))->tamanio_tabla);
		for(int b = 0; b < ((patota_data *)list_get(lista_patotas, i))->tamanio_tabla; b++) {
			log_info(logger, "Inicio elemento %d: %d", b + 1, ((patota_data *)list_get(lista_patotas, i))->tabla_segmentos[b]);	
		}
	}
	log_info(logger, "Lista de tripulantes: %d", lista_tripulantes->elements_count);
	for (int i = 0; i < lista_tripulantes->elements_count; i++) {
		log_info(logger, "TID %d", ((trip_data *)list_get(lista_tripulantes, i))->TID);
		log_info(logger, "PID: %d", ((trip_data *)list_get(lista_tripulantes, i))->PID);
	}
	return EXIT_SUCCESS;
}

void* rutina_hilos(void* socket, t_tripulante* mi_tripulante) {
	t_log* logger = log_create("miramhq.log", "HILOX", 1, LOG_LEVEL_INFO);
	log_info(logger, "HOLA MUNDO, SOY UN HILO %d", variable);
	variable++;
	
	int socket_cliente = esperar_cliente((int)socket);
	// data_socket((int)socket, logger);
	// data_socket((int)socket_cliente, logger);

	while(1) {
		t_list* mensaje_in = recibir_mensaje((int)socket_cliente);
		if(!validar_mensaje(mensaje_in, logger)) {
			printf("FALLO EN MENSAJE CON HILO RAM\n");
		}
		else
			printf("EL HILO DISCORD ME DIJO: %d\n", (int)list_get(mensaje_in, 0));
		
		t_mensaje* mensaje_out = crear_mensaje(TODOOK);
		enviar_mensaje((int)socket_cliente, mensaje_out);
		liberar_mensaje(mensaje_out);
	}
	return 0;
}



bool iniciar_patota(t_log* logger, t_list* parametros, t_list* mapa_segmentos, t_list* lista_patotas, int patota) {
	int tamanio_pcb = sizeof(t_patota);
	int tamanio_tarea = 0;
	int tamanio_bloque_tareas = 0;
	int cantidad_tareas = (int)list_get(parametros, 3);
	char** vtareas = malloc(sizeof(char *) * cantidad_tareas);
	uint32_t* vtareas_inicio = malloc(sizeof(uint32_t) * cantidad_tareas);
	uint32_t* vtareas_tamanio = malloc(sizeof(uint32_t) * cantidad_tareas);

	for(int i = 0; i < cantidad_tareas; i++) {
		char* tarea_i = (char *)list_get(parametros, 4 + i);
		vtareas_inicio[i] = tamanio_bloque_tareas;
		tamanio_tarea = strlen(tarea_i) + 1;
		tamanio_bloque_tareas += tamanio_tarea;
		vtareas[i] = tarea_i;
		vtareas_tamanio[i] = tamanio_tarea;
	}

	if (tamanio_pcb + tamanio_bloque_tareas > memoria_libre) {
		return false;
	}
	// CREO SEGMENTOS
	log_info(logger, "CREO SEGMENTO PCB");
	t_segmento* segmento_pcb = crear_segmento(mapa_segmentos, tamanio_pcb, algoritmo);
	if(segmento_pcb == NULL) {
		log_info(logger, "Entro a realizar compactacion");
		// uint32_t final_memoria = realizar_compactacion();
		segmento_pcb = crear_segmento(mapa_segmentos, tamanio_pcb, algoritmo);
	}
	segmento_pcb->duenio = patota;
	log_info(logger, "CREO SEGMENTO TAREAS");
	t_segmento* segmento_tareas = crear_segmento(mapa_segmentos, tamanio_bloque_tareas, algoritmo);
	printf("Cree segmento tareas\n");
	if(segmento_tareas == NULL) {	
		log_info(logger, "Entro a realizar compactacion");
		// uint32_t final_memoria = realizar_compactacion();
		segmento_tareas = crear_segmento(mapa_segmentos, tamanio_bloque_tareas, algoritmo);
	}
	segmento_tareas->duenio = patota;
	// CREO ESTRUCTURA PATOTA PARA GUARDAR EN TABLA
	patota_data* nueva_patota = malloc(sizeof(patota_data));
	nueva_patota->PID = patota;
	nueva_patota->tabla_segmentos = malloc(2 * sizeof(uint32_t));
	nueva_patota->tabla_segmentos[0] = segmento_pcb->inicio;
	nueva_patota->tabla_segmentos[1] = segmento_tareas->inicio;
	nueva_patota->tamanio_tabla = 2;
	list_add(lista_patotas, nueva_patota);
	printf("Cree estructrua patota\n");
	// CREO ESTRUCTURA TAREAS PARA GUARDAR EN TABLA
	tareas_data* nuevo_bloque_tareas = malloc(sizeof(tareas_data));
	nuevo_bloque_tareas->cant_tareas = cantidad_tareas;
	nuevo_bloque_tareas->inicio_tareas = vtareas_inicio;
	nuevo_bloque_tareas->tamanio_tareas = vtareas_tamanio;
	
	// SEGMENTO PCB
	/*t_list* parametros_pcb = list_create();
	log_info(logger, "AGREGO PARAM");
	agregar_parametro_a_segmento(parametros_pcb, (void *)patota, T_ENTERO);
	log_info(logger, "AGREGO PARAM");
	agregar_parametro_a_segmento(parametros_pcb, (void *)patota, T_ENTERO);
	log_info(logger, "AGREGO PARAM");
	agregar_parametro_a_segmento(parametros_pcb, (void *)segmento_tareas->inicio, T_ENTERO);
	log_info(logger, "SEGMENTO PCB");
	segmentar(memoria_ram, segmento_pcb, parametros_pcb);
	list_destroy(parametros_pcb);*/
	printf("Voy a nuevo segmentar pcb\n");
	/*nuevo_segmentar(memoria_ram, segmento_pcb->inicio, (void *)patota, SEG_INT, sizeof(uint32_t));
	nuevo_segmentar(memoria_ram, segmento_pcb->inicio + sizeof(uint32_t), (void *)segmento_tareas->inicio, SEG_INT, sizeof(uint32_t));
	*/
	segmentar_entero(memoria_ram, segmento_pcb->inicio, patota);
	segmentar_entero(memoria_ram, segmento_pcb->inicio + sizeof(uint32_t), segmento_tareas->inicio);
	// SEGMENTO TAREAS
	
	/*t_list* parametros_tareas = list_create();
	log_info(logger, "AGREGO PARAM");
	agregar_parametro_a_segmento(parametros_pcb, (void *)patota, T_ENTERO);
	for(int i = 0; i < cantidad_tareas; i++) {
		log_info(logger, "AGREGO PARAM");
		agregar_parametro_a_segmento(parametros_tareas, vector_tareas[i], T_CADENA);		
	}
	log_info(logger, "SEGMENTO TAREAS");
	segmentar(memoria_ram, segmento_tareas, parametros_tareas);
	list_destroy(parametros_tareas);*/

	printf("Voy a nuevo segmentar tareas\n");
	for(int i = 0; i < cantidad_tareas; i++) {
		log_info(logger, "nuevo segmentar");
		segmentar_string(memoria_ram, segmento_tareas->inicio + vtareas_inicio[i], vtareas[i]);
		free(vtareas[i]);
	}
	
	/*for(int i = 0; i < cantidad_tareas; i++) {
		log_info(logger, "nuevo segmentar");
		nuevo_segmentar(memoria_ram, segmento_tareas->inicio + nuevo_bloque_tareas->posiciones_tareas[i], vector_tareas[i], SEG_STRING, vector_tareas_tam[i]);
		free(vector_tareas[i]);
	}
	free(vector_tareas);
	free(vector_tareas_tam);*/
	return true;
}

bool iniciar_tripulante(t_log* logger, t_list* parametros, t_list* mapa_segmentos, t_list* lista_tripulantes, t_list* lista_patotas) {
	int tamanio_tcb = sizeof(t_tripulante);
	if (tamanio_tcb > memoria_libre) {
		return false;
	}
	t_segmento* segmento_tcb = crear_segmento(mapa_segmentos, sizeof(t_tripulante), algoritmo);
	
	if(segmento_tcb == NULL) {
		// uint32_t final_memoria = realizar_compactacion();
		segmento_tcb = crear_segmento(mapa_segmentos, sizeof(t_tripulante), algoritmo);
	}
	
	int id_patota = (int)list_get(parametros, 1);
	patota_data* patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	// nuevo_tripulante->pcb = patota->tabla_segmentos[0];
	/*
	t_list* parametros_tcb = list_create();
	list_add(parametros_tcb, list_get(parametros, 2));
	list_add(parametros_tcb, 'N');
	list_add(parametros_tcb, list_get(parametros, 3));
	list_add(parametros_tcb, list_get(parametros, 4));
	list_add(parametros_tcb, 0);
	list_add(parametros_tcb, patota->tabla_segmentos[0]);*/
	
	
	/*log_info(logger, "AGREGO PARAM");
	agregar_parametro_a_segmento(parametros_tcb, (void *)id_patota, T_ENTERO);
	log_info(logger, "AGREGO PARAM");
	agregar_parametro_a_segmento(parametros_tcb, (void *)list_get(parametros, 2), T_ENTERO);
	log_info(logger, "AGREGO PARAM");
	agregar_parametro_a_segmento(parametros_tcb, (void *)'N', T_CARACTER);
	log_info(logger, "AGREGO PARAM");
	agregar_parametro_a_segmento(parametros_tcb, (void *)list_get(parametros, 3), T_ENTERO);
	log_info(logger, "AGREGO PARAM");
	agregar_parametro_a_segmento(parametros_tcb, (void *)list_get(parametros, 4), T_ENTERO);
	log_info(logger, "AGREGO PARAM");
	agregar_parametro_a_segmento(parametros_tcb, (void *)0, T_ENTERO);
	log_info(logger, "AGREGO PARAM");
	agregar_parametro_a_segmento(parametros_tcb, (void *)patota->tabla_segmentos, T_ENTERO);
	log_info(logger, "SEGMENTO TCB");
	segmentar(memoria_ram, segmento_tcb, parametros_tcb);
	 uint32_t TID;
    char estado;
    uint32_t posicion_x;
    uint32_t posicion_y;
    uint32_t proxima_tarea;
    uint32_t pcb;*/
	/*segmentar_tcb(memoria_ram, segmento_tcb, parametros_tcb);
	uint32_t desplazamiento = 0;
	nuevo_segmentar(memoria_ram, segmento_tcb->inicio + desplazamiento, list_get(parametros, 2), SEG_INT, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	nuevo_segmentar(memoria_ram, segmento_tcb->inicio + desplazamiento, (void *)'N', SEG_CHAR, sizeof(char));
	desplazamiento += sizeof(char);
	nuevo_segmentar(memoria_ram, segmento_tcb->inicio + desplazamiento, list_get(parametros, 3), SEG_INT, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	nuevo_segmentar(memoria_ram, segmento_tcb->inicio + desplazamiento, list_get(parametros, 4), SEG_INT, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	nuevo_segmentar(memoria_ram, segmento_tcb->inicio + desplazamiento, 0, SEG_INT, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	nuevo_segmentar(memoria_ram, segmento_tcb->inicio + desplazamiento, (void *)patota->tabla_segmentos[0], SEG_INT, sizeof(uint32_t));
	*/
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
	// free(nuevo_tripulante);
	// CREO ESTRUCTURA TRIPULANTE PARA GUARDAR EN TABLA
	uint32_t TID = (int)list_get(parametros, 2);
	trip_data* nuevo_trip = malloc(sizeof(trip_data));
	nuevo_trip->PID = (uint32_t)list_get(parametros, 1);
	nuevo_trip->TID = (uint32_t)list_get(parametros, 2);
	if(patota->tamanio_tabla - 2 < TID) {
		patota->tabla_segmentos = realloc(patota->tabla_segmentos, sizeof(uint32_t *) * TID + 2);
		patota->tamanio_tabla = TID;
	}
	patota->tabla_segmentos[TID + 1] = segmento_tcb->inicio;
	patota->tamanio_tabla++;
	list_add(lista_tripulantes, nuevo_trip);
	return true;
}