#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include "nodo.h"
#include "ram.h"

//Carga una pagina especifica en una posicion de la RAM
//Actualiza los valores de la TMP
//Notifica a las de mas TMP's del mismo grupo
int cargarARAM(int pid, int gid, int num_pagina, FILE *bin, struct TablaMarcos *manecilla,
     struct Nodo *listos, struct Nodo *ejecutando, struct Nodo *suspendidos, TablaMarcos *tms, TablaMarcos *tmm, char *RAM) {
    int marco_disco = -1;
    int contador_paginas = 0;

    struct Nodo *aux_l = listos->siguiente;
    struct Nodo *aux_e = ejecutando->siguiente;
    struct Nodo *aux_s = suspendidos->siguiente;

    //Buscar en la TMS la ubicacion fisica de la pagina
    for (int i = 0; i < TOTAL_MARCOS_DISCO; i++) {
        if (tms[i].grupo == gid) {
            if (contador_paginas == num_pagina) {
                marco_disco = i;
                break;
            }
            contador_paginas++;
        }
    }

    //Buscar un marco libre en la TMM 
    for (int marco_ram = 0; marco_ram < TOTAL_MARCOS_RAM; marco_ram++) {
        if (tmm[marco_ram].num_marco == manecilla->num_marco) {
            
            //Copiar datos del disco a la RAM
            fseek(bin, marco_disco * TAMANO_MARCO, SEEK_SET);
            fread(RAM + (marco_ram * TAMANO_MARCO), 1, TAMANO_MARCO, bin);
            
            //Actualizar TMM 
            tmm[marco_ram].propietario = pid; //proceso que se fue a suspendidos
            tmm[marco_ram].num_pagina = num_pagina;
            tmm[marco_ram].grupo = gid;
            mvprintw(39, 2, "Pagina %d del PID %d cargada en Marco RAM %d", num_pagina, pid, marco_ram);

            //Actualizar tmp's del mismo grupo                          
            while(aux_l != NULL) {
                if(aux_l->GID == gid && aux_l->PID != pid){
                    aux_l->tmp[num_pagina].num_marco_ram = marco_ram;
                } 
                aux_l = aux_l->siguiente;
            }

            while(aux_e != NULL) {
                if(aux_e->GID == gid && aux_e->PID != pid){
                    aux_e->tmp[num_pagina].num_marco_ram = marco_ram;
                } 
                aux_e = aux_e->siguiente;
            }

            while(aux_s != NULL) {
                if(aux_s->GID == gid && aux_s->PID != pid){
                    aux_s->tmp[num_pagina].num_marco_ram = marco_ram;
                } 
                aux_s = aux_s->siguiente;
            }

            return marco_ram; 
        }
    }

    return -1;
}

//Elimina todas las paginas de un proceso que termino a menos que existan mas procesos
// en el mismo grupo, en dado caso hereda la propiedad
void eliminarPaginas(struct Nodo *proceso, TablaMarcos *tms, TablaMarcos *tmm, struct Nodo *listos, struct Nodo *ejecutando, struct Nodo *suspendidos, FILE *bin, char *RAM){
    int pid_busqueda = proceso->PID; //el que se va a desalojar
    struct Nodo *heredero = buscarHerederoGID(listos, ejecutando, suspendidos, proceso->GID, pid_busqueda);
    char buffer[TAMANO_MARCO];
    memset(buffer,0,sizeof(buffer));
    
    if(heredero != NULL) { //Hay mas de un proceso con el mismo GID
        for (int i=0; i<TOTAL_MARCOS_RAM; i++){
            if(tmm[i].propietario == pid_busqueda){
                tmm[i].propietario = heredero->PID;
                heredero->tmp[tmm[i].num_pagina].num_marco_ram = tmm[i].num_marco;
            }
        }

        for (int i=0; i<TOTAL_MARCOS_DISCO; i++){
            if(tms[i].propietario == pid_busqueda){
                tms[i].propietario = heredero->PID;
                heredero->tmp[tms[i].num_pagina].num_marco_disco = tms[i].num_marco;
            }
        }
    } else { //No hay mas procesos con el mismo GID
        for (int i=0; i<TOTAL_MARCOS_RAM; i++){
            if(tmm[i].propietario == pid_busqueda){
                tmm[i].propietario = 0;
                tmm[i].grupo = 0;
                tmm[i].num_pagina = -1; 
                tmm[i].usado_recien = 0;
                memset((RAM + TAMANO_MARCO * i) ,0,TAMANO_MARCO);
            }
        }

        for(int i=0; i<TOTAL_MARCOS_DISCO; i++){
            if(tms[i].propietario == pid_busqueda){ 
                tms[i].propietario = 0;
                tms[i].grupo = 0;
                tms[i].num_pagina = -1;
                fseek(bin,TAMANO_MARCO*i,SEEK_SET); 
                fwrite(buffer, sizeof(char), TAMANO_MARCO, bin);
            }
        }
        fflush(bin); //vacia el bufer hacia el archivo fisico
    }

    //Dejamos la TMP del proceso que acabo limpia (aunque no hace falta)
    for (int i=0; i<proceso->num_paginas; i++){
        proceso->tmp[i].num_marco_disco = -1;
        proceso->tmp[i].num_marco_ram = -1;
    }
}

//Devuelve el primer proceso que encuentre perteneciente al mismo grupo de otro
struct Nodo* buscarHerederoGID(struct Nodo *listos, struct Nodo *ejecutando, struct Nodo *suspendidos, int gid, int pid_actual) {
    struct Nodo *aux_l = listos->siguiente;
    struct Nodo *aux_e = ejecutando->siguiente;
    struct Nodo *aux_s = suspendidos->siguiente;

    while(aux_l != NULL) {
        if(aux_l->GID == gid && aux_l->PID != pid_actual){
            return aux_l;
        } 
        aux_l = aux_l->siguiente;
    }

    while(aux_e != NULL) {
        if(aux_e->GID == gid && aux_e->PID != pid_actual){
            return aux_e;
        } 
        aux_e = aux_e->siguiente;
    }

    while(aux_s != NULL) {
        if(aux_s->GID == gid && aux_s->PID != pid_actual){
            return aux_s;
        } 
        aux_s = aux_s->siguiente;
    }
    return NULL; //No hay nadie mas en el grupo
}

//Busca un 0 de recien usado, desaloja el marco y devuelve ese marco
//Notifica a las TMp's de los procesos del mismo grupo que ya no tienen esa pagina 
struct TablaMarcos *algoritmoReloj(TablaMarcos *manecilla, struct Nodo *listos, struct Nodo *ejecutando, struct Nodo *suspendidos, TablaMarcos *tmm, char *RAM){
    struct Nodo *aux_l = listos->siguiente;
    struct Nodo *aux_e = ejecutando->siguiente;
    struct Nodo *aux_s = suspendidos->siguiente;
    
    while(manecilla->usado_recien == 1){
        manecilla->usado_recien = 0;
        manecilla = manecilla->siguiente;
        manecilla->puntero = false;
    }

    while(aux_l != NULL) {
        if(aux_l->GID == manecilla->grupo) {
            aux_l->tmp[manecilla->num_pagina].num_marco_ram = -1;
        }
        aux_l = aux_l->siguiente;
    }

    while(aux_e != NULL) {
        if(aux_e->GID == manecilla->grupo) {
            aux_e->tmp[manecilla->num_pagina].num_marco_ram = -1;
        }
        aux_e = aux_e->siguiente;
    }

    while(aux_s != NULL) {
        if(aux_s->GID == manecilla->grupo) {
            aux_s->tmp[manecilla->num_pagina].num_marco_ram = -1;
        }
        aux_s = aux_s->siguiente;
    }

    tmm[manecilla->num_marco].propietario = 0;
    tmm[manecilla->num_marco].num_pagina = -1;
    tmm[manecilla->num_marco].grupo = 0;

    memset((RAM + TAMANO_MARCO * manecilla->num_marco) ,0,TAMANO_MARCO);

    return manecilla; //Este es el marco con la pagina a desalojar
}