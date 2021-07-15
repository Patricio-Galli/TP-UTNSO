#include "consola.h"

void dibujar_mapa(void* continuar_consola) {
    NIVEL* nivel;
    bool* continuar = (bool *)continuar_consola;
	int cols, rows;
	int err;
    int cantidad_tripulantes = 0;
    char id_actual = '0';
    t_list* tripulantes_para_actualizar;
	nivel_gui_inicializar();
	nivel_gui_get_area_nivel(&cols, &rows);
    nivel = nivel_crear("Memoria Mi-RAM-HQ");
    printf("ENTRO AL WHILE DE CONSOLA");
    while (*continuar) {
        nivel_gui_dibujar(nivel);
        while(cantidad_tripulantes < list_size(lista_tripulantes)) {
            trip_data* nuevo_trip = (trip_data *)list_get(lista_tripulantes, cantidad_tripulantes);
            err = personaje_crear(nivel, id_actual, nuevo_trip->posicion_x, nuevo_trip->posicion_y);
	        // ASSERT_CREATE(nivel, '@', err);
            nuevo_trip->modificado = true;
            id_actual++;
            cantidad_tripulantes++;
        }
        tripulantes_para_actualizar = tripulantes_modificados();
        for(int i = 0; i < list_size(tripulantes_para_actualizar); i++) {
            ITEM_NIVEL* item_auxiliar = (ITEM_NIVEL *)list_get(nivel->items, i);
            trip_data* tripulante_auxiliar = list_get(lista_tripulantes, i);
            if(tripulante_auxiliar->seguir) {
                item_mover(nivel, item_auxiliar->id, tripulante_auxiliar->posicion_x, tripulante_auxiliar->posicion_y);
                tripulante_auxiliar->modificado = false;
            }
            else {
                item_borrar(nivel, item_auxiliar->id);
                sem_post(tripulante_auxiliar->eliminar_consola);
            }
        }
    }
    nivel_destruir(nivel);
    nivel_gui_terminar();
    free(continuar_consola);
    // return EXIT_SUCCESS;
}