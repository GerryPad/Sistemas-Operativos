#include <stdio.h>
#include "nodo.h"

#define TOTAL_MARCOS_DISCO 32768

struct Nodo* crearCabecera(){
    struct Nodo *cabecera = malloc(sizeof(struct Nodo));
    cabecera->siguiente = NULL;
    return cabecera;
}

struct Nodo* crearNodo(int n, int m, char *archivo, int n_paginas){
    struct Nodo *nuevo = malloc(sizeof(struct Nodo));
    if (nuevo == NULL){
        mvprintw(39,2, "No se reservo memoria");
        return NULL;
    }
    nuevo->num_paginas = n_paginas;

    if (n_paginas > 0) {
        nuevo->tmp = (TMP *)malloc(n_paginas * sizeof(TMP));
        if (nuevo->tmp == NULL) {
            mvprintw(39,2, "No se reservo memoria para la TMP");
            //free(nuevo); // Limpiamos el nodo si falló la TMP
            return NULL;
        }
        
        //Inicializamos las páginas con valores por defecto (ej. -1 significa no asignado)
        for(int i = 0; i < n_paginas; i++) {
            nuevo->tmp[i].num_pagina = i;
            nuevo->tmp[i].num_marco_ram = -1; 
            nuevo->tmp[i].num_marco_disco = -1;
        }
    } else {
        nuevo->tmp = NULL; // El proceso no requiere páginas inicialmente
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
    //strcpy(nuevo->estado, "listos");
    nuevo->estadoTermino = 0;
    nuevo->CPU = 0;
    nuevo->GCPU = 0;
    nuevo->prioridad = 0; //Si deberia empezar en 20 o en 0?
    nuevo->hora_entrada = 0;
    nuevo->tiempo_espera = 0;
    //nuevo -> tmp = NULL;
    return nuevo;
}

void actualizaTMP(struct Nodo *act, TablaMarcos *tms) {
    int pid = act->PID;

    for(int i = 0; i < TOTAL_MARCOS_DISCO; i++) {
        if(tms[i].propietario == pid) {
            int pag = tms[i].num_pagina;
            
            //Validación de seguridad para no segmentar memoria
            if (pag >= 0 && pag < act->num_paginas) {
                act->tmp[pag].num_marco_disco = i;
            }
        }
    }
}

void insertarFinal(struct Nodo *cabecera, struct Nodo *nuevo){
    struct Nodo *aux = cabecera;

    while(aux->siguiente != NULL){
        aux = aux->siguiente;
    }
    aux->siguiente = nuevo;
}

struct Nodo *extraerPID(struct Nodo *lista, int PID){
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
        
        mvprintw(37,2,"No hay mas procesos en listos.");
        return NULL;
    }

    lista->siguiente=lista->siguiente->siguiente;
    aux->siguiente=NULL;
    return(aux);
} 

struct Nodo *buscaPID(struct Nodo *lista, int pid){ //mover a nodo.c
    struct Nodo * aux = lista->siguiente;

    while(aux != NULL && aux->PID != pid){
        aux = aux->siguiente;
    
    }

    if(aux==NULL){
        return NULL;
    }
    return aux;
}

struct Nodo *buscaGID(struct Nodo *lista, int gid) { //mover a nodo.c
    struct Nodo *aux = lista->siguiente;

    while (aux != NULL && aux->GID != gid) {
        aux = aux->siguiente;
    }

    if(aux==NULL){
        return NULL;
    }

    return aux;
}