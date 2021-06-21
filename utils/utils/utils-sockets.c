#include "utils-sockets.h"

int puerto_desde_socket(int socket) {
	struct addrinfo hints;
	socklen_t largo = sizeof(struct sockaddr);
	if(getsockname(socket, &(hints.ai_addr), &largo) != 0) {
		// perror("getsockname");
		return -1;
	}
	int puerto_coor = ntohs(((struct sockaddr_in *)&(hints.ai_addr))->sin_port);
	return puerto_coor;
}

bool validar_socket_servidor(int socket, t_log* logger) {
    switch (socket){
    case -1:
        log_error(logger, "Error al crear socket. La conexión falló en getaddrinfo");
        break;
    case -2:
        log_error(logger, "Error al crear socket. Puerto no disponible");
        break;
     case -3:
        log_error(logger, "Error al crear socket. Fallo al conectarse al servidor");
        break;
    default:
        return true;
    }
    return false;
}

void data_socket(int socket, t_log* logger) {
	char ipstr[INET_ADDRSTRLEN];
	struct sockaddr_in dir_cliente;
	socklen_t largo = sizeof(struct addrinfo);
	
	getsockname(socket, (struct sockaddr *)&dir_cliente, &largo);
	inet_ntop(AF_INET, &(dir_cliente.sin_addr), ipstr, sizeof ipstr);
	log_info(logger, "Dirección del servidor: %s:%d\n", ipstr, ntohs(dir_cliente.sin_port));

	if(getpeername(socket, (struct sockaddr *)&dir_cliente, &largo) == 0) {
		inet_ntop(AF_INET, &(dir_cliente.sin_addr), ipstr, sizeof ipstr);
		log_info(logger, "Dirección cliente: %s:%d\n", ipstr, ntohs(dir_cliente.sin_port));
	}
}