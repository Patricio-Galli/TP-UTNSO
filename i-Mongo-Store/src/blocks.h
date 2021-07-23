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
#include "imongostore.h"


char* crear_MD5(char, int);
void agregar_Caracteres(char caracter_llenado, int cantidad_caracteres);
char* proximo_bloque_libre();
#endif /* BLOCKS_H_ */
