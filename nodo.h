#ifndef NODO_H
#define NODO_H
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <time.h>

typedef struct {
    int num_marco;
    int propietario; // 0 = libre, != 0 es el PID asignado
    int num_pagina;
} TablaMarcos; 

typedef struct {
    int num_marco_ram;
    int num_marco_disco;
    int num_pagina;
} TMP;

struct Nodo{
    int PID;
    int GID;
    //char estado[16];
    int estadoTermino;
    int PC;
    char archivo[64]; 
    char IR[64];
    int registros[4];
    int CPU; 
    int GCPU;
    int prioridad;
    struct Nodo *siguiente;
    TMP *tmp; //Ver si esto no genera conflicto
    int num_paginas;
    time_t hora_entrada;
    int tiempo_espera;
};

//Prototipos de funcion en ncurses.c
struct Nodo *crearCabecera();
struct Nodo *crearNodo(int n, int m, char *archivo, int n_pagina);
void insertarFinal(struct Nodo *cabecera, struct Nodo *nuevo);
struct Nodo *extraerPID(struct Nodo *lista, int PID);
struct Nodo *desencolar(struct Nodo *lista);
struct Nodo *buscaPID(struct Nodo *lista, int pid);
struct Nodo *buscaGID(struct Nodo *lista, int gid);
void actualizaTMP(struct Nodo *act, TablaMarcos *tms);

#endif 