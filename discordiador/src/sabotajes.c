#include "sabotajes.h"

void emergency_trips_running() {
	while(!list_is_empty(tripulantes_running)) {
		int index = list_size(tripulantes_running)-1;

		tripulante* trip_quitar = (tripulante*)list_get(tripulantes_running, index);
		int indice_trip_quitar = index;
		index--;

		while(index >= 0) {
			tripulante* nuevo_tripulante = (tripulante*)list_get(tripulantes_running, index);

			if(nuevo_tripulante->id_trip < trip_quitar->id_trip || (nuevo_tripulante->id_trip == trip_quitar->id_trip && nuevo_tripulante->id_patota < trip_quitar->id_patota)) {
				trip_quitar = nuevo_tripulante;
				indice_trip_quitar = index;
			}

			index--;
		}
		log_info(logger, "Quitando de running al trip %d", trip_quitar->id_trip);

		trip_quitar->continuar = false;
		agregar_emergencia(trip_quitar);
		sem_wait(&trip_quitar->sem_blocked);
		list_remove(tripulantes_running, indice_trip_quitar);
	}
}

void emergency_trips_ready() {
	log_info(logger, "Quitando tripulantes de ready");
	while(!queue_is_empty(cola_ready)) {
		tripulante* trip = quitar_ready();
		log_info(logger, "Quitando de ready al trip %d", trip->id_trip);
		agregar_emergencia(trip);
	}
}

void* detector_sabotaje(void* socket_mongo) {
	hay_sabotaje = false;
	int socket_sabotajes = *(int*)socket_mongo;
	while(!salir) {
		t_list* mensaje_sabotaje = recibir_mensaje(socket_sabotajes);

		if((int)list_get(mensaje_sabotaje, 0) == SABO_P) {
			hay_sabotaje = true;

			int pos_x = (int)list_get(mensaje_sabotaje, 1);
			int pos_y = (int) list_get(mensaje_sabotaje, 2);

			log_warning(logger, "Hubo un sabotaje en %d|%d", pos_x, pos_y);

			emergency_trips_running();
			emergency_trips_ready();

			//emergency_blocked -> se hace en ejecutar_io() a medida que van finalizando

			if(!list_is_empty(cola_emergencia)){
				int index = list_size(cola_emergencia)-1;
				tripulante* resolvedor = (tripulante*)list_get(cola_emergencia, index);
				int indice_resolvedor = index;
				index--;

				while(index >= 0) {
					tripulante* posible_resolvedor = (tripulante*)list_get(cola_emergencia, index);

					if(distancia_a(posible_resolvedor, pos_x, pos_y) <= distancia_a(resolvedor, pos_x, pos_y)) {
						resolvedor = posible_resolvedor;
						indice_resolvedor = index;
					}

					index--;
				}

				log_info(logger, "Resolvedor %d", resolvedor->id_trip);

				resolver_sabotaje(resolvedor, pos_x, pos_y, socket_sabotajes);

				list_remove(cola_emergencia, indice_resolvedor);
				list_add(cola_emergencia, resolvedor);

				log_info(logger, "Resolvedor mandado al final de la cola");

				while(!list_is_empty(cola_emergencia)) {
					tripulante* trip = (tripulante*)list_get(cola_emergencia, 0);
					log_info(logger, "Tripulante %d en ready", trip->id_trip);
					trip->continuar = true;
					agregar_ready(trip);
					list_remove(cola_emergencia, 0);
				}

				hay_sabotaje = false;
				sem_post(&finalizo_sabotaje);

			}else
				log_error(logger, "No hay tripulantes en la nave, no se puede resolver el sabotaje");
		}else
			log_warning(logger, "No entendi el mensaje");

		liberar_mensaje_in(mensaje_sabotaje);
	}
	return 0;
}

void resolver_sabotaje(tripulante* trip, int pos_x, int pos_y, int socket_sabotajes) {
	t_mensaje* mensaje_ini = crear_mensaje(SABO_I);
	agregar_parametro_a_mensaje(mensaje_ini, (void*)trip->id_trip, ENTERO);
	agregar_parametro_a_mensaje(mensaje_ini, (void*)trip->id_patota, ENTERO);
	enviar_y_verificar(mensaje_ini, socket_sabotajes, "Fallo al iniciar resolucion del sabotaje");

	while(trip->posicion[0] != pos_x) {
		(trip->posicion[0] < pos_x) ? trip->posicion[0]++ : trip->posicion[0]--;
		avisar_movimiento(trip);
		sleep(ciclo_CPU);
	}

	while(trip->posicion[1] != pos_y) {
		(trip->posicion[1] < pos_y) ? trip->posicion[1]++ : trip->posicion[1]--;
		avisar_movimiento(trip);
		sleep(ciclo_CPU);
	}

	t_mensaje* mensaje_fin = crear_mensaje(SABO_F);
	enviar_y_verificar(mensaje_fin, socket_sabotajes, "Fallo al finalizar resolucion del sabotaje");
}
