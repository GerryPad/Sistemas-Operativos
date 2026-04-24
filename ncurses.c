#include <stdio.h>
#include <curses.h>
#include "ncurses.h"

void imprimir_registros(int renglon, char *instruccion){
    mvprintw(3, 2, "%-8s %-15s %-20s %-20s %-20s %-20s", 
        "PC", "IR", "EAX", "EBX", "ECX", "EDX");
    move(5,2);
    clrtoeol();
    mvprintw(5, 2, "%-8d %-15s %-20d %-20d %-20d %-20d", 
        renglon, 
        instruccion, 
        registros[0].valor, 
        registros[1].valor, 
        registros[2].valor, 
        registros[3].valor  
    );
    refresh();
}

void limpia_lineas() {
    move(22,2); clrtoeol();
    move(23,2); clrtoeol();
    move(24,2); clrtoeol();
    move(25,2); clrtoeol();
    move(20,2); clrtoeol();
    refresh();
}

//Imprimir primero el ejecutando, despues los listos en el orden que estan en listos y finalmente los terminados en su orden
void imprimir_listas(struct Nodo *cabecera_ejecutando, struct Nodo *cabecera_listos, struct Nodo *cabecera_terminados){
    struct Nodo *aux_e = cabecera_ejecutando->siguiente;
    struct Nodo *aux_l = cabecera_listos->siguiente;
    struct Nodo *aux_te = cabecera_terminados->siguiente;

    mvprintw(7, 2, "%-6s %-6s %-8s %-12s %-8s %-15s %-20s %-20s %-20s %-20s %-8s %-8s %-8s", 
        "PID", "GID", "File", "Estatus", "PC", "IR", "EAX", "EBX", "ECX", "EDX", "CPU", "GCPU", "Prioridad");


    for(int j = 8; j<20; j++){
        move(j,2);
        clrtoeol();
    } 
 //Esta es la lista de ejecutando
   
    if(aux_e != NULL){
        mvprintw(8, 2,"%-6d %-6d %-8s %-12s %-8s %-15s %-20s %-20s %-20s %-20s %-8d %-8d %-8d", 
        aux_e->PID,
        aux_e->GID,
        aux_e->archivo,
        aux_e->estado,
        "---",
        "---",
        "---",
        "---",
        "---",
        "---", 
        aux_e->CPU,
        aux_e->GCPU,
        aux_e->prioridad
        );
    }

    int i=9;
    while(aux_l != NULL){
        if(i>=20 || aux_l == NULL){
            break;
        }

        move(i,2);
        clrtoeol();
        mvprintw(i, 2, "%-6d %-6d %-8s %-12s %-8d %-15s %-20d %-20d %-20d %-20d %-8d %-8d %-8d", 
        aux_l->PID,
        aux_l->GID,
        aux_l->archivo,
        aux_l->estado,
        aux_l->PC,
        aux_l->IR,
        aux_l->registros[0],
        aux_l->registros[1],
        aux_l->registros[2],
        aux_l->registros[3],
        aux_l->CPU,
        aux_l->GCPU,
        aux_l->prioridad
        );

        aux_l = aux_l->siguiente;
        i++;
    }

    //Lista de terminados
    while(aux_te != NULL){
        if(i>=20 || aux_te == NULL){
            break;
        }
        move(i,2);
        clrtoeol();
        mvprintw(i, 2, "%-6d %-6d %-8s %-12s %-8d %-15s %-20d %-20d %-20d %-20d %-8d %-8d %-8d", 
        aux_te->PID,
        aux_te->GID,
        aux_te->archivo,
        aux_te->estado,
        aux_te->PC,
        aux_te->IR,
        aux_te->registros[0],
        aux_te->registros[1],
        aux_te->registros[2],
        aux_te->registros[3],
        aux_te->CPU,
        aux_te->GCPU,
        aux_te->prioridad
        );

        aux_te = aux_te->siguiente;
        i++;
    }
    refresh();
}