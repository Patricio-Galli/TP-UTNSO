#include "consola.h"

void dibujar_mapa(void* continuar_consola) {
    NIVEL* nivel;
    bool* continuar = (bool *)continuar_consola;
	int cols, rows;
	int err;
    int cantidad_tripulantes = 0;
    char id_actual = '0';
	nivel_gui_inicializar();
	nivel_gui_get_area_nivel(&cols, &rows);
    nivel = nivel_crear("Memoria Mi-RAM-HQ");

    while (*continuar) {
        nivel_gui_dibujar(nivel);
        while(cantidad_tripulantes < cantidad_tripulantes_activos()) {
            trip_data* nuevo_trip = (trip_data *)list_get(lista_tripulantes, cantidad_tripulantes);
            err = personaje_crear(nivel, id_actual, nuevo_trip->posicion_x, nuevo_trip->posicion_y);
	        // ASSERT_CREATE(nivel, '@', err);
            id_actual++;
            cantidad_tripulantes++;
        }
        for(int i = 0; i < cantidad_tripulantes; i++) {
            ITEM_NIVEL* item_auxiliar = (ITEM_NIVEL *)list_get(nivel->items, i);
            trip_data* tripulante_auxiliar = list_get(lista_tripulantes, i);
            item_mover(nivel, item_auxiliar->id, tripulante_auxiliar->posicion_x, tripulante_auxiliar->posicion_y);
        }
    }
    nivel_destruir(nivel);
    nivel_gui_terminar();
    // return EXIT_SUCCESS;
}