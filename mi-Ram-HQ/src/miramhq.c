#include "miramhq.h"

int main(void)
{
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
	log_info(logger, "A llegado un nuevo cliente");


	// crear_memoria(); → malloc(fruta);
	// 
	// crear_pcb();
	// crear_tcb();


	t_list* lista;
	char* buffer;
	while(1) {
		int cod_op = recibir_operacion(socket_discord);
		switch(cod_op) {
		case MENSAJE:
			buffer = recibir_mensaje(socket_discord);
			int cantidad_detectada = 0;
			log_info(logger, "Recibi un mensaje: %s", buffer);

			char** input = string_split(buffer, " ");
			int* lista_sockets = crear_conexiones_hilos(input, &cantidad_detectada);
			printf("tripulantes a crear: %d, pude crear:%d\n", atoi(input[1]), cantidad_detectada);
			
			struct sockaddr_in addr_aux;
			socklen_t largo = sizeof(struct addrinfo);
			char ipstr[INET_ADDRSTRLEN];

			for(int i=0; i < atoi(input[1]); i++) {
				printf("VOY a enviar un puerto, del socket %d\n", lista_sockets[i]);
				getsockname(lista_sockets[i], (struct sockaddr*)&addr_aux, &largo);
				inet_ntop(AF_INET, &(addr_aux.sin_addr), ipstr, sizeof ipstr);
				char str_puerto[7];
				sprintf(str_puerto, "%d", ntohs(addr_aux.sin_port));
				printf("VOY a enviar un puerto, %s\n", str_puerto);
				// log_info(logger,"%s:%d\n", ipstr, ntohs(addr_aux.sin_port));
				
				enviar_mensaje(str_puerto, socket_discord);
			}
			
			break;
		case PAQUETE:
			lista = recibir_paquete(socket_discord);
			printf("Me llegaron los siguientes valores:\n");
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}
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
		//else *cantidad_detectada++;
	}
	printf("EN este momento cantidad_detectada vale: %d\n", *cantidad_detectada);
	return lista_sockets;
}
