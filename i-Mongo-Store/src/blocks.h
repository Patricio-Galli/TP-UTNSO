#ifndef BLOCKS_H_
#define BLOCKS_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<commons/config.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/bitarray.h>
#include <commons/collections/dictionary.h>
#include<sys/stat.h>
#include<dirent.h>
#include<fcntl.h>//open
#include<pthread.h>
#include<sys/mman.h>
#include "imongostore.h"

typedef struct {
	int posicion_x;
	int posicion_y;
	int id_trip;
	int id_patota;
	int socket_discord;
}tripulante;

#define MAX(x,y) (((x)>(y)) ? (x) : (y))

char* crear_MD5(char, int);
void sumar_caracteres(char, int);
void quitar_caracteres(char, int);
char* proximo_bloque_libre();
int escribir_caracter_en_bloque(char,int,char* ,int );
int borrar_caracter_en_bloque(char ,int ,char* ,int );
void actualizar_posicion(tripulante*, int, int,char*);
char* juntar_posiciones(int, int);
int escribir_caracter_en_bitacora(char*,int,char*,int);

int roundUp(int, int);
#endif /* BLOCKS_H_ */
