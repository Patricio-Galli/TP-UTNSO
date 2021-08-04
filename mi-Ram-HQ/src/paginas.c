#include "paginas.h"

uint32_t marcos_logicos_disponibles() {
    div_t cantidad_compleja = div(memoria_ram.tamanio_swap, memoria_ram.tamanio_pagina);
    uint32_t cantidad_marcos_totales = cantidad_compleja.quot;
    uint32_t cantidad_marcos_disponibles = 0;
    
    for(int i = 0; i < cantidad_marcos_totales; i++) {
        if(memoria_ram.mapa_logico[i]->duenio == 0)
            cantidad_marcos_disponibles++;
    }
    return cantidad_marcos_disponibles;
}

uint32_t marcos_reales_disponibles() {
    div_t cantidad_compleja = div(memoria_ram.tamanio_memoria, memoria_ram.tamanio_pagina);
    uint32_t cantidad_marcos_totales = cantidad_compleja.quot;
    uint32_t cantidad_marcos_disponibles = 0;
    
    for(int i = 0; i < cantidad_marcos_totales; i++) {
        if(memoria_ram.mapa_fisico[i]->duenio == 0)
            cantidad_marcos_disponibles++;
    }
    return cantidad_marcos_disponibles;
}

t_marco* obtener_marco_libre_fisico() {
    t_marco* marco_disponible;
    for(int i = 0; i < memoria_ram.tamanio_memoria / TAMANIO_PAGINA; i++) {
        // log_info(logger, "Marco ocupado por %d", memoria_ram.mapa_fisico[i]->duenio);
        if (memoria_ram.mapa_fisico[i]->duenio == 0) {
            marco_disponible = memoria_ram.mapa_fisico[i];
            break;
        }
    }
    // log_info(logger, "Obtengo marco libre fisico. Nros: %d/%d", marco_disponible->nro_virtual, marco_disponible->nro_real);
    return marco_disponible;
}

t_marco* obtener_marco_libre_virtual() {
    t_marco* marco_disponible;
    for(int i = 0; i < memoria_ram.tamanio_swap / TAMANIO_PAGINA; i++) {
        if (memoria_ram.mapa_logico[i]->duenio == 0) {
            marco_disponible = memoria_ram.mapa_logico[i];
            break;
        }
    }
    // log_info(logger, "Obtengo marco libre logico. Nros: %d/%d", marco_disponible->nro_virtual, marco_disponible->nro_real);
    return marco_disponible;
}

uint32_t frames_necesarios(uint32_t memoria_libre_ultimo_frame, uint32_t tamanio) {
    uint32_t cant_frames = (div(tamanio, memoria_ram.tamanio_pagina)).quot;
    if((div(tamanio, memoria_ram.tamanio_pagina)).rem > memoria_libre_ultimo_frame)
        cant_frames++;
    return cant_frames; 
}

uint32_t* asignar_frames_p(uint32_t id_patota, uint32_t cant_frames) {
    if(cant_frames == 0)    return NULL;
    // log_info(logger, "Asigno frames. Patota: %d. Cant: %d", id_patota, cant_frames);
    patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
    // log_info(logger, "Realloc. Tamanio void* = %d", sizeof(void*));
    if(mi_patota->cant_frames > 0)
        mi_patota->frames = realloc(mi_patota->frames, (cant_frames + mi_patota->cant_frames) * sizeof(t_marco *));
    else
        mi_patota->frames = calloc(cant_frames, sizeof(t_marco *));
    
    uint32_t* frames_asignados = malloc(cant_frames * sizeof(t_marco *));
    uint32_t nro_frames_asignados = 0;
    int frames_disponibles_memoria = cant_frames - marcos_reales_disponibles();
    if(frames_disponibles_memoria < 0)  frames_disponibles_memoria = cant_frames;

    while(cant_frames > nro_frames_asignados) {
        t_marco* nuevo_marco;
        if(frames_disponibles_memoria) {
            log_info(logger, "Obtuve marco libre fisico");
            nuevo_marco = obtener_marco_libre_fisico();
            // log_info(logger, "Dir logica: %d, dir física: %d", nuevo_marco->nro_virtual, nuevo_marco->nro_virtual);
        }
        else {
            log_info(logger, "Obtuve marco libre virtual");
            nuevo_marco = obtener_marco_libre_virtual();
            // log_info(logger, "Dir logica: %d, dir física: %d", nuevo_marco->nro_virtual, nuevo_marco->nro_virtual);
        }
        if(nuevo_marco == NULL) log_warning(logger, "Obtuve marco NULL");
        log_info(logger, "Marcos libres: reales %d/ virtuales%d", marcos_reales_disponibles(), marcos_logicos_disponibles());
        nuevo_marco->duenio = id_patota;
        frames_asignados[nro_frames_asignados] = nuevo_marco->nro_virtual;
        nro_frames_asignados++;
        mi_patota->frames[mi_patota->cant_frames] = nuevo_marco->nro_virtual;
        mi_patota->cant_frames++;
        log_info(logger, "Asigno frame. Cant frames de la patota: %d", mi_patota->cant_frames);
    }
    log_info(logger, "Asigne frames");
    return frames_asignados;    // QUITAR
}

void borrar_marco(uint32_t nro_marco) {
    t_marco* marco_descartable = memoria_ram.mapa_logico[nro_marco];
    marco_descartable->duenio = 0;
}

void* inicio_marco(uint32_t nro_marco) {
    t_marco* mi_marco = memoria_ram.mapa_logico[nro_marco];
    return memoria_ram.inicio + mi_marco->nro_real * memoria_ram.tamanio_pagina;
}

uint32_t inicio_marco_logico(uint32_t nro_marco) {
    t_marco* mi_marco = memoria_ram.mapa_logico[nro_marco];
    return mi_marco->nro_real * memoria_ram.tamanio_pagina;
}

void hacer_backup_marco(uint32_t nro_marco_fisico) {
    t_marco* mi_marco = memoria_ram.mapa_fisico[nro_marco_fisico];
    fseek(memoria_ram.inicio_swap, mi_marco->nro_virtual * TAMANIO_PAGINA, 0);
    fwrite(inicio_marco(nro_marco_fisico), TAMANIO_PAGINA, 1, memoria_ram.inicio_swap);
}

void incorporar_marco(uint32_t nro_marco) {
    log_info(logger, ":)-");
    t_marco* marco_incorporado = memoria_ram.mapa_logico[nro_marco];
    t_marco* marco_descartable;
    log_info(logger, ":)-");
    if(marco_incorporado->presencia == true) {
        log_info(logger, ":)-");
        return ;
    } // EL MARCO YA ESTA EN MEMORIA
    log_info(logger, "Incorporo marco");
    if(marcos_reales_disponibles()) {
        marco_descartable = obtener_marco_libre_fisico();
    }    // HAY MARCO LIBRE EN MEMORIA
    else {
        if(memoria_ram.algoritmo_reemplazo == LRU) {
            marco_descartable = reemplazo_por_lru(nro_marco);
        }
        if(memoria_ram.algoritmo_reemplazo == CLOCK) {
            marco_descartable = reemplazo_por_clock(nro_marco);
        }
    }   // HAY QUE HACER REEMPLAZO PARA INCORPORAR MARCO A MEMORIA
    if(marco_descartable->modificado) {
        hacer_backup_marco(marco_descartable->nro_virtual);
    }
    marco_descartable->presencia = false;
    void* memoria_vieja = malloc(TAMANIO_PAGINA);
    fseek(memoria_ram.inicio_swap, 0, TAMANIO_PAGINA * marco_incorporado->nro_virtual);
    fread(memoria_vieja, TAMANIO_PAGINA, 1, memoria_ram.inicio_swap);
    memcpy(inicio_marco(marco_descartable->nro_virtual), memoria_vieja, TAMANIO_PAGINA);
    free(memoria_vieja);
}

void actualizar_entero_paginacion(uint32_t id_patota, uint32_t desplazamiento, uint32_t valor) {
    patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
    div_t posicion_compuesta = div(desplazamiento, memoria_ram.tamanio_pagina);
    uint32_t bytes_cargados = 0;
    uint32_t bytes_disponibles = memoria_ram.tamanio_pagina - posicion_compuesta.rem;
    uint32_t pagina_actual = posicion_compuesta.quot;
    uint32_t bytes_necesarios = sizeof(uint32_t);
    log_info(logger, "actualizar_entero. Pagina %d, inicio_pagina: %d", pagina_actual, posicion_compuesta.rem);

    uint32_t data = valor;
    uint32_t inicio_pagina = posicion_compuesta.rem;
    while(bytes_cargados < bytes_necesarios) {
        t_marco* marco_auxiliar = memoria_ram.mapa_logico[mi_patota->frames[pagina_actual]];
        log_info(logger, "Bytes cargados %d, Bytes necesarios %d", bytes_cargados, bytes_necesarios);
        if(bytes_disponibles > bytes_necesarios - bytes_cargados)
            bytes_disponibles = bytes_necesarios - bytes_cargados;
        log_info(logger, "Pagina actual %d. Frames de patota %d. Tamanio vector %d", pagina_actual, mi_patota->cant_frames, sizeof(mi_patota->frames[0]));
        
        sem_wait(&marco_auxiliar->semaforo_mutex);
        incorporar_marco(mi_patota->frames[pagina_actual]);
        // log_info(logger, "Inicio de marco que actualizo %p. Data: %d. Bytes_disponibles: %d", inicio_marco(mi_patota->frames[pagina_actual]) + inicio_pagina, data, bytes_disponibles);
        log_info(logger, "Itero entero. Pagina %d, inicio_pagina: %d. Bytes a cargar: %d", pagina_actual, inicio_pagina, bytes_disponibles);
        memcpy(inicio_marco(mi_patota->frames[pagina_actual]) + inicio_pagina, &data, bytes_disponibles);
        marco_auxiliar->modificado = true;
        sem_post(&marco_auxiliar->semaforo_mutex);
        
        bytes_cargados += bytes_disponibles;
        bytes_disponibles = TAMANIO_PAGINA;
        inicio_pagina = 0;
        pagina_actual++;
    }
    // log_info(logger, "Dato nuevo %d. Obtenido por funcion %d", data, obtener_entero_paginacion(id_patota, desplazamiento));
}

uint32_t obtener_entero_paginacion(uint32_t id_patota, uint32_t desplazamiento) {
    patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
    div_t posicion_compuesta = div(desplazamiento, memoria_ram.tamanio_pagina);
    uint32_t bytes_cargados = 0;
    uint32_t bytes_disponibles = memoria_ram.tamanio_pagina - posicion_compuesta.rem;
    uint32_t pagina_actual = posicion_compuesta.quot;
    uint32_t bytes_necesarios = sizeof(uint32_t);
    // log_info(logger, "obtener_entero. Pagina %d, inicio_pagina: %d", pagina_actual, posicion_compuesta.rem);

    uint32_t data;

    uint32_t inicio_pagina = posicion_compuesta.rem;
    while(bytes_cargados < bytes_necesarios) {
        // log_info(logger, "Bytes cargados %d, Bytes necesarios %d", bytes_cargados, bytes_necesarios);
        t_marco* marco_auxiliar = memoria_ram.mapa_logico[mi_patota->frames[pagina_actual]];
        if(bytes_disponibles > bytes_necesarios - bytes_cargados)
            bytes_disponibles = bytes_necesarios - bytes_cargados;
        sem_wait(&marco_auxiliar->semaforo_mutex);
        if(memoria_ram.mapa_logico[mi_patota->frames[pagina_actual]]->presencia == false)
            incorporar_marco(mi_patota->frames[pagina_actual]);
        // log_info(logger, "Itero entero. Pagina %d, inicio_pagina: %d. Bytes a cargar: %d", pagina_actual, inicio_pagina, bytes_disponibles);
        memcpy(&data, inicio_marco(mi_patota->frames[pagina_actual]) + inicio_pagina, bytes_disponibles);
        sem_post(&marco_auxiliar->semaforo_mutex);
        
        bytes_cargados += bytes_disponibles;
        bytes_disponibles = TAMANIO_PAGINA;
        inicio_pagina = 0;
        pagina_actual++;
    }
    // log_info(logger, "Dato obtenido: %d", data);
    return data;
}

t_marco* reemplazo_por_lru(uint32_t nro_marco) {
    return NULL;
}

t_marco* reemplazo_por_clock(uint32_t nro_marco) {
    return NULL;
    // uint32_t frames_quitados[cant_a_reemplazar];
    // uint32_t marcos_encontrados = 0;
    // while(marcos_encontrados < cant_a_reemplazar) {
    //     bool marco_encontrado = false;
    //     if(memoria_ram.mapa_fisico[memoria_ram.puntero_clock]->bit_uso) {
    //         memoria_ram.mapa_fisico[memoria_ram.puntero_clock]->bit_uso = 0;
    //     }
    //     else
    //         marco_encontrado = true;
    //     for(int i = 0; i < marcos_encontrados; i++) {
    //         if(marco_encontrado && memoria_ram.mapa_fisico[memoria_ram.puntero_clock]->nro_virtual == frames_quitados[i])
    //             marco_encontrado = false;
    //     }
    //     if(marco_encontrado) {
    //         marco_a_remover = memoria_ram.mapa_fisico[memoria_ram.puntero_clock];
    //         frames_quitados[marcos_encontrados] = marco_a_remover->nro_virtual;
    //         marcos_encontrados++;
    //     }
    //     memoria_ram.puntero_clock++;
    //     if(memoria_ram.puntero_clock == memoria_ram.tamanio_memoria / memoria_ram.tamanio_pagina)
    //         memoria_ram.puntero_clock = 0;
    // }
}

/*void quitar_marcos(uint32_t cant_a_reemplazar) {
    t_marco* marco_a_remover;
    uint32_t frames_quitados[cant_a_reemplazar];
    uint32_t marcos_encontrados = 0;

    if(memoria_ram.algoritmo_reemplazo == LRU) {
        // marco_a_remover = list_get_maximum(memoria_ram.mapa_fisico, (*tiempo_maximo));
    }
    if(memoria_ram.algoritmo_reemplazo == CLOCK) {
        // uint32_t frames_quitados[cant_a_reemplazar];
        // uint32_t marcos_encontrados = 0;
        while(marcos_encontrados < cant_a_reemplazar) {
            bool marco_encontrado = false;
            if(memoria_ram.mapa_fisico[memoria_ram.puntero_clock]->bit_uso) {
                memoria_ram.mapa_fisico[memoria_ram.puntero_clock]->bit_uso = 0;
            }
            else
                marco_encontrado = true;
            for(int i = 0; i < marcos_encontrados; i++) {
                if(marco_encontrado && memoria_ram.mapa_fisico[memoria_ram.puntero_clock]->nro_virtual == frames_quitados[i])
                    marco_encontrado = false;
            }
            if(marco_encontrado) {
                marco_a_remover = memoria_ram.mapa_fisico[memoria_ram.puntero_clock];
                frames_quitados[marcos_encontrados] = marco_a_remover->nro_virtual;
                marcos_encontrados++;
            }
            memoria_ram.puntero_clock++;
            if(memoria_ram.puntero_clock == memoria_ram.tamanio_memoria / memoria_ram.tamanio_pagina)
                memoria_ram.puntero_clock = 0;
        }
    }

    if(marco_a_remover->modificado) {
        hacer_backup_marco(nro_marco_fisico);
    }
    marco_a_remover->presencia = false;
    uint32_t desplazamiento = marco_a_remover->nro_real * memoria_ram.tamanio_pagina;
    memcpy(memoria_ram.inicio + desplazamiento, 0, sizeof(memoria_ram.tamanio_pagina));
}*/

/*void asignar_frames(uint32_t id_patota, uint32_t cant_frames) {
    patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
    int frames_a_reemplazar = cant_frames - marcos_reales_disponibles();
    for(int i = 0; i < frames_a_reemplazar; i++) {
        // quitar_marcos(frames_a_reemplazar);
    }
    
    if(mi_patota->cant_frames == cant_frames) {
        mi_patota->frames = calloc(cant_frames, sizeof(t_marco *));
    }
    else {
        mi_patota->frames = realloc(mi_patota->frames, (cant_frames + mi_patota->cant_frames) * sizeof(t_marco *));
    }
    uint32_t marcos_asignados = 0;
    for(int i = 0; i < marcos_reales_disponibles() && marcos_asignados < cant_frames; i++) {
        if(memoria_ram.mapa_fisico[i]->duenio == 0) {
            marcos_asignados++;
            mi_patota->frames[mi_patota->cant_frames - 1] = memoria_ram.mapa_fisico[i]->nro_virtual;
            mi_patota->cant_frames++;
            memoria_ram.mapa_fisico[i]->presencia = true;
            memoria_ram.mapa_fisico[i]->duenio = id_patota;
            memoria_ram.mapa_fisico[i]->modificado = false;
            // memoria_ram.mapa_fisico[i]->nro_real = i;
        }
    }
}*/

// uint32_t* valor_entero;
    // char* valor_char;
    // switch(tipo) {
    //     case ENTERO:
    //         valor_entero = malloc(sizeof(uint32_t));
    //         *valor_entero = (void *)valor;
    //         bytes_necesarios = sizeof(uint32_t);
    //         data = valor_entero;
    //         break;
    //     case CARACTER:
    //         valor_char = malloc(sizeof(char));
    //         *valor_char = (void *)valor;
    //         bytes_necesarios = sizeof(char);
    //         data = valor_char;
    //         break;
    //     case BUFFER:
    //         // data = valor;
    //         bytes_necesarios = strlen(valor);
    //         data = realloc(valor, bytes_necesarios);
    //         break;
    // }