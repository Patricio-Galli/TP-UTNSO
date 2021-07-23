/*
 * utils-server.h
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#ifndef UTILS_SERVER_H_
#define UTILS_SERVER_H_

#include "utils-conexiones.h"
#include <commons/log.h>

#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <net/if.h>

t_log* logger;

int crear_conexion_servidor(char *ip, int puerto);
int esperar_cliente(int);

t_list* recibir_paquete(int);

#endif /* UTILS_SERVER_H_ */