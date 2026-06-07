#ifndef NCURSES_H
#define NCURSES_H
#include <stdbool.h>
#include "instrucciones.h"
#include "nodo.h"

//Prototipos de funcion en ncurses.c
void imprimir_registros(int renglon, char *instruccion);
void limpia_lineas();
void imprimir_listas(struct Nodo *cabecera_ejecutando, struct Nodo *cabecera_listos, struct Nodo *cabecera_terminados, struct Nodo *cabecera_suspendidos, struct Nodo *cabecera_nuevos);
void imprimirTms(TablaMarcos *tms);
void imprimirTmp(struct Nodo *proceso);
void imprimirTmm(TablaMarcos *tmm);
#endif