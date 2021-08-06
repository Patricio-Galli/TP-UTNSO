#include "sabotajes.h"

void inicializar_detector_sabotajes(int socket_discord) {
	ubicaciones_sabotajes = config_get_array_value(config, "POSICIONES_SABOTAJE");
	contador_sabotajes = 0;

	signal(SIGUSR1, analizador_sabotajes);

	int socket_d = crear_conexion_servidor(IP_MONGO, 0, 1);

	t_mensaje* mensaje_out = crear_mensaje(SND_PO);
	agregar_parametro_a_mensaje(mensaje_out, (void *)puerto_desde_socket(socket_d), ENTERO);
	enviar_mensaje(socket_discord, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	socket_sabotajes = esperar_cliente(socket_d);
	log_info(logger, "Detector de sabotajes iniciado exitosamente");
}

void analizador_sabotajes(int senial) {
	char** ubicacion_dividida = string_split(ubicaciones_sabotajes[contador_sabotajes], "|");
	int pos_x = atoi(ubicacion_dividida[0]);
	int pos_y = atoi(ubicacion_dividida[1]);
	liberar_split(ubicacion_dividida);

	log_info(logger, "Sabotaje recibido en ubicacion %d|%d", pos_x, pos_y);

	log_warning(logger, "Enviando sabotaje al discordiador");
	t_mensaje* mensaje_out = crear_mensaje(SABO_P);
	agregar_parametro_a_mensaje(mensaje_out, (void*)pos_x, ENTERO);
	agregar_parametro_a_mensaje(mensaje_out, (void*)pos_y, ENTERO);
	enviar_mensaje(socket_sabotajes, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	t_list* mensaje_in = recibir_mensaje(socket_sabotajes);

	mensaje_out = crear_mensaje(TODOOK);

	if((int)list_get(mensaje_in, 0) == SABO_I) {
		int id_trip = (int)list_get(mensaje_in, 1);
		int id_patota = (int)list_get(mensaje_in, 2);
		tripulante* trip = obtener_tripulante(id_trip, id_patota);

		log_info(logger, "El tripulante %d de la patota %d esta yendo a la ubicacion del sabotaje", id_trip, id_patota);
		inicio_sabotaje(trip->dir_bitacora);

		enviar_mensaje(socket_sabotajes, mensaje_out);
		liberar_mensaje_in(mensaje_in);

		mensaje_in = recibir_mensaje(socket_sabotajes);

		if((int)list_get(mensaje_in, 0) == SABO_F) {
			log_info(logger, "El tripulante llego a la ubicacion del sabotaje");
			fin_sabotaje(trip->dir_bitacora);
			enviar_mensaje(socket_sabotajes, mensaje_out);
			//todo RESOLVER SABOTAJE
			contador_sabotajes++;
		} else
			log_error(logger, "Se Murio");

		liberar_mensaje_in(mensaje_in);
		liberar_mensaje_out(mensaje_out);
	}else {
		log_error(logger, "No se pudo resolver el sabotaje");
		liberar_mensaje_in(mensaje_in);
		liberar_mensaje_out(mensaje_out);
	}
}
