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

void* suma_tamanios(void* seed, void* elemento) {
    return (void *)(seed + ((t_segmento *)elemento)->tamanio);
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
    segmento_siguiente->tamanio -= nuevo_tamanio;
    
    if(segmento_siguiente->tamanio == 0) {
        list_remove(mapa_segmentos, segmento_nuevo->n_segmento + 1);
    }
    else {
        segmento_siguiente->inicio += nuevo_tamanio;
        
        t_link_element* iterador = mapa_segmentos->head;
        // Hago que el iterador apunte a la posicion del segmento_siguiente
        for(int i = 0; i < segmento_nuevo->n_segmento + 1; i++) {
            iterador = iterador->next;
        }
        // A cada segmento que le sigue incremento el nro_segmento
        while (iterador->next != NULL) {
            ((t_segmento *)iterador->data)->n_segmento++;
            iterador = iterador->next;
        }
        ((t_segmento *)iterador->data)->n_segmento++;
    }
    segmento_nuevo->tamanio = nuevo_tamanio;
    list_destroy(segmentos_libres);
    list_destroy(segmentos_validos);

    return segmento_nuevo;
}

void eliminar_segmento(t_list* mapa_segmentos, t_segmento* segmento) {
    int nro_segmento = segmento->n_segmento;
    t_segmento* segmento_anterior;
    t_segmento* segmento_siguiente;
    bool compactar_segmento_anterior = false;
    bool compactar_segmento_siguiente = false;

    segmento->duenio = 0;

    if(nro_segmento > 0) {
        segmento_anterior = list_get(mapa_segmentos, nro_segmento - 1);
        if(segmento_anterior->duenio == 0) {
            compactar_segmento_anterior = true;
        }
    }
    if (nro_segmento < mapa_segmentos->elements_count - 1) {
        segmento_siguiente = list_get(mapa_segmentos, nro_segmento + 1);
        if(segmento_siguiente->duenio == 0) {
            compactar_segmento_siguiente = true;
        }
    }
    t_link_element* iterador = mapa_segmentos->head;
    if (compactar_segmento_siguiente) {
        segmento->tamanio += segmento_siguiente->tamanio;
        list_remove(mapa_segmentos, segmento_siguiente->n_segmento);
        for(int i = 0; i < segmento->n_segmento; i++) {
            iterador = iterador->next;
        }
        iterador = iterador->next;
        while (iterador->next != NULL) {
            ((t_segmento *)iterador->data)->n_segmento--;
            iterador = iterador->next;
        }
        ((t_segmento *)iterador->data)->n_segmento--;
    }
    
    iterador = mapa_segmentos->head;
    if (compactar_segmento_anterior) {
        segmento_anterior->tamanio += segmento->tamanio;
        list_remove(mapa_segmentos, segmento->n_segmento);
        for(int i = 0; i < segmento_anterior->n_segmento; i++) {
            iterador = iterador->next;
        }
        iterador = iterador->next;
        while (iterador->next != NULL) {
            ((t_segmento *)iterador->data)->n_segmento--;
            iterador = iterador->next;
        }
        ((t_segmento *)iterador->data)->n_segmento--;
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

void segmentar_bloque(void* memoria, uint32_t posicion, void* data, uint32_t tamanio) {
    memcpy(memoria + posicion, data, tamanio);
}

void* obtener_bloque_memoria(void* memoria, t_segmento* segmento) {
    void* bloque = malloc(segmento->tamanio);
    memcpy(bloque, memoria + segmento->inicio, segmento->tamanio);
    return bloque;
}

void realizar_compactacion() {
    t_link_element* aux_segmento = mapa_segmentos->head;
    uint32_t segmentos_quitados = 0;
    uint32_t tamanio_total = 0;
    uint32_t corrimiento_inicio = 0;
    patota_data* patota_auxiliar;
    uint32_t indice_segmento;
    uint32_t cant_segmentos = mapa_segmentos->elements_count;
    void* bloque_memoria;

    t_list* tripulantes = tripulantes_activos();
    for(int i = 0; i < cantidad_tripulantes_activos(); i++) {
        trip_data* un_trip = (trip_data *)list_get(tripulantes_activos(), i);
        sem_wait(un_trip->semaforo_hilo);
    }
    for(int i = 1; i < cant_segmentos + 1; i++) {
        if(((t_segmento *)aux_segmento->data)->duenio != 0 && corrimiento_inicio != 0) {
            // Actualizo el t_segmento
            ((t_segmento *)aux_segmento->data)->n_segmento -= segmentos_quitados;
            uint32_t nuevo_inicio = ((t_segmento *)aux_segmento->data)->inicio - corrimiento_inicio;
            
            // Obtengo el bloque de memoria y lo pego en el primer lugar libre a la izquierda (siempre va a haber espacio)
            bloque_memoria = obtener_bloque_memoria(memoria_ram, (t_segmento *)aux_segmento->data);
            segmentar_bloque(memoria_ram, nuevo_inicio, bloque_memoria, ((t_segmento *)aux_segmento->data)->tamanio);
            tamanio_total += ((t_segmento *)aux_segmento->data)->tamanio;
            free(bloque_memoria);

            // Actualizo la posición del segmento en la patota_data que corresponde (duenio)
            ((t_segmento *)aux_segmento->data)->inicio = nuevo_inicio;
            patota_auxiliar = (patota_data *)list_get(lista_patotas, ((t_segmento *)aux_segmento->data)->duenio - 1);
            indice_segmento = (uint32_t)((t_segmento *)aux_segmento->data)->indice;
            patota_auxiliar->tabla_segmentos[indice_segmento] = nuevo_inicio;

            // De acuerdo al tipo de segmento que sea, realizo los cambios necesarios
            if(indice_segmento == 0) {
                /* Debo encontrar todos los tripulantes de la patota y actualizar su puntero a pcb */
                t_list* tripulantes_a_actualizar = tripulantes_de_patota(((t_segmento *)aux_segmento->data)->duenio);
                t_link_element* trip_auxiliar = tripulantes_a_actualizar->head;
                uint32_t inicio_auxiliar;
                printf("Cant trip de patota %d\n", tripulantes_a_actualizar->elements_count);
                for(int i = 0; i < tripulantes_a_actualizar->elements_count; i++) {
                    inicio_auxiliar = (uint32_t)((t_segmento *)trip_auxiliar->data)->inicio;
                    actualizar_valor_tripulante(memoria_ram + inicio_auxiliar, PCB_POINTER, nuevo_inicio);
                    trip_auxiliar = trip_auxiliar->next;
                }
                list_destroy(tripulantes_a_actualizar);
            }
            if(indice_segmento == 1) {  // El segmento que se mueve es el segmento de tareas
                actualizar_ubicacion_tareas(memoria_ram + patota_auxiliar->tabla_segmentos[0], nuevo_inicio);
            }
            
            if(indice_segmento > 1) {   // El segmento que se mueve es el segmento de un tripulante
                trip_data* aux_tripulante = tripulante_de_lista(((t_segmento *)aux_segmento->data)->duenio, indice_segmento - 1);
                aux_tripulante->inicio = nuevo_inicio;
            }
            aux_segmento = aux_segmento->next;
        }
        
        else {
            corrimiento_inicio += ((t_segmento *)aux_segmento->data)->tamanio;
            if(i != cant_segmentos) {
                free(aux_segmento->data);
                aux_segmento = aux_segmento->next;
            }
            segmentos_quitados++;
            list_remove(mapa_segmentos, i - segmentos_quitados);
            printf("Corrimiento nuevo: %d. Seg '-': %d\n", corrimiento_inicio, segmentos_quitados);
        }
    }
    t_segmento * segmento_final = malloc(sizeof(t_segmento));
    segmento_final->duenio = 0;
    segmento_final->n_segmento = mapa_segmentos->elements_count;
    
    segmento_final->tamanio = tamanio_memoria - tamanio_total;
    segmento_final->inicio = tamanio_total;
    list_add(mapa_segmentos, segmento_final);

    for(int i = 0; i < cantidad_tripulantes_activos(); i++) {
        trip_data* un_trip = (trip_data *)list_get(tripulantes_activos(), i);
        sem_post(un_trip->semaforo_hilo);
    }
    list_destroy(tripulantes);
}

uint32_t memoria_libre() {
    t_list* segmentos_libres = list_filter(mapa_segmentos, (*condicion_segmento_libre));
    uint32_t espacio_libre = 0;
    if(segmentos_libres)
        espacio_libre = (uint32_t)list_fold(segmentos_libres, 0, (*suma_tamanios));
    list_destroy(segmentos_libres);
    return espacio_libre;
}