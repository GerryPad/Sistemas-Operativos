#ifndef NODO_H
#define NODO_H
#include <stdlib.h>
#include <string.h>
#include <curses.h>

struct Nodo{
    int PID;
    char estado[16];
    int PC;
    char archivo[64]; 
    char IR[64];
    int registros[4];
    struct Nodo *siguiente;
};

//Prototipos de funcion en ncurses.c
struct Nodo *crearCabecera();
struct Nodo *crearNodo(int n, char *archivo);
void insertarFinal(struct Nodo *cabecera, struct Nodo *nuevo);
struct Nodo *mataPID(struct Nodo *lista, int PID);
struct Nodo *desencolar(struct Nodo *lista);

#endif