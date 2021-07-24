#ifndef SERVIDOR_H_
#define SERVIDOR_H_


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<unistd.h>
#include"utils/utils-mensajes.h"
#include<commons/config.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/bitarray.h>
#include<commons/collections/list.h>
#include<utils/utils-server.h>
#include<utils/utils-sockets.h>

#include<sys/stat.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<dirent.h>
#include<fcntl.h>//open
#include<pthread.h>
#include"blocks.h"



#define ERROR_CONEXION -1
//#define IP_MONGO "192.168.0.104"
//#define PUERTO_RAM "9000"

t_config* config;
t_bitarray *bitmap;
t_log* logger;
void* blocks_copy;

typedef struct {
	int posicion_x;
	int posicion_y;
	int id_trip;
	int id_patota;
	int socket_discord;
}tripulante;

#define IP_MONGO "127.0.0.2"
#define PUERTO_MONGO 9100
char* crear_superBloque(char*);
char* crear_blocks(char*);
void crear_metadata(char*);
void crear_bitacora(char*);
char* generar_directorio(char*);
char* obtener_directorio(char*);
void imprimir_bitmap(t_bitarray*);
void* detector_sabotajes(void* s);
void* rutina_trip(void* t);


#endif /* MONGO_H_ */

