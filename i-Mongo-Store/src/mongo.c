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

		if (!validar_mensaje(mensaje_in, logger)) {
			log_info(logger, "Cliente desconectado dentro del while");
			close(server_fd);
			log_destroy(logger);
			return ERROR_CONEXION;
		}

		switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
			case INIT_P:
				log_info(logger, "Discordiador inicio una patota");

				pat++;
				tripu = 1;

				t_mensaje* mensaje_ou = crear_mensaje(TODOOK);
				enviar_mensaje(socket_discord, mensaje_ou);
				liberar_mensaje(mensaje_ou);

				break;
			case INIT_T:
				log_info(logger, "Discordiador solicitó iniciar un tripulante");
				tripulante* trip = malloc(sizeof(tripulante));

				trip->id_patota = pat;
				trip->id_trip = tripu;
				trip->posicion_x = (int)list_get(mensaje_in, 1);
				trip->posicion_y = (int)list_get(mensaje_in, 2);
				trip->socket_discord = crear_conexion_servidor(IP_MONGO, 0, 1);

				pthread_t hilo_nuevo;
				pthread_create(&hilo_nuevo, NULL, rutina_trip, trip);

				t_mensaje* mensaje_out = crear_mensaje(SND_PO);
				agregar_parametro_a_mensaje(mensaje_out, (void *)puerto_desde_socket(trip->socket_discord), ENTERO);
				enviar_mensaje(socket_discord, mensaje_out);

				liberar_mensaje(mensaje_out);
				tripu++;

				break;
			default:
				log_warning(logger, "No entendi el mensaje");
				break;
		}
		list_destroy(mensaje_in);
	}

	log_warning(logger, "FINALIZANDO MONGO");
	return 0;
}

void* rutina_trip(void* t) {
	tripulante* trip = (tripulante*) t;

	int socket_cliente = esperar_cliente(trip->socket_discord);

	t_list* mensaje_in = recibir_mensaje(socket_cliente);

	if((int)list_get(mensaje_in, 0)==TODOOK)
		log_info(logger, "Iniciado correctamente el tripulante %d de la patota %d en MONGO", trip->id_trip, trip->id_patota);

	list_destroy(mensaje_in);

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
