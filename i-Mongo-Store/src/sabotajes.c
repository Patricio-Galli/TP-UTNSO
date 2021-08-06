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

		char* DIR_bitacora = obtener_directorio("/Files/Bitacoras/Tripulante");
		char* mensaje_inicio = string_new();
		char* id_trip_str = string_itoa(id_trip);
		char* id_patota_str = string_itoa(id_patota);
		string_append(&DIR_bitacora,id_trip_str);
		string_append(&DIR_bitacora,"-");
		string_append(&DIR_bitacora,id_patota_str);
		string_append(&DIR_bitacora,".ims");

		string_append(&mensaje_inicio,"El tripulante ");
		string_append(&mensaje_inicio,id_trip_str);
		string_append(&mensaje_inicio," de la patota ");
		string_append(&mensaje_inicio,id_patota_str);
		string_append(&mensaje_inicio," empezo a moverse hacia la ubicacion del sabotaje.");

		log_info(logger, mensaje_inicio);
		escribir_mensaje_en_bitacora(mensaje_inicio, DIR_bitacora);

		enviar_mensaje(socket_sabotajes, mensaje_out);
		liberar_mensaje_in(mensaje_in);

		mensaje_in = recibir_mensaje(socket_sabotajes);

		if((int)list_get(mensaje_in, 0) == SABO_F) {
			log_info(logger, "El tripulante llego a la ubicacion del sabotaje");
			escribir_mensaje_en_bitacora("El tripulante llego a la ubicacion del sabotaje.", DIR_bitacora);
			enviar_mensaje(socket_sabotajes, mensaje_out);
			//todo RESOLVER SABOTAJE
			contador_sabotajes++;
		} else
			log_error(logger, "Se Murio");

		free(id_trip_str);
		free(id_patota_str);
		free(mensaje_inicio);
		free(DIR_bitacora);
		liberar_mensaje_in(mensaje_in);
		liberar_mensaje_out(mensaje_out);
	}else {
		log_error(logger, "No se pudo resolver el sabotaje");
		liberar_mensaje_in(mensaje_in);
		liberar_mensaje_out(mensaje_out);
	}
}
