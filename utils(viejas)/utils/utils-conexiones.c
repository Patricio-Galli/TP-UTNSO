#include "utils-conexiones.h"

void crear_buffer(t_mensaje* mensaje) {
	mensaje->buffer = malloc(sizeof(t_buffer));
	mensaje->buffer->tamanio = 0;
	mensaje->buffer->contenido = NULL;
}

t_mensaje* crear_mensaje(protocolo_msj cod_op) {
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->op_code = cod_op;
	crear_buffer(mensaje);
	return mensaje;
}

void agregar_parametro_a_mensaje(t_mensaje* mensaje, void* parametro, int tipo_parametro, t_log* logger) {
	switch (tipo_parametro) {
	case ENTERO:
		log_info(logger, "case: ENTERO tamanio = %d", mensaje->buffer->tamanio);
		log_info(logger, "Parametro %d", *(int *)parametro);
		mensaje->buffer->contenido = realloc(mensaje->buffer->contenido, mensaje->buffer->tamanio + sizeof(int));

		memcpy(mensaje->buffer->contenido + mensaje->buffer->tamanio, parametro, sizeof(int));
		mensaje->buffer->tamanio = mensaje->buffer->tamanio + sizeof(int);
		log_info(logger, "memcpy del valor: tamanio = %d", mensaje->buffer->tamanio);
		break;
	case BUFFER:
		log_info(logger, "case: BUFFER tamanio = %d. Parametro %s", mensaje->buffer->tamanio, parametro);
		int tamanio_buffer = (strlen(parametro) + 1) * sizeof(char);
		mensaje->buffer->contenido = realloc(mensaje->buffer->contenido, mensaje->buffer->tamanio + sizeof(int) + tamanio_buffer);

		memcpy(mensaje->buffer->contenido + mensaje->buffer->tamanio, &tamanio_buffer, sizeof(int));
		memcpy(mensaje->buffer->contenido + mensaje->buffer->tamanio + sizeof(int), parametro, tamanio_buffer);
		mensaje->buffer->tamanio = mensaje->buffer->tamanio + sizeof(int) + tamanio_buffer;
		log_info(logger, "%s, %d", parametro, tamanio_buffer);
		break;
	default:
		log_error(logger, "Mal uso de la función agregar_parametro_a_mensaje en el parámetro tipo");
		break;
	}
}

void enviar_mensaje(int socket, t_mensaje* mensaje) {
	int tamanio_buffer = mensaje->buffer->tamanio + sizeof(int);
	void * buffer_to_send = malloc(tamanio_buffer);

	memcpy(buffer_to_send, &(mensaje->op_code), sizeof(int));
	memcpy(buffer_to_send + sizeof(int), mensaje->buffer->contenido, mensaje->buffer->tamanio);

	send(socket, buffer_to_send, tamanio_buffer, 0);
	free(buffer_to_send);
}

void* recibir_parametro(int socket, tipo_msj tipo) {
	int parametro;
	int tamanio_buffer;
	switch (tipo) {
	case ENTERO:
		recv(socket, &parametro, sizeof(int), MSG_WAITALL);
		return (void *)parametro;
		break;
	case BUFFER:
		recv(socket, &tamanio_buffer, sizeof(int), MSG_WAITALL);
		char *buffer = malloc(tamanio_buffer * sizeof(char));
		recv(socket, buffer, tamanio_buffer, MSG_WAITALL);
		return buffer;
		break;
	}
	return NULL;
}

t_list* recibir_mensaje(int socket) {
	protocolo_msj op_code;
	t_list* lista_parametros = list_create();

	recv(socket, &op_code, sizeof(int), MSG_WAITALL);
	list_add(lista_parametros, (void *)op_code);

	switch(op_code) {
	case INIT_P:	// Iniciar patota
	// INIT_P | id_patota [int] | cantidad_tripulantes [int] | cant_tareas [int] | (tareas)
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));

		int cant_tareas = (int)list_get(lista_parametros, 3);
		for(int iterador = 0; iterador < cant_tareas; iterador++) {
			list_add(lista_parametros, recibir_parametro(socket, BUFFER));
		}
		break;

	case INIT_T:	// Iniciar tripulante
	// INIT_T | id_patota [int] | id_trip [int] | posicion_x [int] | posicion_y [int]
	case SABO_P:	// Nuevo Sabotaje
	// SABO_P | id_patota [int] | id_trip [int] | posicion_x [int] | posicion_y [int]
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		break;

	case SHOW_T:	// Mostrar información de tripulante
	// SHOW_T | estado [int] | posicion_x [int] | posicion_y [int]
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		break;

	case DATA_T:	// Conocer tripulante
	// DATA_T | id_tripulante [int] | id_patota [int]
	case ELIM_T:	// Expulsar tripulante
	// ELIM_T | id_tripulante [int] | id_patota [int]
	case ACTU_T:	// Actualizar ubicación
	// ACTU_T | posicion_x [int] | posicion_y [int]
	case BITA_D:	// Obtener bitácora
	// BITA_D | id_trip [int] | id_patota [int]
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		break;

	case BITA_C:	// Mostrar Bitácora
	// BITA_C | cant_lineas [int] | (líneas)
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));

		int cant_lineas = (int)list_get(lista_parametros, 1);
		for(int iterador = 0; iterador < cant_lineas; iterador++) {
			list_add(lista_parametros, recibir_parametro(socket, BUFFER));
		}
		break;

	case SND_PO:	// Enviar puerto
	// SND_PO | puerto [int]
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		break;

	case TASK_T:	// Enviar tarea
	// TASK_T | largo_tarea [int] | tarea [str]
		list_add(lista_parametros, recibir_parametro(socket, BUFFER));
		break;

	case NEW_PO:	// Solicitar Puerto
	// NEW_PO
	case NEXT_T:	// Próxima tarea
	// NEXT_T
	case TODOOK:	// Validación correcta
	// TODOOK
		break;

	case BITA_T:	// Actualizar bitácora
	// BITA_T | largo_mensaje [int] | mensaje [str]
		// por definir
		break;

	case TAR_ES:	// Tarea E/S
	// TAR_ES | tarea_code [str] | parametro_tarea [int]
		// por definir
		break;

	default:
		printf("default !!!\n");
	}
	return lista_parametros;
}
