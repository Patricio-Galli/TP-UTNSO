#include "bitacora.h"

//se puede hacer en un switch
void actualizar_posicion(tripulante* tripulante, int x_nueva, int y_nueva, char* DIR_Bit_Tripulante){
	int x_anterior = tripulante->posicion_x;
	int y_anterior = tripulante->posicion_y;
	tripulante->posicion_x = x_nueva;
	tripulante->posicion_y = y_nueva;

	char* mensaje = string_new();
	char* posiciones_viejas = juntar_posiciones(x_anterior, y_anterior);
	char* posiciones_nuevas = juntar_posiciones(x_nueva, y_nueva);

	string_append(&mensaje,"Se mueve de ");
	string_append(&mensaje,posiciones_viejas);
	string_append(&mensaje," a ");
	string_append(&mensaje,posiciones_nuevas);
	string_append(&mensaje,".");
	escribir_mensaje_en_bitacora(mensaje,DIR_Bit_Tripulante);

	free(mensaje);
	free(posiciones_viejas);
	free(posiciones_nuevas);
}
void comienza_tarea(char* tarea, char* DIR_Bit_Tripulante){

	char* mensaje = string_new();
	string_append(&mensaje, "Comienza ejecucion de tarea ");
	string_append(&mensaje, tarea);
	string_append(&mensaje, ".");
	escribir_mensaje_en_bitacora(mensaje, DIR_Bit_Tripulante);
	free(mensaje);
}
void finaliza_tarea(char* tarea, char* DIR_Bit_Tripulante){
	char* mensaje = string_new();
	string_append(&mensaje,"Finaliza la tarea ");
	string_append(&mensaje,tarea);
	string_append(&mensaje,".");
	escribir_mensaje_en_bitacora(mensaje,DIR_Bit_Tripulante);
	free(mensaje);
}

void inicio_sabotaje(char* DIR_Bit_Tripulante){
	char* mensaje = string_new();
	string_append(&mensaje, "Se corre en panico hacia la ubicacion del sabotaje.");
	escribir_mensaje_en_bitacora(mensaje, DIR_Bit_Tripulante);
	free(mensaje);
}

void fin_sabotaje(char* DIR_Bit_Tripulante){
	char* mensaje = string_new();
	string_append(&mensaje, "Se resuelve el sabotaje.");
	escribir_mensaje_en_bitacora(mensaje, DIR_Bit_Tripulante);
	free(mensaje);
}

int escribir_caracter_en_bitacora(char* mensaje,int ultima_pos_mensaje,char* bloque_libre,int size){
	int bloqueALlenar = atoi(bloque_libre);
	int cantidad_caracteres = string_length(mensaje)-ultima_pos_mensaje; //si son 20 carac el msg,y escribi 3carac, estoy en la posicion 2, y me faltan 17 caracteres
	int i = 0;

	for(int caracteresEnBloque = size%block_size; caracteresEnBloque < block_size && cantidad_caracteres > 0; caracteresEnBloque++){
		int desplazamiento = bloqueALlenar * block_size + caracteresEnBloque;

		char* agregar_blocks = string_from_format("%c",mensaje[ultima_pos_mensaje]);

		pthread_mutex_lock(&actualizar_blocks);
			memcpy(blocks_copy+desplazamiento, agregar_blocks, sizeof(char));
		pthread_mutex_unlock(&actualizar_blocks);

		free(agregar_blocks);

		ultima_pos_mensaje++;
		cantidad_caracteres--;
		i++;
	}

	if(cantidad_caracteres==0)
		log_info(logger, "Se completo la escritura de caracteres");

	return ultima_pos_mensaje;
}

void escribir_mensaje_en_bitacora(char* mensaje, char* DIR_Bit_Tripulante) {
	int cantidad_caracteres = string_length(mensaje);

	t_config* metadata = config_create(DIR_Bit_Tripulante);
	char** bloques_anteriores = config_get_array_value(metadata,"BLOCKS");
	int size_original = config_get_int_value(metadata,"SIZE");
	int size = size_original;
	int cantidad_original_bloques = roundUp(size,block_size);
	int ultima_pos_mensaje = 0;

	if(size % block_size != 0) {
		log_info(logger, "Se encontro lugar en el ultimo bloque");
		ultima_pos_mensaje = escribir_caracter_en_bitacora(mensaje, ultima_pos_mensaje, bloques_anteriores[cantidad_original_bloques-1], size);
	}

	int cant_anterior_bloques = roundUp(size,block_size);
	size += cantidad_caracteres; //sumo todos los bytes

	int bloques_max = roundUp(size,block_size);
	int bloques_faltantes = bloques_max - cant_anterior_bloques;

	if(bloques_faltantes > 0) {//si ocupo mas bloques de los que tengo asignados, debo agregar mas bloques
		char* bloques_a_agregar = string_new();

		pthread_mutex_lock(&actualizar_bitmap);
			char* bloque_libre = proximo_bloque_libre();
		pthread_mutex_unlock(&actualizar_bitmap);

		string_append(&bloques_a_agregar,bloque_libre);//agrego el proximo bloque disponible que va a almacenar los caracteres
		ultima_pos_mensaje = escribir_caracter_en_bitacora(mensaje, ultima_pos_mensaje, bloque_libre, size_original + ultima_pos_mensaje);

		if(bloque_libre != NULL)
			free(bloque_libre);

		bloques_faltantes--;

		for(int i = 0; i < bloques_faltantes; i++) { //si siguen faltando agregar bloques los agrego
			pthread_mutex_lock(&actualizar_bitmap);
				bloque_libre = proximo_bloque_libre();
			pthread_mutex_unlock(&actualizar_bitmap);

			string_append(&bloques_a_agregar, ",");
			string_append(&bloques_a_agregar, bloque_libre);
			ultima_pos_mensaje=escribir_caracter_en_bitacora(mensaje, ultima_pos_mensaje,bloque_libre,size_original+ultima_pos_mensaje);

			if(bloque_libre != NULL)
				free(bloque_libre);

		}
		string_append(&bloques_a_agregar,"]");

		char* bloques_totales = string_new();
		string_append(&bloques_totales,"[");

		if(cant_anterior_bloques > 0) {
			string_append(&bloques_totales,bloques_anteriores[0]);

			for(int i = 1; i < cant_anterior_bloques; i++){
				string_append(&bloques_totales,",");
				string_append(&bloques_totales, bloques_anteriores[i]);
			}

			string_append(&bloques_totales,",");
		}

		char* tamanio_str = string_itoa(size);
		config_save(metadata);
		config_set_value(metadata,"SIZE",tamanio_str);
		free(tamanio_str);

		liberar_split(bloques_anteriores);
		string_append(&bloques_totales, bloques_a_agregar);

		free(bloques_a_agregar);
		config_set_value(metadata,"BLOCKS",bloques_totales);

		free(bloques_totales);

		config_save(metadata);
		config_destroy(metadata);
	}
}
