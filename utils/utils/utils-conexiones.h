/*
 * utils-conexiones.h
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#ifndef UTILS_CONEXIONES_H_
#define UTILS_CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/collections/list.h>

typedef enum {
	MENSAJE,
	PAQUETE
} op_code;

typedef struct {
	int size;
	void* stream;
} t_buffer;

typedef struct {
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

void* serializar_paquete(t_paquete* paquete, int bytes);

char* recibir_mensaje(int);
char* recibir_buffer(int* size, int socket);
int recibir_operacion(int);
void eliminar_paquete(t_paquete* paquete);

void enviar_mensaje(char* mensaje, int socket);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);

#endif /* UTILS_CONEXIONES_H_ */