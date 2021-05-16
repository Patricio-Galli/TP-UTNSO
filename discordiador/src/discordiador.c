/*
 ============================================================================
 Name        : A_MongOs.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "discordiador.h"

int main(void) {
	// PARTE 2
	t_log* logger = iniciar_logger();
	log_info(logger, "Soy un log");

	t_config* config = leer_config();
	char* valor = config_get_string_value(config, "CLAVE");
	log_info(logger, valor);

	leer_consola(logger);

	// PARTE 3
	//conexion a RAM_HQ
	int conexion_Ram_HQ = crear_conexion(
		config_get_string_value(config, "IP"),
		config_get_string_value(config, "PUERTO")
	);

	enviar_mensaje(valor, conexion_Ram_HQ);

	t_paquete* paquete = armar_paquete();

	enviar_paquete(paquete, conexion_Ram_HQ);

	// Viejo terminar programa
	eliminar_paquete(paquete);
	log_destroy(logger);
	config_destroy(config);
	close(conexion_Ram_HQ);
}
//conexion a iMongo_Store
/*int conexion_Imongo_Store = crear_conexion(
			config_get_string_value(config, "IP"),
			config_get_string_value(config, "PUERTO")
		);
*/
t_log* iniciar_logger() {
	return log_create("discordiador.log", "A_MongOs", 1, LOG_LEVEL_INFO);
}

t_config* leer_config() {
	return config_create("discordiador.config");
}
void leer_consola(t_log* logger) {
	char* leido = readline(">");

	while(strncmp(leido, "", 1) != 0) {
		log_info(logger, leido);
		free(leido);
		leido = readline(">");
	}

	free(leido);
}

t_paquete* armar_paquete() {
	t_paquete* paquete = crear_paquete();

	char* leido = readline(">");

	while(strncmp(leido, "", 1) != 0) {
		agregar_a_paquete(paquete, leido, strlen(leido) + 1);
		free(leido);
		leido = readline(">");
	}

	free(leido);

	return paquete;
}

