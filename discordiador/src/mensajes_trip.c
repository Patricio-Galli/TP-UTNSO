#include "mensajes_trip.h"

bool respuesta_OK(t_list* respuesta, char* mensaje_fallo) {
	if(!validar_mensaje(respuesta, logger)) {
		log_warning(logger, "FALLO EN COMUNICACION");
		return false;
	}else if((int)list_get(respuesta, 0) != TODOOK) {
		log_warning(logger, "%s", mensaje_fallo);
		return false;
	}
	return true;
}

void enviar_y_verificar(t_mensaje* mensaje_out, int socket, char* mensaje_error) {
	enviar_mensaje(socket, mensaje_out);
	t_list* mensaje_in = recibir_mensaje(socket);
	respuesta_OK(mensaje_in, mensaje_error);

	liberar_mensaje_in(mensaje_in);
	liberar_mensaje_out(mensaje_out);
}

char* solicitar_tarea(tripulante* trip) {
	char* tarea = "no_task";

	if(0){//if(RAM_ACTIVADA) {
		t_mensaje* mensaje_out = crear_mensaje(NEXT_T);
		enviar_mensaje(trip->socket_ram, mensaje_out);
		t_list* mensaje_in = recibir_mensaje(trip->socket_ram);

		log_error(logger, "Trip %d recibe mensaje %d, tarea %s.", trip->id_trip, (int)list_get(mensaje_in, 0), (char*)list_get(mensaje_in, 1));

		if(!validar_mensaje(mensaje_in, logger))
			log_warning(logger, "FALLO EN MENSAJE CON HILO RAM\n");
		else if((int)list_get(mensaje_in, 0) == TASK_T)
			tarea = (char*)list_get(mensaje_in, 1);

		liberar_mensaje_out(mensaje_out);
		liberar_mensaje_in(mensaje_in);
	} else {
		switch(trip->posicion[0]) {
			case 3:
				tarea = "GENERAR_OXIGENO 10;6;0;4";
				break;
			case 6:
				tarea = "CONSUMIR_OXIGENO 5;8;5;4";
				break;
			case 8:
				tarea = "DESCARTAR_BASURA 13;13;13;13";
				break;
			case 13:
				break;
			default:
				tarea = "ESPERAR;3;3;3";
				break;
		}
	}

	return tarea;
}

void avisar_movimiento(tripulante* trip) {
	if(RAM_ACTIVADA) {
		t_mensaje* mensaje_out = crear_mensaje(ACTU_P);
		agregar_parametro_a_mensaje(mensaje_out, (void*)trip->posicion[0], ENTERO);
		agregar_parametro_a_mensaje(mensaje_out, (void*)trip->posicion[1], ENTERO);

		enviar_y_verificar(mensaje_out, trip->socket_ram, "Fallo en la actualizacion de posicion en RAM");
	}

	if(MONGO_ACTIVADO) {
		t_mensaje* mensaje_out = crear_mensaje(ACTU_P);
		agregar_parametro_a_mensaje(mensaje_out, (void*)trip->posicion[0], ENTERO);
		agregar_parametro_a_mensaje(mensaje_out, (void*)trip->posicion[1], ENTERO);

		enviar_y_verificar(mensaje_out, trip->socket_mongo, "Fallo en la actualizacion de posicion en MONGO");
	}
}

void actualizar_estado(tripulante* trip, estado estado_trip) {
	trip->estado = estado_trip;

	if(0){//if(RAM_ACTIVADA) {
		t_mensaje* mensaje_out = crear_mensaje(ACTU_E);
		agregar_parametro_a_mensaje(mensaje_out, (void*)trip->estado, ENTERO);

		enviar_y_verificar(mensaje_out, trip->socket_ram, "Fallo en la actualizacion del estado en RAM");
	}
}
