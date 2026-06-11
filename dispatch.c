#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "dispatch.h"
#include "swap.h"

void guardaPCB(struct Nodo *PCB, int pc, char *linea){
    PCB->PC = pc; //Sera que tenemos que guardar la siguiente instruccion o donde se quedo?
    strcpy(PCB->IR,linea);
    PCB->registros[0]=registros[0].valor;
    PCB->registros[1]=registros[1].valor;
    PCB->registros[2]=registros[2].valor;
    PCB->registros[3]=registros[3].valor;
}

//Esto debe devolver un entero para el pc
int restauraPCB(struct Nodo *proceso_actual, char *archivo){
    //pc = proceso_actual->PC
    registros[0].valor = proceso_actual->registros[0];
    registros[1].valor = proceso_actual->registros[1];
    registros[2].valor = proceso_actual->registros[2];
    registros[3].valor = proceso_actual->registros[3];
    strcpy(archivo, proceso_actual->archivo); 
    return proceso_actual->PC; //Se ocupaba hacer esto porque si no, jamas devolvia el valor
}
//---------------------------------------------------------------------------------------------------------------------------------
void aumentaGCPU(struct Nodo *listos, int gid){
    struct Nodo *aux = listos->siguiente;

    while (aux != NULL) {
        if(aux->GID== gid){
            aux->GCPU= aux->GCPU+20;
        }
        aux = aux->siguiente;
    }
    

}

int contarGrupos(struct Nodo *listos, struct Nodo *ejecutando, int max_gid) {
    int contador = max_gid;  //Asumir que todos los grupos estan activos

    for (int i = 1; i <= max_gid; i++) {
        if (buscaGID(ejecutando, i) != NULL) {
            continue; //Si esta en ejecutando ya no lo busques en listos
        } else {
            if (buscaGID(listos, i) == NULL) {
                //Si tampoco esta en listos, el grupo no esta activo
                contador--;
            }
        }
    }

    //mvprintw(30, 0, "Grupos: %-4d", contador);
    return contador;
}

void actualizaCGPU(struct Nodo *aux){
    
    while(aux != NULL) {
        aux->CPU = aux->CPU/2;
        aux->GCPU = aux->GCPU/2;
        aux = aux->siguiente;
    }
}

void calculoPrioridades(struct Nodo *listos, struct Nodo *suspendidos, int grupos) {
    struct Nodo *aux = listos->siguiente;
    struct Nodo *aux2 = listos->siguiente;
    struct Nodo *aux3 = suspendidos->siguiente;
    int p_base = 60; //Cambiamos la prioridad base a 60 
    //float wk=1.0/grupos;

    actualizaCGPU(aux2);
    actualizaCGPU(aux3);
    
    while(aux!= NULL){
        //aux->CPU = aux->CPU/2;
        //aux->GCPU = aux->GCPU/2;
        aux->prioridad = p_base + (int) ((aux->CPU/2.0)) + (int) ((aux->GCPU * grupos/4.0));
        aux = aux->siguiente;
    }
}

struct Nodo *planificador(struct Nodo *listos, struct Nodo *ejecutando) {
    if (listos->siguiente == NULL){
        return NULL; //Caundo no hay nada en listos
    } 

    struct Nodo *aux = listos->siguiente; //Para ir recorriendo la lista de listos
    struct Nodo *proceso_prioritario = aux; //De primeras, suponemos que el primer nodo es el que tiene mayor prioridad (numero menor)
    while(aux != NULL){ //Recorremos hasta el final de la lista
        if(aux->prioridad < proceso_prioritario->prioridad){ //Preguntamos si el proceso actual en listos tiene prioridad mayor
            proceso_prioritario = aux; //En caso de que si, el nuevo proceso prioritario es el de actual de listos
        }
        aux=aux->siguiente;
    }

    struct Nodo *proceso = extraerPID(listos, proceso_prioritario->PID); //"desencolamos" el proceso de mayor prioridad
    //strcpy(proceso_prioritario->estado, "ejecutando");
    proceso_prioritario->estadoTermino = 0;
    insertarFinal(ejecutando, proceso);
    return proceso;
}

void sacarSuspendidos(struct Nodo *suspendidos, struct Nodo *listos){
    struct Nodo *aux_s = suspendidos->siguiente;
    struct Nodo *proceso_a_mover = NULL;

    while(aux_s != NULL){
        if(difftime(time(NULL), aux_s->hora_entrada) >= aux_s->tiempo_espera) {
            proceso_a_mover = extraerPID(suspendidos, aux_s->PID);
            insertarFinal(listos, proceso_a_mover);
        }
        aux_s = aux_s->siguiente;
    }
}

void sacarNuevos(struct Nodo *nuevos, struct Nodo *listos, TablaMarcos *tms, FILE *bin){
    struct Nodo *aux_n=nuevos->siguiente;
    struct Nodo *proceso_a_mover=NULL;
    int libre=0;

    for(int i=0; i<TOTAL_MARCOS_DISCO; i++){
        if(tms[i].propietario == 0){
            libre++;
        }
    }
    while(aux_n !=NULL){
        if(aux_n->num_paginas<=libre){
            proceso_a_mover=extraerPID(nuevos,aux_n->PID);
            insertarFinal(listos,proceso_a_mover);
            guardarTextoABinario(proceso_a_mover->archivo, bin, proceso_a_mover->PID, proceso_a_mover->GID, tms);
        }
        aux_n=aux_n->siguiente;
    }

}