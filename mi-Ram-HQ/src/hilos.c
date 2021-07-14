#include "hilos.h"

void* rutina_hilos(void* data) {
	// t_log* logger = log_create("miramhq.log", "HILOX", 1, LOG_LEVEL_INFO);
	// log_info(logger, "HOLA MUNDO, SOY UN HILO %d", variable);
    trip_data* tripulante = (trip_data *)data;
	patota_data* segmento_patota = (patota_data *)list_get(lista_patotas, tripulante->PID - 1);
	tareas_data* segmento_tareas = (tareas_data *)list_get(lista_tareas, tripulante->PID - 1);
	uint32_t ip;
	char* tarea_nueva;

	int socket_cliente = esperar_cliente(tripulante->socket);
    bool cliente_conectado = true;
    if(socket_cliente < 0)
        cliente_conectado = false;
	
    int posicion_x;
	int posicion_y;
    
	t_mensaje* mensaje_out;
	t_list* mensaje_in;

	while(tripulante->seguir && cliente_conectado) {
		mensaje_in = recibir_mensaje(socket_cliente);
        sem_wait(tripulante->semaforo_hilo);
		// if(!validar_mensaje(mensaje_in, logger)) {
			// printf("FALLO EN MENSAJE CON HILO RAM\n");
		// }
		// else
			// printf("EL HILO DISCORD ME DIJO: %d\n", (int)list_get(mensaje_in, 0));
		switch ((int)list_get(mensaje_in, 0)) {
		case NEXT_T:
			ip = obtener_valor_tripulante(memoria_ram + segmento_patota->tabla_segmentos[tripulante->TID + 1], INS_POINTER);
			if(ip + 1 > segmento_tareas->cant_tareas) {
				// IP fuera de rango
				mensaje_out = crear_mensaje(ER_MSJ);
			}
			else {
				// IP valido
				tarea_nueva = obtener_tarea(memoria_ram + segmento_patota->tabla_segmentos[1], segmento_tareas, ip);
				actualizar_valor_tripulante(memoria_ram + segmento_patota->tabla_segmentos[tripulante->TID + 1], INS_POINTER, ip + 1);
				mensaje_out = crear_mensaje(TASK_T);
				agregar_parametro_a_mensaje(mensaje_out, tarea_nueva, BUFFER);
			}
			break;
		case ACTU_P:
			tripulante->posicion_x = (uint32_t)list_get(mensaje_in, 1);
			tripulante->posicion_y = (uint32_t)list_get(mensaje_in, 2);
			tripulante->modificado = true;

			actualizar_valor_tripulante(memoria_ram + segmento_patota->tabla_segmentos[tripulante->TID + 1], POS_X, tripulante->posicion_x);
			actualizar_valor_tripulante(memoria_ram + segmento_patota->tabla_segmentos[tripulante->TID + 1], POS_Y, tripulante->posicion_y);

			// log_info(logger, "Tripulante %d posicion actualizada %d|%d", id_trip, posicion_x, posicion_y);

			mensaje_out = crear_mensaje(TODOOK);
            enviar_mensaje(socket_cliente, mensaje_out);
		    liberar_mensaje_out(mensaje_out);
			break;
		case ELIM_T:
			break;
		case SHOW_T:
			break;
        case ER_SOC:
        case ER_RCV:
            cliente_conectado = false;
            break;
		}
        sem_post(tripulante->semaforo_hilo);
        liberar_mensaje_in(mensaje_in);
	}

    // close(socket_cliente);
    // close(tripulante->socket);
    // sem_destroy(tripulante->semaforo_hilo);
	return 0;
}