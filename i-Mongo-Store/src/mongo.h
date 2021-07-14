#ifndef MONGO_H_
#define MONGO_H_

#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>

#include <utils/utils-server.h>
#include <utils/utils-sockets.h>
#include <utils/utils-mensajes.h>

#include <pthread.h>

#define IP_MONGO "127.0.0.1"
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
