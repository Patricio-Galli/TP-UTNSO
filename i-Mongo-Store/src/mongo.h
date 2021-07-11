

#ifndef MONGO_H_
#define MONGO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <utils/utils-server.h>
#include <utils/utils-sockets.h>
#include <utils/utils-mensajes.h>

#include <commons/collections/list.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define IP_MONGO "192.168.0.105"
#define ERROR_CONEXION -1

typedef struct {
	int posicion_x;
	int posicion_y;
	int id_trip;
	int id_patota;
	int socket_discord;
}tripulante;

void* rutina_trip(void* t);

#endif /* MONGO_H_ */
