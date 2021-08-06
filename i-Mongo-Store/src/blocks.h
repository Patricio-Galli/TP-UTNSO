#ifndef BLOCKS_H_
#define BLOCKS_H_

#include "global.h"
#include "mongo.h"

char* crear_MD5(char, int);
void sumar_caracteres(char, int);
void quitar_caracteres(char, int);
char* proximo_bloque_libre();
int escribir_caracter_en_bloque(char,int,char* ,int );
int borrar_caracter_en_bloque(char ,int ,char* ,int );
void actualizar_posicion(tripulante*, int, int,char*);
void comienza_tarea(char*,char*);
void finaliza_tarea(char*,char*);
void inicio_sabotaje(char*);
void fin_sabotaje(char*);
char* juntar_posiciones(int, int);
int escribir_caracter_en_bitacora(char*,int,char*,int);
void escribir_mensaje_en_bitacora(char* , char* );
int roundUp(int, int);

#endif /* BLOCKS_H_ */
