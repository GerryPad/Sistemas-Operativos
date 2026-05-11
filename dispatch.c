#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dispatch.h"

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

void calculoPrioridades(struct Nodo *listos, int grupos) {
    struct Nodo *aux = listos->siguiente;
    int p_base = 20;
    //float wk=1.0/grupos;

    while(aux!= NULL){
        aux->CPU = aux->CPU/2;
        aux->GCPU = aux->GCPU/2;
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

    struct Nodo *proceso = mataPID(listos, proceso_prioritario->PID); //"desencolamos" el proceso de mayor prioridad
    strcpy(proceso_prioritario->estado, "ejecutando");
    insertarFinal(ejecutando, proceso);
    return proceso;
}

