#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/config.h>
#include<dirent.h>
#include<commons/log.h>
#include<commons/string.h>
#include<utils/utils-server.h>
#include<commons/collections/list.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<dirent.h>



#define ERROR_CONEXION -1
//#define IP_RAM "192.168.0.104"
//#define PUERTO_RAM "9000"

#define IP_RAM "127.0.0.2"
#define PUERTO_RAM 9100
void* crear_superBloque(char*,int);
#endif /* SERVIDOR_H_ */

