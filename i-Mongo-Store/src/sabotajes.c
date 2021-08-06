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
	log_info(logger, "Sabotaje recibido");
}

/*
void* detector_sabotajes(void* s) {
	int socket_detector = esperar_cliente(*(int *)s);
	int contador = 0;
	char** ubicaciones = config_get_array_value(config, "POSICIONES_SABOTAJE");

	log_info(logger, "Detector de sabotajes iniciado exitosamente");

	while(1) {
		sleep(9999);

		char** ubicacion_dividida = string_split(ubicaciones[contador], "|");
		int pos_x = atoi(ubicacion_dividida[0]);
		int pos_y = atoi(ubicacion_dividida[1]);
		liberar_split(ubicacion_dividida);

		log_warning(logger, "ENVIANDO SABOTAJE AL DISCORDIDADOR");
		t_mensaje* mensaje_out = crear_mensaje(SABO_P);

		agregar_parametro_a_mensaje(mensaje_out, (void*)pos_x, ENTERO);
		agregar_parametro_a_mensaje(mensaje_out, (void*)pos_y, ENTERO);

		enviar_mensaje(socket_detector, mensaje_out);
		liberar_mensaje_out(mensaje_out);

		t_list* mensaje_in = recibir_mensaje(socket_detector);

		mensaje_out = crear_mensaje(TODOOK);

		if((int)list_get(mensaje_in, 0) == SABO_I) {
			log_info(logger, "Tripulante empezo a resolver el sabotaje");
			enviar_mensaje(socket_detector, mensaje_out);
			liberar_mensaje_in(mensaje_in);

			mensaje_in = recibir_mensaje(socket_detector);

			if((int)list_get(mensaje_in, 0) == SABO_F) {
				log_info(logger, "Tripulante termino de resolver el sabotaje");
				enviar_mensaje(socket_detector, mensaje_out);
			}

			liberar_mensaje_in(mensaje_in);
			liberar_mensaje_out(mensaje_out);
		}else
			log_error(logger, "No se pudo resolver el sabotaje");

		sleep(600);
	}
}
*/
