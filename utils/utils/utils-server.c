/*
 * conexiones.c
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#include"utils-server.h"

int crear_conexion_servidor(char *ip, int puerto) {
	int socket_servidor;
	int error;
	struct addrinfo hints, *servinfo, *addr_aux;
	
	char str_puerto[7];
	sprintf(str_puerto, "%d", puerto);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	//hints.ai_flags = AI_PASSIVE;	// En pricipio es 0 (any)
	//hints.protocol = 0;			// TCP

	if ((error = getaddrinfo(ip, str_puerto, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(error));
		return -1;
	}

	int yes = 1;
	for (addr_aux=servinfo; addr_aux != NULL; addr_aux = addr_aux->ai_next) {
		if ((socket_servidor = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
			printf("fallo en socket\n");
			continue;
		}
		if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
			perror("setsockopt");
			continue;
		}
		
		if (bind(socket_servidor, addr_aux->ai_addr, addr_aux->ai_addrlen) == -1) {
			close(socket_servidor);
			printf("bind fail\n");
			continue;
		}
		
		break;
	}

	if(addr_aux == NULL) {
		return -1;
	}
	
	listen(socket_servidor, 1);
	if ((error = listen(socket_servidor, 1)) != 0) {
		fprintf(stderr, "socket_servidor error: %s\n", gai_strerror(error));
		return -1;
	}
	char ipstr[INET_ADDRSTRLEN];
	socklen_t largo = sizeof(hints);
	getsockname(socket_servidor, addr_aux->ai_addr, &largo);
	inet_ntop(AF_INET, &((struct sockaddr_in *)addr_aux->ai_addr)->sin_addr, ipstr, sizeof ipstr);
	printf("%s:%d\n", ipstr, ntohs(((struct sockaddr_in *)addr_aux->ai_addr)->sin_port));

	freeaddrinfo(servinfo);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor) {
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	// log_info(logger, "Se conecto un cliente!: %d", dir_cliente.sin_port);
	char ipstr[INET_ADDRSTRLEN];
	socklen_t largo = sizeof(struct addrinfo);
	
	getsockname(socket_servidor, (struct sockaddr *)&dir_cliente, &largo);
	inet_ntop(AF_INET, &(dir_cliente.sin_addr), ipstr, sizeof ipstr);
	printf("Dirección del servidor (ram): %s:%d\n", ipstr, ntohs(dir_cliente.sin_port));

	if(getpeername(socket_cliente, (struct sockaddr *)&dir_cliente, &largo) == 0) {
		inet_ntop(AF_INET, &(dir_cliente.sin_addr), ipstr, sizeof ipstr);
		printf("Dirección cliente (discordiador): %s:%d\n", ipstr, ntohs(dir_cliente.sin_port));
	}

	return socket_cliente;
}

//podemos usar la lista de valores para poder hablar del for y de como recorrer la lista


/*
struct ifreq ifr;
ifr.ifr_addr.sa_family = AF_INET;
strcpy(ifr.ifr_name, "enp0s9");

if (ioctl(socket_servidor, SIOCGIFADDR, &ifr) < 0) {
	printf("mori en ioctl\n");
	return -1;      //No encuentro el dispositivo
}
printf("IP de eth0: %s\n", inet_ntoa( (*(struct in_addr *) &ifr.ifr_addr.sa_data[2])));
*/