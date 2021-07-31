#ifndef MONGO_H_
#define MONGO_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<unistd.h>

#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include<commons/string.h>
#include<commons/bitarray.h>
#include <utils/utils-server.h>
#include <utils/utils-sockets.h>
#include <utils/utils-mensajes.h>

#include<sys/stat.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<dirent.h>
#include<fcntl.h>//open
#include<pthread.h>
#include"blocks.h"

#define IP_MONGO "127.0.0.1"
#define ERROR_CONEXION -1

t_config* config;
t_bitarray *bitmap;
t_log* logger;
uint32_t block_size;
uint32_t blocks_amount;
void* blocks_copy;

pthread_mutex_t actualizar_blocks;
pthread_mutex_t actualizar_bitmap;
/*
typedef struct {
	int posicion_x;
	int posicion_y;
	int id_trip;
	int id_patota;
	int socket_discord;
}tripulante;
*/
char* crear_superBloque(char*);
char* crear_blocks(char*);
void uso_blocks(void*);
void crear_metadata(char*);
void crear_bitacora(char*);
char* generar_directorio(char*);
char* obtener_directorio(char*);
void imprimir_bitmap(t_bitarray*);
void* detector_sabotajes(void* s);
void* rutina_trip(void* t);
void* detector_sabotajes(void* s);
void* rutina_trip(void* t);

#endif /* MONGO_H_ */