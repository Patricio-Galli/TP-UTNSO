#include "mongo.h"

t_config* config;
t_log* logger;

int tripu, pat = 0;

int main() {

	logger = log_create("mongo.log", "MONGO", 1, LOG_LEVEL_INFO);
    config = config_create("mongo.config");

	int server_fd = crear_conexion_servidor(IP_MONGO, config_get_int_value(config, "PUERTO"), 1);

	if(!validar_socket(server_fd, logger)) {
		close(server_fd);
		log_destroy(logger);
		return ERROR_CONEXION;
	}

	log_info(logger, "Servidor listo");
	int socket_discord = esperar_cliente(server_fd);
	log_info(logger, "Conexión establecida con el discordiador");

	while(1) {
		log_info(logger, "Esperando información del discordiador");

		t_list* mensaje_in = recibir_mensaje(socket_discord);
		t_mensaje* mensaje_out;

		if (!validar_mensaje(mensaje_in, logger)) {
			log_info(logger, "Cliente desconectado dentro del while");
			close(server_fd);
			log_destroy(logger);
			return ERROR_CONEXION;
		}

		switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
			case INIT_S:
				log_info(logger, "Iniciando el detector de sabotajes");
				int socket_detector = crear_conexion_servidor(IP_MONGO, 0, 1);

				pthread_t hilo_detector_sabotajes;
				pthread_create(&hilo_detector_sabotajes, NULL, detector_sabotajes, &socket_detector);

				mensaje_out = crear_mensaje(SND_PO);
				agregar_parametro_a_mensaje(mensaje_out, (void *)puerto_desde_socket(socket_detector), ENTERO);
				enviar_mensaje(socket_discord, mensaje_out);

				break;
			case INIT_P:
				log_info(logger, "Discordiador inicio una patota");

				pat++;
				tripu = 1;

				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_discord, mensaje_out);
				break;
			case INIT_T:
				log_info(logger, "Discordiador solicito iniciar un tripulante");
				tripulante* trip = malloc(sizeof(tripulante));

				trip->id_patota = pat;
				trip->id_trip = tripu;
				trip->posicion_x = (int)list_get(mensaje_in, 1);
				trip->posicion_y = (int)list_get(mensaje_in, 2);
				trip->socket_discord = crear_conexion_servidor(IP_MONGO, 0, 1);

				pthread_t hilo_nuevo;
				pthread_create(&hilo_nuevo, NULL, rutina_trip, trip);

				mensaje_out = crear_mensaje(SND_PO);
				agregar_parametro_a_mensaje(mensaje_out, (void *)puerto_desde_socket(trip->socket_discord), ENTERO);
				enviar_mensaje(socket_discord, mensaje_out);

				tripu++;
				break;
			case BITA_D:
				log_info(logger, "Discordiador solicito la bitacora del tripulante %d de la patota %d", (int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2));
				int cantidad_lineas_archivo = 4;

				mensaje_out = crear_mensaje(BITA_C);

				agregar_parametro_a_mensaje(mensaje_out, (void*)cantidad_lineas_archivo, ENTERO);
				for(int i = 1; i <= cantidad_lineas_archivo; i++) {
					char linea[25];
					sprintf(linea, "%d) BITACORA ..........", i);

					agregar_parametro_a_mensaje(mensaje_out, (void*)linea, BUFFER); //esto es lo unico que tienen que mantener adentro del for, despues tienen que ir cargando en linea las lineas del archivo de bitacora
				}

				enviar_mensaje(socket_discord, mensaje_out);
				break;
			default:
				log_warning(logger, "No entendi el mensaje");
				break;
		}
		list_destroy(mensaje_in);
		liberar_mensaje(mensaje_out);
	}

	log_warning(logger, "FINALIZANDO MONGO");
	return 0;
}

void* detector_sabotajes(void* s) {
	int socket_detector = esperar_cliente(*(int *)s);
	int posicion_x = 8, posicion_y = 8;

	log_info(logger, "Detector de sabotajes iniciado exitosamente");

	while(1) {
		sleep(10);

		t_mensaje* mensaje_out = crear_mensaje(SABO_P);

		agregar_parametro_a_mensaje(mensaje_out, (void*)posicion_x, ENTERO);
		agregar_parametro_a_mensaje(mensaje_out, (void*)posicion_y, ENTERO);

		enviar_mensaje(socket_detector, mensaje_out);

		liberar_mensaje(mensaje_out);
	}
}

void* rutina_trip(void* t) {
	tripulante* trip = (tripulante*) t;

	int socket_cliente = esperar_cliente(trip->socket_discord);
	log_info(logger, "Iniciado el tripulante %d de la patota %d", trip->id_trip, trip->id_patota);

	while(1) {
		t_mensaje* mensaje_out;
		t_list* mensaje_in = recibir_mensaje(socket_cliente);

		if(!validar_mensaje(mensaje_in, logger))
			log_info(logger, "Fallo el mensaje");

		switch ((int)list_get(mensaje_in, 0)) {
			case EXEC_1:
				log_info(logger, "EXEC_1 - Tripulante %d de la patota %d ejecutando tarea: %s", trip->id_trip, trip->id_patota, (char*)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case EXEC_0:
				log_info(logger, "EXEC_0 - Tripulante %d de la patota %d detuvo ejecucion", trip->id_trip, trip->id_patota);
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case GEN_OX:
				log_info(logger, "GEN_OX - Tripulante %d de la patota %d generando %d de oxigeno", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case CON_OX:
				log_info(logger, "CON_OX - Tripulante %d de la patota %d consumiendo %d de oxigeno", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case GEN_CO:
				log_info(logger, "GEN_CO - Tripulante %d de la patota %d generando %d de comida", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case CON_CO:
				log_info(logger, "CON_CO - Tripulante %d de la patota %d consumiendo %d de comida", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case GEN_BA:
				log_info(logger, "GEN_BA - Tripulante %d de la patota %d generando %d de basura", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case DES_BA:
				log_info(logger, "DES_BA - Tripulante %d de la patota %d desechando %d de basura", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case ACTU_T:
				log_info(logger, "ACTU_T - Tripulante %d de la patota %d nueva posicion: %d|%d", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
		}
		liberar_mensaje(mensaje_out);
		list_destroy(mensaje_in);
	}
}
