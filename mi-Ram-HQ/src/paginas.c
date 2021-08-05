#include "paginas.h"

uint32_t marcos_logicos_disponibles() {
    uint32_t cantidad_marcos_totales = (memoria_ram.tamanio_memoria + memoria_ram.tamanio_swap) / memoria_ram.tamanio_pagina;
    uint32_t cantidad_marcos_disponibles = 0;
    
    for(int i = 0; i < cantidad_marcos_totales; i++) {
        if(memoria_ram.mapa_logico[i]->duenio == 0)
            cantidad_marcos_disponibles++;
    }
    log_info(logger, "Marcos totales %d", cantidad_marcos_totales);
    log_info(logger, "Marcos totales disponibles %d", cantidad_marcos_totales);
    return cantidad_marcos_disponibles;
}

uint32_t marcos_reales_disponibles() {
    uint32_t cantidad_marcos_totales = (memoria_ram.tamanio_memoria) / memoria_ram.tamanio_pagina;
    uint32_t cantidad_marcos_disponibles = 0;
    
    for(int i = 0; i < cantidad_marcos_totales; i++) {
        if(memoria_ram.mapa_logico[i]->presencia == 0)
            cantidad_marcos_disponibles++;
    }
    return cantidad_marcos_disponibles;
}

t_marco* obtener_marco_disponible() {
    t_marco* marco_disponible;
    for(int i = 0; i < marcos_reales_disponibles(); i++) {
        if (memoria_ram.mapa_fisico[i]->duenio == 0) {
            marco_disponible = memoria_ram.mapa_fisico[i];
            break;
        }
    }
    return marco_disponible;
}

uint32_t frames_necesarios(uint32_t memoria_libre_ultimo_frame, uint32_t tamanio) {
    uint32_t cant_frames = (div(tamanio, memoria_ram.tamanio_pagina)).quot;
    if((div(tamanio, memoria_ram.tamanio_pagina)).rem > memoria_libre_ultimo_frame)
        cant_frames++;
    return cant_frames; 
}

void asignar_frames(uint32_t id_patota, uint32_t cant_frames) {
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
}

void quitar_marcos(uint32_t cant_a_reemplazar) {
    t_marco* marco_a_remover;
    uint32_t frames_quitados[cant_a_reemplazar];
    uint32_t marcos_encontrados = 0;
    /*void* tiempo_maximo(void* marco1, void* marco2) {
        if(((t_marco *)marco1)->tiempo >= ((t_marco *)marco2)->tiempo)
            return marco1;
        else
            return marco2;
    }*/

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
        // hacer_backup_marco(nro_marco_fisico);
    }
    marco_a_remover->presencia = false;
    uint32_t desplazamiento = marco_a_remover->nro_real * memoria_ram.tamanio_pagina;
    memcpy(memoria_ram.inicio + desplazamiento, 0, sizeof(memoria_ram.tamanio_pagina));
}

void* inicio_marco(uint32_t nro_marco) {
    t_marco* mi_marco = memoria_ram.mapa_logico[nro_marco];
    return memoria_ram.inicio + mi_marco->nro_real * memoria_ram.tamanio_pagina;
}

void hacer_backup_marco(uint32_t nro_marco_fisico) {
    t_marco* mi_marco = memoria_ram.mapa_fisico[nro_marco_fisico];
    fseek(memoria_ram.inicio_swap, mi_marco->nro_virtual * TAMANIO_PAGINA, 0);
    fwrite(inicio_marco(nro_marco_fisico), TAMANIO_PAGINA, 1, memoria_ram.inicio_swap);
}