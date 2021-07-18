#include "consola.h"

char id_actual = '0';

void dibujar_mapa(void* continuar_consola) {
    NIVEL* nivel;
    bool* continuar = (bool *)continuar_consola;
	int cols, rows;
	int err;
    
    t_list* tripulantes_activos;
	nivel_gui_inicializar();
	nivel_gui_get_area_nivel(&cols, &rows);
    nivel = nivel_crear("Memoria Mi-RAM-HQ");
    log_info(logger, "ENTRO AL WHILE DE CONSOLA");
    t_movimiento* movimiento_pendiente;
    trip_data* tripulante_por_cambiar;
    while (*continuar) {
        log_info(logger, "Itero");
        nivel_gui_dibujar(nivel);
        log_info(logger, "nivel_gui_dibujar");
        sem_wait(&semaforo_consola);
        log_info(logger, "Pase el semaforo");
        movimiento_pendiente = list_remove(movimientos_pendientes, 0);
        
        char id = obtener_id_tripulante(nivel, movimiento_pendiente->PID, movimiento_pendiente->TID);
        if(movimiento_pendiente->seguir == false) {
            log_info(logger, "Elimino un tripulante");
            item_borrar(nivel, id);
            continue;
        }

        if(id == id_actual) {
            id_actual++;
            personaje_crear(nivel, id, movimiento_pendiente->pos_x, movimiento_pendiente->pos_y);
            continue;
        }
        log_info(logger, "Muevo el tripulante");
        item_mover(nivel, id, movimiento_pendiente->pos_x, movimiento_pendiente->pos_y);
        log_info(logger, "Termino iteración");
    }
    log_info(logger, "Paso a destruir la consola");
    nivel_destruir(nivel);
    nivel_gui_terminar();
    free(continuar_consola);
    // return EXIT_SUCCESS;
}

char obtener_id_tripulante(NIVEL* nivel, uint32_t id_patota, uint32_t id_trip) {
    bool es_mi_tripulante(void* item) {
        if(((ITEM_NIVEL *)item)->patota == id_patota && ((ITEM_NIVEL *)item)->tripulante == id_trip)
            return true;
        else
            return false;
    }

    t_list* item_correspondiente = list_filter(nivel->items, (*es_mi_tripulante));
    if(list_size(item_correspondiente) == 0) {
        list_destroy(item_correspondiente);
        return id_actual;
    }
    ITEM_NIVEL* mi_item = list_get(item_correspondiente, 0);
    list_destroy(item_correspondiente);
    return mi_item->id;
}