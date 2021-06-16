#include "miramhq.h"

int main(void) {
	t_log* logger = log_create("miramhq.log", "Mi-RAM-HQ", 1, LOG_LEVEL_DEBUG);
	log_info(logger, "Creando conexiones");
	int server_fd = crear_conexion_servidor(IP_RAM, PUERTO_RAM);
	
	if(server_fd < 0) {
		log_info(logger, "Fallo en la conexión del servidor");
		close(server_fd);
		return ERROR_CONEXION;
	}

	log_info(logger, "Servidor listo para recibir al cliente");
	int socket_discord = esperar_cliente(server_fd);
	log_info(logger, "Ha llegado un nuevo cliente");

	t_list* lista;
	char* buffer;
	// while(1) {
		log_info(logger, "Entro a recibir mensaje");
		t_list* lista_parametros = recibir_mensaje(socket_discord);

		switch((int)list_get(lista_parametros, 0)) { // protocolo del mensaje
		case INIT_P:
			log_info(logger, "Largo de lista %d", lista_parametros->elements_count);
			int largo_lista = lista_parametros->elements_count;
			log_info(logger, "op code: %d", (int )list_get(lista_parametros, 0));
			// memcpy(&op_code, (int *)list_get(lista_parametros, 0), sizeof(int));
			log_info(logger, "Recibi id_ patota %d", (int)list_get(lista_parametros, 1));
			log_info(logger, "Recibi cant_tripulantes %d", (int)list_get(lista_parametros, 2));
			log_info(logger, "Recibi cant_tareas %d", (int)list_get(lista_parametros, 3));
			log_info(logger, "Tarea 1: %s", (char *)list_get(lista_parametros, 4));
			log_info(logger, "Tarea 2: %s", (char *)list_get(lista_parametros, 5));
			log_info(logger, "Hemos vencido al recibir mensaje");
			return 0;
			break;
		case INIT_T:
			//lista = recibir_paquete(socket_discord);
			printf("Me llegaron los siguientes valores:\n");
			break;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			return -1;
			/*recv(socket_discord, &cod_op, sizeof(int), MSG_WAITALL);
			log_info(logger, "Hemos vencido al recibir mensaje : %d", cod_op);
			recv(socket_discord, &cod_op, sizeof(int), MSG_WAITALL);
			log_info(logger, "Hemos vencido al recibir mensaje : %d", cod_op);*/
			break;
		}
	return EXIT_SUCCESS;
}


int* crear_conexiones_hilos(char** input, int* cantidad_detectada) {
	int cantidad_tripulantes = atoi(input[1]);
	int* lista_sockets = malloc(sizeof(int) * cantidad_tripulantes);

	for(int iterador = 0; iterador < cantidad_tripulantes; iterador++) {
		lista_sockets[iterador] = crear_conexion_servidor(IP_RAM, 0);
		if(lista_sockets[iterador] < 0) {
			printf("No se pudo crear socket %d\n", iterador);
		}
		else *cantidad_detectada++;
	}
	printf("EN este momento cantidad_detectada vale: %d\n", *cantidad_detectada);
	return lista_sockets;
}