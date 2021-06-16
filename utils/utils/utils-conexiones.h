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
#include<commons/log.h>

typedef enum { 	INIT_P, INIT_T, DATA_T, SHOW_T, ELIM_T, NEW_PO, SND_PO, ACTU_T,
				NEXT_T,	TASK_T,	BITA_D,	BITA_T,	BITA_C,	TAR_ES,	SABO_P,	TODOOK
} protocolo_msj;

typedef enum {
	ENTERO,
	BUFFER
} tipo_msj;

typedef struct {
	int tamanio;
	void* contenido;
} t_buffer;

typedef struct {
	protocolo_msj op_code;
	t_buffer* buffer;
} t_mensaje;

t_mensaje* crear_mensaje(protocolo_msj cod_op);
void agregar_parametro_a_mensaje(t_mensaje* mensaje, void* parametro, int tipo_parametro, t_log* logger);
void enviar_mensaje(int socket, t_mensaje* mensaje);

t_list* recibir_mensaje(int socket);
#endif /* UTILS_CONEXIONES_H_ */