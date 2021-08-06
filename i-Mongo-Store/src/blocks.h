#ifndef BLOCKS_H_
#define BLOCKS_H_

#include "global.h"
#include "mongo.h"
#include "utilidades.h"
//#include "bitacora.h"

void sumar_caracteres(char, int);
void quitar_caracteres(char, int);
char* proximo_bloque_libre();
int escribir_caracter_en_bloque(char,int,char* ,int );
int borrar_caracter_en_bloque(char ,int ,char* ,int );

#endif /* BLOCKS_H_ */
