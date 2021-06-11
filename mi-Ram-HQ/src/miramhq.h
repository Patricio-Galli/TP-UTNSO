#ifndef _MIRAMHQ_H_
#define _MIRAMHQ_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<utils/utils-server.h>
#include<commons/collections/list.h>

#define ERROR_CONEXION -1
//#define IP_RAM "192.168.0.104"
//#define PUERTO_RAM "9000"

#define IP_RAM "127.0.0.1"
#define PUERTO_RAM 9000

int* crear_conexiones_hilos(char** buffer, int* cantidad_detectada);

#endif /* _MIRAMHQ_H_ */