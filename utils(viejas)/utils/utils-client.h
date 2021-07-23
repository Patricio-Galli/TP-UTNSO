/*
 * conexiones.h
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#ifndef UTILS_CLIENT_H_
#define UTILS_CLIENT_H_

#include "utils-conexiones.h"

int crear_conexion_cliente(char* ip, char* puerto);
void liberar_conexion(int socket_cliente);

#endif /* UTILS_CLIENT_H_ */