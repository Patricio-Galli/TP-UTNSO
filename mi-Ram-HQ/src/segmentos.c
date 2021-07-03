#include "segmentos.h"

uint32_t tamanio_segmento;

bool condicion_segmento_libre(void* segmento_memoria) {
    if(((t_segmento*)segmento_memoria)->duenio == 0)
        return true;
    else
        return false;
}

bool condicion_segmento_apto(void* segmento_memoria) {
	if(((t_segmento*)segmento_memoria)->tamanio >= tamanio_segmento)
		return true;
	else
		return false;
}

void* minimo_tamanio(void* elemento1, void* elemento2) {
    if ( ((t_segmento *)elemento1)->tamanio < ((t_segmento *)elemento2)->tamanio ) {
        return elemento1;
    }
    else
        return elemento2;
}

t_segmento* crear_segmento(t_list* mapa_segmentos, uint32_t nuevo_tamanio, algoritmo_segmento algoritmo) {
	t_segmento *segmento_nuevo = malloc(sizeof(t_segmento));
    tamanio_segmento = nuevo_tamanio;

	t_list* segmentos_libres = list_filter(mapa_segmentos, (*condicion_segmento_libre));
	t_list* segmentos_validos = list_filter(segmentos_libres, (*condicion_segmento_apto));

	if(segmentos_validos->elements_count == 0) {
        return (void *)0;
    }

    if(algoritmo == FF) {
        memcpy(segmento_nuevo, (t_segmento *)list_get(segmentos_validos, 0), sizeof(t_segmento));
    }
    if(algoritmo == BF) {
        memcpy(segmento_nuevo, (t_segmento *)list_get_minimum(mapa_segmentos, (void *)(*minimo_tamanio)), sizeof(t_segmento));
        // No testeado
    }
    list_add_in_index(mapa_segmentos, segmento_nuevo->n_segmento, segmento_nuevo);
    
    t_segmento* segmento_siguiente = (t_segmento *)list_get(mapa_segmentos, segmento_nuevo->n_segmento + 1);
    segmento_siguiente->n_segmento++;
    segmento_siguiente->inicio += nuevo_tamanio;
    segmento_siguiente->tamanio -= nuevo_tamanio;
    segmento_nuevo->tamanio = nuevo_tamanio;
    return segmento_nuevo;
}

void eliminar_segmento(t_list* mapa_segmentos, t_segmento* segmento) {
    int nro_segmento = segmento->n_segmento;
    t_segmento* segmento_anterior;
    t_segmento* segmento_siguiente;
    bool compactar_segmento_anterior = false;
    bool compactar_segmento_siguiente = false;
    int correr_nro_seg = 0;

    if(nro_segmento > 0) {
        segmento_anterior = list_get(mapa_segmentos, nro_segmento - 1);
        if(segmento_anterior->duenio == 0) {
            compactar_segmento_anterior = true;
        }
    }
    if (nro_segmento < mapa_segmentos->elements_count - 1) {
        segmento_siguiente = list_get(mapa_segmentos, nro_segmento + 1);
        compactar_segmento_siguiente = true;
    }
    
    if (compactar_segmento_siguiente) {
        segmento->tamanio += segmento_siguiente->tamanio;
        segmento->duenio = 0;
        list_remove(mapa_segmentos, segmento_siguiente->n_segmento);
        correr_nro_seg++;
    }
    
    if (compactar_segmento_anterior) {
        segmento_anterior->tamanio += segmento->tamanio;
        list_remove(mapa_segmentos, segmento->n_segmento);
        correr_nro_seg++;
    }
    
}

void segmentar_caracter(void* memoria, uint32_t posicion, char data) {
    char valor = data;
    memcpy(memoria + posicion, &valor, sizeof(char));
}

void segmentar_entero(void* memoria, uint32_t posicion, uint32_t data) {
    uint32_t valor = data;
    memcpy(memoria + posicion, &valor, sizeof(uint32_t));
}

void segmentar_string(void* memoria, uint32_t posicion, char* data) {
    memcpy(memoria + posicion, data, strlen(data) + 1);
}