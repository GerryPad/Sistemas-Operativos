#include <stdio.h>
#include "nodo.h"

struct Nodo* crearCabecera(){
    struct Nodo *cabecera = malloc(sizeof(struct Nodo));
    cabecera->siguiente = NULL;
    return cabecera;
}

struct Nodo* crearNodo(int n, int m, char *archivo){
    struct Nodo *nuevo = malloc(sizeof(struct Nodo));
    if (nuevo == NULL){
        mvprintw(30,2, "No se reservo memoria");
        return NULL;
    }
    nuevo->PID = n;
    nuevo->GID = m;
    nuevo->PC = 0; 
    for (int i=0; i<4; i++){
        nuevo->registros[i] = 0;
    }
    strcpy(nuevo->archivo, archivo);
    strcpy(nuevo->IR, "---");
    nuevo->siguiente = NULL;
    strcpy(nuevo->estado, "listos");
    nuevo->prioridad = 0; //Si deberia empezar en 20 o en 0?
    nuevo->CPU=0;
    nuevo->GCPU=0;
    return nuevo;
}

void insertarFinal(struct Nodo *cabecera, struct Nodo *nuevo){
    struct Nodo *aux = cabecera;

    while(aux->siguiente != NULL){
        aux = aux->siguiente;
    }
    aux->siguiente = nuevo;
}

struct Nodo *mataPID(struct Nodo *lista, int PID){
    struct Nodo * aux = lista->siguiente;
    struct Nodo * aux2 = lista;

    while(aux != NULL && aux->PID != PID){
        aux = aux->siguiente;
        aux2 = aux2->siguiente;
    }

    if(aux==NULL){
        return NULL;
    }

    aux2->siguiente=aux->siguiente;
    aux->siguiente = NULL;

    return aux;
}

struct Nodo * desencolar(struct Nodo *lista){
    struct Nodo *aux=lista->siguiente;
    if(aux==NULL){
        
        mvprintw(25,2,"No hay mas procesos en listos.");
        return NULL;
    }

    lista->siguiente=lista->siguiente->siguiente;
    aux->siguiente=NULL;
    return(aux);
}