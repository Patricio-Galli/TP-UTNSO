#ifndef _PAGINAS_H_
#define _PAGINAS_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "memoria_ram.h"

uint32_t marcos_logicos_disponibles();
uint32_t marcos_reales_disponibles();
uint32_t frames_necesarios(uint32_t memoria_libre_ultimo_frame, uint32_t tamanio);

void asignar_frames(uint32_t id_patota, uint32_t cant_frames);

t_marco* obtener_marco_disponible();
uint32_t obtener_reemplazo();

void segmentar_entero_p(uint32_t id_patota, uint32_t data);
void* inicio_marco(uint32_t);

void quitar_marcos(uint32_t cant_a_reemplazar);

#endif /* _PAGINAS_H_ */