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

void eliminar_segmento(t_list* mapa_segmentos, int segmento) {
    
}

void segmentar(void* memoria, t_segmento* segmento, t_list* parametros_seg) {
    printf("Entro a segmentar();\n");
    int desplazamiento = 0;
    void* inicio_segmento = memoria + segmento->inicio;
    int tamanio_elemento;
    int veces = 0;
    t_link_element* nodo_parametro = parametros_seg->head;
    segmento->duenio = (uint32_t)((nodo_seg *)(nodo_parametro->data))->data;
    printf("duenio = %d\n", segmento->duenio);
    nodo_parametro = nodo_parametro->next;
    printf("entro al while. Bucle de %d\n", parametros_seg->elements_count);
    if((nodo_parametro == NULL))
        printf("DOY NULL\n");
    do {
        veces++;
        tamanio_elemento = ((nodo_seg *)(nodo_parametro->data))->tamanio;
        void* data = ((nodo_seg *)(nodo_parametro->data))->data;
        printf("segmento, while\ntamanio_elem = %d\ndata = %d\n", tamanio_elemento, ((nodo_seg *)(nodo_parametro->data))->data);
        memcpy(inicio_segmento + desplazamiento, &data, tamanio_elemento);
        printf("sobrevivo al memcpy\n");
        desplazamiento += tamanio_elemento;
        /*if (((nodo_seg *)nodo_parametro->data)->tipo == T_CADENA) {
            free(((nodo_seg *)(nodo_parametro->data))->data);   // creo que rompe si no es un string
            printf("Entro a cadena\n");
        }*/
        free(nodo_parametro->data);
        printf("Libero datas %d\n", veces);
        nodo_parametro = nodo_parametro->next;
    }
    while (nodo_parametro->next != NULL);
    
    printf("salgo del while\n");
}

void agregar_parametro_a_segmento(t_list* parametros, void* data, tipo_dato tipo) {
    /*nodo_seg* nuevo_parametro = malloc(sizeof(nodo_seg));
    switch (tipo) {
    case T_ENTERO:
        nuevo_parametro->tamanio = sizeof(uint32_t);
        break;
    case T_CARACTER:
        nuevo_parametro->tamanio = sizeof(char);
        break;
    case T_CADENA:
        nuevo_parametro->tamanio = strlen((char *)data) + 1;
        break;
    default:
        break;
    }
    nuevo_parametro->data = data;
    nuevo_parametro->tipo = tipo;
    list_add(parametros, nuevo_parametro);*/
}

void nuevo_segmentar(void* memoria, uint32_t posicion, void* data, tipo_dato tipo, uint32_t tamanio) {
    /*int valor_int;
    char valor_char;
    switch (tipo) {
    case SEG_INT:
        valor_int = data;
        memcpy(memoria + posicion, &valor_int, tamanio);
        break;
    case SEG_CHAR:
        valor_char = data;
        memcpy(memoria + posicion, &valor_char, tamanio);
        break;
    case SEG_STRING:
        memcpy(memoria + posicion, data, tamanio);
        break;
    }*/
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

void segmentar_tcb(void* memoria, t_segmento* segmento, t_list* parametros) {
    uint32_t desplazamiento = 0;
    for(int i = 0; i < 6; i++) {
        if(i != 1) {
            nuevo_segmentar(memoria, segmento->inicio + desplazamiento, list_get(parametros, 1), SEG_CHAR, sizeof(char));
	        desplazamiento += sizeof(char);
        }
        else {
            nuevo_segmentar(memoria, segmento->inicio + desplazamiento, list_get(parametros, i), SEG_INT, sizeof(uint32_t));
	        desplazamiento += sizeof(uint32_t);
        }
    }
}

void segmentar_pcb(void* memoria, t_segmento* segmento, t_list* parametros) {
    nuevo_segmentar(memoria, segmento->inicio, list_get(parametros, 0), SEG_INT, sizeof(uint32_t));
    nuevo_segmentar(memoria, segmento->inicio + sizeof(uint32_t), list_get(parametros, 1), SEG_INT, sizeof(uint32_t));
}

void segmentar_tareas(void* memoria, t_segmento* segmento, t_list* parametros, int* vector_tareas_tam) {
    uint32_t desplazamiento = 0;
    char* tarea;
    for(int i = 0; i < 6; i++) {
        tarea = list_get(parametros, i);
        nuevo_segmentar(memoria, segmento->inicio + desplazamiento, tarea, SEG_STRING, sizeof(uint32_t));
        desplazamiento += vector_tareas_tam[i];
        free(tarea);
    }
}