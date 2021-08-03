/*
	============================================================================
	Name		:	discordiador.c
	Author		: 
	Version		:
	Copyright	:	Your copyright notice
	Description	:	Hello World in C, Ansi-style
	============================================================================
*/

#include "discordiador.h"

sem_t semaforo_tripulante;
t_list* lista_tripulantes;

int id_patota_actual = 0;
int id_tripulante = 1;
t_config* config;
int variable;

int main() {
	t_log* logger = log_create("discordiador.log", "DISCORDIADOR", 1, LOG_LEVEL_INFO);

    config = config_create("discordiador.config");
	int socket_ram, socket_mongo = 0;

	socket_ram = crear_conexion_cliente(
		config_get_string_value(config, "IP_MI_RAM_HQ"),
		config_get_string_value(config, "PUERTO_MI_RAM_HQ")
		);
	data_socket(socket_ram, logger);
	if(!validar_socket(socket_ram, logger) || !validar_socket(socket_mongo, logger)) {
		close(socket_ram);
		close(socket_mongo);
		log_destroy(logger);
		return ERROR_CONEXION;
	}

	bool continuar = true;
	char* buffer_consola;
	command_code funcion_consola;
	t_mensaje* mensaje_out;

	char tarea1[] = "Soy la tarea pepe argento";
	char tarea2[] = "Soy la tarea moni argento";
	char tarea3[] = "Soy la tarea maria elena fuseneco";
	lista_tripulantes = list_create();
	int variable = 0;
	while(continuar) {
		buffer_consola = leer_consola();
		variable++;
		/*
		funcion_consola = mapStringToEnum(primer_palabra(buffer_consola));*/
		switch (variable) {
		case 1:
			funcion_consola = INICIAR_PATOTA;
			break;
		case 2:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 3:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 4:
			funcion_consola = INICIAR_PATOTA;
			break;
		case 5:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 6:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 7:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 8:
			funcion_consola = INICIAR_PATOTA;
			break;
		case 9:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 10:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 11:
			funcion_consola = EXPULSAR_TRIPULANTE;
			break;
		case 12:
			funcion_consola = EXPULSAR_TRIPULANTE;
			break;
		case 13:
			funcion_consola = EXPULSAR_TRIPULANTE;
			break;
		case 14:
			funcion_consola = EXPULSAR_TRIPULANTE;
			break;	
		case 15:
			// funcion_consola = INICIAR_PATOTA;
			funcion_consola = ERROR;
			break;
		case 16:
			// funcion_consola = INICIAR_TRIPULANTE;
			funcion_consola = ERROR;
			break;
		case 17:
			// funcion_consola = INICIAR_TRIPULANTE;
			funcion_consola = ERROR;
			break;
		case 18:
			funcion_consola = INICIAR_PATOTA;
			break;
		case 19:
			funcion_consola = INICIAR_PATOTA;
			break;
		case 20:
			funcion_consola = INICIAR_PATOTA;
			break;
		case 21:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 22:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		default:
			log_info(logger, "No hay más instrucciones");
			funcion_consola = EXIT_DISCORDIADOR;
			break;
		}

		t_list *mensaje_in;
		switch(funcion_consola) {
		case INICIAR_PATOTA:
			log_info(logger, "Iniciar patota. Creando mensaje");
			mensaje_out = crear_mensaje(INIT_P);
			agregar_parametro_a_mensaje(mensaje_out, (void *)2, ENTERO);	// cant_trip
			agregar_parametro_a_mensaje(mensaje_out, (void *)3, ENTERO);	// cant_tareas
			agregar_parametro_a_mensaje(mensaje_out, &tarea1, BUFFER);		// tarea 1
			agregar_parametro_a_mensaje(mensaje_out, &tarea2, BUFFER);		// tarea 2
			agregar_parametro_a_mensaje(mensaje_out, &tarea3, BUFFER);
			
			enviar_mensaje(socket_ram, mensaje_out);
			liberar_mensaje_out(mensaje_out);

			mensaje_in = recibir_mensaje(socket_ram);
			if((int)list_get(mensaje_in, 0) == TODOOK) {
				log_info(logger, "aeeea, sabalero, sabalero");
				id_patota_actual++;
			}
			if((int)list_get(mensaje_in, 0) == NO_SPC) {
				log_info(logger, "El servidor dice que no había espacio");
			}

			liberar_mensaje_in(mensaje_in);
			
			id_tripulante = 1;
			break;
		case INICIAR_TRIPULANTE:
			log_info(logger, "Iniciar tripulante. Creando mensaje");
			mensaje_out = crear_mensaje(INIT_T);
			agregar_parametro_a_mensaje(mensaje_out, (void *)1 + variable, ENTERO);				// posicion_x
			agregar_parametro_a_mensaje(mensaje_out, (void *)1 + variable, ENTERO);				// posicion_y
			
			enviar_mensaje(socket_ram, mensaje_out);
			liberar_mensaje_out(mensaje_out);

			mensaje_in = recibir_mensaje(socket_ram);
			if(!validar_mensaje(mensaje_in, logger)) {
				log_info(logger, "El servidor ha muerto, doy por finalizada esta wea");
				close(socket_ram);
				close(socket_mongo);
				log_destroy(logger);
				return ERROR_CONEXION;
			}
			log_info(logger, "Recibi respuesta: %d", (int)list_get(mensaje_in, 0));
			// if((int)list_get(mensaje_in, 0) == SND_PO) {
			// 	log_info(logger, "Entro a recibir puerto\n");
			// 	int nuevo_puerto = (int)list_get(mensaje_in, 1);
			// 	pthread_t* nuevo_hilo = malloc(sizeof(pthread_t));
			// 	char str_puerto[7];
			// 	sprintf(str_puerto, "%d", (int)nuevo_puerto);
			// 	int socket = crear_conexion_cliente(config_get_string_value(config, "IP_MI_RAM_HQ"), str_puerto);
			// 	// sem_init(&semaforo_tripulante, 0, 0);
				
			// 	t_tripulante* nuevo_tripulante = malloc(sizeof(t_tripulante));
			// 	nuevo_tripulante->id_patota = id_patota_actual;
			// 	nuevo_tripulante->id_trip = id_tripulante;
			// 	nuevo_tripulante->hilo = nuevo_hilo;
			// 	nuevo_tripulante->vivir = true;
			// 	nuevo_tripulante->socket = socket;
			// 	nuevo_tripulante->px = 1 + variable;
			// 	nuevo_tripulante->py = 1 + variable;
			// 	log_info(logger, "Muero rapido: %d", variable % 2);
			// 	nuevo_tripulante->muero_rapido = variable % 2;

			// 	pthread_create(nuevo_hilo, NULL, rutina_hilos, (void *)nuevo_tripulante);
			// 	log_info(logger, "Agrego tripulante %d - %d", id_patota_actual, id_tripulante);
			// 	list_add(lista_tripulantes, nuevo_tripulante);
			// 	log_info(logger, "Cantidad de tripulantes actual: %d", list_size(lista_tripulantes));
			// }
			if((int)list_get(mensaje_in, 0) == NO_SPC) {
				log_info(logger, "El servidor dice que no había espacio");
			}

			liberar_mensaje_in(mensaje_in);
			id_tripulante++;
			break;
		case EXPULSAR_TRIPULANTE:
			log_info(logger,"Expulsar tripulante");
			int id_patota;
			int id_trip;
			mensaje_out = crear_mensaje(ELIM_T);
			log_info(logger, "Cree mensaje %d", funcion_consola);
			if(variable == 11) {
				log_info(logger,"Expulsar tripulante 1");
				id_patota = 1;
				id_trip = 1;
			}
			if(variable == 12) {
				log_info(logger,"Expulsar tripulante 2");
				id_patota = 3;
				id_trip = 1;
			}
			if(variable == 13) {
				log_info(logger,"Expulsar tripulante 3");
				id_patota = 2;
				id_trip = 1;
			}
			if(variable == 14) {
				log_info(logger,"Expulsar tripulante 4");
				id_patota = 2;
				id_trip = 3;
			}
			/*if(variable == 18) {
				log_info(logger,"Expulsar tripulante 5");
				agregar_parametro_a_mensaje(mensaje, (void *)2, ENTERO);
				agregar_parametro_a_mensaje(mensaje, (void *)3, ENTERO);
			}*/
			log_info(logger, "Tripulante a matar: %d - %d", id_patota, id_trip);
			t_tripulante* trip_to_kill = tripulante_de_lista(id_patota, id_trip);
			log_info(logger, "VOy a cancelar el hilo");
			list_remove(lista_tripulantes, posicion_trip(id_patota, id_trip));
			log_info(logger, "VOy a cancelar el hilo");
			pthread_cancel(*trip_to_kill->hilo);
			trip_to_kill->vivir = false;
			pthread_join(*trip_to_kill->hilo, NULL);
			log_info(logger, "VOy a enviar_mensaje");
			agregar_parametro_a_mensaje(mensaje_out, (void *)id_trip, ENTERO);
			agregar_parametro_a_mensaje(mensaje_out, (void *)id_patota, ENTERO);
			enviar_mensaje(socket_ram, mensaje_out);
			liberar_mensaje_out(mensaje_out);
			log_info(logger, "VOy a recibir_mensaje");
			mensaje_in = recibir_mensaje(socket_ram);
			if(!validar_mensaje(mensaje_in, logger)) {
				log_info(logger, "El servidor ha muerto, doy por finalizada esta wea");
				close(socket_ram);
				close(socket_mongo);
				log_destroy(logger);
				return ERROR_CONEXION;
			}
			liberar_mensaje_in(mensaje_in);
			break;
		case INICIAR_PLANIFICACION:
			sem_post(&semaforo_tripulante);
			break;
		case PAUSAR_PLANIFICACION:
			break;
		case OBTENER_BITACORA:
			log_info(logger,"OBTENER BITACORA");
			break;
		case EXIT_DISCORDIADOR:
			mensaje_out = crear_mensaje(64);
			enviar_mensaje(socket_ram, mensaje_out);
			liberar_mensaje_out(mensaje_out);
			
			sem_destroy(&semaforo_tripulante);
			continuar = false;
			break;
		case ERROR:
			log_error(logger,"COMANDO INVALIDO, INTENTE NUEVAMENTE");
		}
		free(buffer_consola);
	}
	log_destroy(logger);
	return 0;
}

void* rutina_hilos(void* mi_tripulante) {
	t_log* logger = log_create("discordiador.log", "HILOX", 0, LOG_LEVEL_DEBUG);
	log_info(logger, "HOLA MUNDO, SOY UN HILO %d", variable);
	// variable--;
	int agregado_en_x = 0;

	t_mensaje* mensaje_out = crear_mensaje(ACTU_E);
	agregar_parametro_a_mensaje(mensaje_out, (void *)3, ENTERO);
	enviar_mensaje(((t_tripulante *)mi_tripulante)->socket, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	t_list* mensaje_in = recibir_mensaje(((t_tripulante *)mi_tripulante)->socket);
	liberar_mensaje_in(mensaje_in);
	while(1) {
		sleep(10000);
	}

	while(1) {
		if(!((t_tripulante *)mi_tripulante)->vivir)
			break;
		
		mensaje_out = crear_mensaje(ACTU_P);
		agregar_parametro_a_mensaje(mensaje_out, (void *)((t_tripulante *)(void *)mi_tripulante)->px + agregado_en_x, ENTERO);
		agregar_parametro_a_mensaje(mensaje_out, (void *)((t_tripulante *)(void *)mi_tripulante)->py, ENTERO);
		enviar_mensaje(((t_tripulante *)mi_tripulante)->socket, mensaje_out);
		liberar_mensaje_out(mensaje_out);

		mensaje_in = recibir_mensaje(((t_tripulante *)mi_tripulante)->socket);
		liberar_mensaje_in(mensaje_in);

		agregado_en_x++;
		sleep(4);
		if(((t_tripulante *)mi_tripulante)->muero_rapido && agregado_en_x == 3)
			break;
	}
	for(int i = 0; i < 4; i++) {
		mensaje_out = crear_mensaje(NEXT_T);
		enviar_mensaje(((t_tripulante *)mi_tripulante)->socket, mensaje_out);
		liberar_mensaje_out(mensaje_out);

		mensaje_in = recibir_mensaje(((t_tripulante *)mi_tripulante)->socket);
		if((uint32_t)list_get(mensaje_in, 0) == TASK_T)
			printf("Tarea: %s\n", (char *)list_get(mensaje_in, 1));
		else
			printf("No hay más tareas\n");
		liberar_mensaje_in(mensaje_in);
	}

	mensaje_out = crear_mensaje(ELIM_T);
	enviar_mensaje(((t_tripulante *)mi_tripulante)->socket, mensaje_out);
	liberar_mensaje_out(mensaje_out);
	printf("ME MUEROOOOOO -*-\n");
	return 0;
}

int posicion_trip(uint32_t id_patota, uint32_t id_trip) {
	int posicion = -1;
	bool encontre = false;
	// printf("Pos trip\n");
	t_link_element* iterador_tripulante = lista_tripulantes->head;
	// printf("Largo lista_tripulantes: %d\n", list_size(lista_tripulantes));
	t_tripulante* trip_auxiliar;
	// printf("2\n");
	while(iterador_tripulante) {
		
		posicion++;
		trip_auxiliar = (t_tripulante *)iterador_tripulante->data;
		if(trip_auxiliar->id_patota == id_patota && trip_auxiliar->id_trip == id_trip) {
			encontre = true;
			break;
		}
		iterador_tripulante = iterador_tripulante->next;
		// printf("posicion %d\n", posicion);
	}
	// printf("Encontre %d\n", posicion);
	if(encontre)
		return posicion;
	else
		return -1;
}

t_tripulante* tripulante_de_lista(uint32_t id_patota, uint32_t id_trip) {
	// printf("Voy a obtener tripulante\n");
	t_tripulante* tripulante = (t_tripulante *)list_get(lista_tripulantes, posicion_trip(id_patota, id_trip));
	// printf("Obtuve tripulante\n");
	return tripulante;
}
/*	Salida pruebas antesde compactacion
SEGMENTO 1/Duenio: 1/Indice: 0/Inicio: 0/Tamanio: 8
SEGMENTO 2/Duenio: 1/Indice: 1/Inicio: 8/Tamanio: 32
SEGMENTO 3/Duenio: 4/Indice: 0/Inicio: 40/Tamanio: 8
SEGMENTO 4/Duenio: 5/Indice: 0/Inicio: 48/Tamanio: 8
SEGMENTO 5/Duenio: 0/Indice: 2/Inicio: 56/Tamanio: 5
SEGMENTO 6/Duenio: 1/Indice: 3/Inicio: 61/Tamanio: 21
SEGMENTO 7/Duenio: 2/Indice: 0/Inicio: 82/Tamanio: 8
SEGMENTO 8/Duenio: 2/Indice: 1/Inicio: 90/Tamanio: 32
SEGMENTO 9/Duenio: 0/Indice: 2/Inicio: 122/Tamanio: 21
SEGMENTO 10/Duenio: 2/Indice: 3/Inicio: 143/Tamanio: 21
SEGMENTO 11/Duenio: 0/Indice: 4/Inicio: 164/Tamanio: 21
SEGMENTO 12/Duenio: 3/Indice: 0/Inicio: 185/Tamanio: 8
SEGMENTO 13/Duenio: 3/Indice: 1/Inicio: 193/Tamanio: 32
SEGMENTO 14/Duenio: 0/Indice: 2/Inicio: 225/Tamanio: 21
SEGMENTO 15/Duenio: 3/Indice: 3/Inicio: 246/Tamanio: 21
SEGMENTO 16/Duenio: 4/Indice: 1/Inicio: 267/Tamanio: 32
SEGMENTO 17/Duenio: 0/Indice: -1219298736/Inicio: 299/Tamanio: 1

Luego de compactacion
SEGMENTO 1/		Duenio: 1/Indice: 0/Inicio: 0/Tamanio: 8
SEGMENTO 2/		Duenio: 1/Indice: 1/Inicio: 8/Tamanio: 32
SEGMENTO 3/		Duenio: 4/Indice: 0/Inicio: 40/Tamanio: 8
SEGMENTO 4/		Duenio: 5/Indice: 0/Inicio: 48/Tamanio: 8
SEGMENTO 5/		Duenio: 1/Indice: 3/Inicio: 56/Tamanio: 21
corrimiento = 5
SEGMENTO 6/		Duenio: 2/Indice: 0/Inicio: 77/Tamanio: 8
SEGMENTO 7/		Duenio: 2/Indice: 1/Inicio: 85/Tamanio: 32
SEGMENTO 8/		Duenio: 2/Indice: 3/Inicio: 117/Tamanio: 21
corrimiento = 26
SEGMENTO 9/		Duenio: 3/Indice: 0/Inicio: 138/Tamanio: 8
SEGMENTO 10/	Duenio: 3/Indice: 1/Inicio: 146/Tamanio: 32
SEGMENTO 11/	Duenio: 3/Indice: 3/Inicio: 178/Tamanio: 21
corrrimiento = 47
SEGMENTO 12/	Duenio: 4/Indice: 1/Inicio: 200/Tamanio: 32
SEGMENTO 13/	Duenio: 0/Indice: 0/Inicio: 232/Tamanio: 68
*/

/*
NUEVOS RESULTADOS
Cantidad de segmentos: 15. Memoria libre: 16
SEGMENTO 1/Duenio: 1/Indice: 0/Inicio: 0/Tamanio: 8
SEGMENTO 2/Duenio: 1/Indice: 1/Inicio: 8/Tamanio: 32
SEGMENTO 3/Duenio: 4/Indice: 0/Inicio: 40/Tamanio: 8
SEGMENTO 4/Duenio: 5/Indice: 0/Inicio: 48/Tamanio: 8
SEGMENTO 5/Duenio: 1/Indice: 3/Inicio: 56/Tamanio: 21
SEGMENTO 6/Duenio: 2/Indice: 0/Inicio: 77/Tamanio: 8
SEGMENTO 7/Duenio: 2/Indice: 1/Inicio: 85/Tamanio: 32
SEGMENTO 8/Duenio: 2/Indice: 3/Inicio: 117/Tamanio: 21
SEGMENTO 9/Duenio: 3/Indice: 0/Inicio: 138/Tamanio: 8
SEGMENTO 10/Duenio: 3/Indice: 1/Inicio: 146/Tamanio: 32
SEGMENTO 11/Duenio: 3/Indice: 3/Inicio: 178/Tamanio: 21
SEGMENTO 12/Duenio: 4/Indice: 1/Inicio: 199/Tamanio: 32
SEGMENTO 13/Duenio: 5/Indice: 1/Inicio: 231/Tamanio: 32
SEGMENTO 14/Duenio: 5/Indice: 2/Inicio: 263/Tamanio: 21
SEGMENTO 15/Duenio: 0/Indice: 2/Inicio: 284/Tamanio: 16
Lista de patotas: 5
Patota 1. PID: 1; Puntero a PCB: 0; Puntero a tareas: 8
PID: 1; Puntero a tareas: 8
Patota 2. PID: 2; Puntero a PCB: 77; Puntero a tareas: 85
PID: 2; Puntero a tareas: 85
Patota 3. PID: 3; Puntero a PCB: 138; Puntero a tareas: 146
PID: 3; Puntero a tareas: 146
Patota 4. PID: 4; Puntero a PCB: 40; Puntero a tareas: 199
PID: 4; Puntero a tareas: 199
Patota 5. PID: 5; Puntero a PCB: 48; Puntero a tareas: 231
PID: 5; Puntero a tareas: 231
Lista de tripulantes activos: 4 :- 4
TID: 2; inicio: 56; estado: N; pos_x: 3; pos_y: 4; IP: 0; Punt PCB: 0
TID: 2; inicio: 117; estado: N; pos_x: 3; pos_y: 4; IP: 0; Punt PCB: 77
TID: 2; inicio: 178; estado: N; pos_x: 3; pos_y: 4; IP: 0; Punt PCB: 138
TID: 1; inicio: 263; estado: N; pos_x: 3; pos_y: 4; IP: 0; Punt PCB: 48
*/