#ifndef NCURSES_H
#define NCURSES_H
#include <stdbool.h>
#include "instrucciones.h"

//Prototipos de funcion en ncurses.c
void imprimir_registros(int renglon, char *instruccion);
void limpia_lineas();

#endif