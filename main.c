#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <curses.h>
#include "instrucciones.h"
#include "ncurses.h"
#include <sys/select.h>

struct Nodo{
    int PID;
    char estado[16];
    int PC;
    char archivo[64]; //O usar FILE * (?)
    char IR[64];
    int registros[4];
    struct Nodo *siguiente;
};

struct Nodo* crearCabecera(){
    struct Nodo *cabecera = malloc(sizeof(struct Nodo));
    cabecera->siguiente = NULL;
    return cabecera;
}

struct Nodo* crearNodo(int n, char *archivo){
    struct Nodo *nuevo = malloc(sizeof(struct Nodo));
    nuevo->PID = n;
    nuevo->PC = 0; 
    for (int i=0; i<4; i++){
        nuevo->registros[i] = 0;
    }
    strcpy(nuevo->archivo, archivo);
    strcpy(nuevo->IR, "---");
    nuevo->siguiente = NULL;
    strcpy(nuevo->estado, "listos");
    return nuevo;
}

void insertarFinal(struct Nodo *cabecera, struct Nodo *nuevo){
    struct Nodo *aux = cabecera;

    while(aux->siguiente != NULL){
        aux = aux->siguiente;
    }

    aux->siguiente = nuevo;
}


void imprimir_listas(struct Nodo *cabecera){
    struct Nodo *aux = cabecera->siguiente;
    mvprintw(7, 2, "%-6s %-8s %-12s %-8s %-15s %-6s %-6s %-6s %-6s", 
        "PID", "File", "Estatus", "PC", "IR", "EAX", "EBX", "ECX", "EDX");
    while(aux!=NULL && 7+aux->PID < 20){
        move(7+aux->PID,2);
        clrtoeol();
        mvprintw(7+aux->PID, 2, "%-6d %-8s %-12s %-8d %-15s %-6d %-6d %-6d %-6d", 
        aux->PID,
        aux->archivo,
        aux->estado,
        aux->PC,
        aux->IR,
        aux->registros[0],
        aux->registros[1],
        aux->registros[2],
        aux->registros[3] 
        );
        refresh();

        aux = aux->siguiente;
    }
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
        printf("La lista esta vacia\n");
        return NULL;
    }

    lista->siguiente=lista->siguiente->siguiente;
    aux->siguiente=NULL;
    return(aux);
}

void guardaPCB(struct Nodo *PCB, int num_renglon, char *linea){
    PCB->PC = num_renglon;
    strcpy(PCB->IR,linea);
    PCB->registros[0]=registros[0].valor;
    PCB->registros[1]=registros[1].valor;
    PCB->registros[2]=registros[2].valor;
    PCB->registros[3]=registros[3].valor;
}

int kbhit(void);        

int main(){

    //Creando nodo de prueba para impresion
    struct Nodo *listos = crearCabecera();
    struct Nodo *terminados = crearCabecera();
    struct Nodo *nuevo;


    char archivo[64], linea[128], comando[256]; //Buffers para leer nombre y linea del archivo.
    int num_renglon, com, pid=1; //com es para hacer un "switch"
    char *token, *ptr, *argumentos;
    bool tokEND, com_valido, interrumpido; //com_valido es para comprobar la existencia del comando
    bool pedir_archivo = true, limpieza = false; 
    
    initscr();
    do{
        tokEND = false;

        if(pedir_archivo){
            com_valido = false; //Suponemos de entrada que el comando no es valido

            while (!com_valido){ //Solicitamos comando hasta que haya uno valido
               
                move(20,2);
                mvprintw(20,2, ">");
                echo();
                getstr(comando);
                noecho();
                move(20, 2); 
                clrtoeol(); 
                refresh();
               
                com = interpretar_comando(comando, archivo); 
                nuevo=crearNodo(pid, archivo);
                pid++;
                insertarFinal(listos,nuevo);
                
                limpieza = true;

                if (com == 1){ //comando salir
                    endwin();
                    return 0;
                } else if (com == 2){ //comando ejecuta
                   com_valido = true;
                } else { //error al ingresar comando
                    move(25,2);
                    clrtoeol();
                    if (com == -1) {
                        mvprintw(25,2, "Error: Falta nombre de archivo.");
                    } else {
                        mvprintw(25,2,"Error: Comando invalido");
                    }
                    refresh();
                    usleep(1000000);
                }
            }
        }

        if(limpieza){
            limpia_lineas();
            limpieza = false;
        }

        pedir_archivo=true; 

        if (access(archivo, F_OK) == 0) {
            FILE *file = fopen(archivo, "rb");
            if (!file) {
                perror("fopen");
                continue;
            }
            
            num_renglon = 0;
            interrumpido=false; //Bandera para cada archivo
            while (fgets(linea, sizeof(linea), file) != NULL) {
                linea[strcspn(linea, "\n\r")] = '\0'; 
                guardaPCB(nuevo,num_renglon,linea);
                imprimir_registros(num_renglon, linea);
                imprimir_listas(listos);
                imprimir_listas(terminados);
                refresh();
                if(kbhit()){ //Cuando haya un teclazo
                    if(limpieza){
                        limpia_lineas();
                    }
                    interrumpido=true;
                    getch();
                    //move(15, 0); clrtoeol();
                    refresh();
                    mvprintw(20, 2, ">");
                    echo();
                    mvscanw(20,3,"%255[^\n]",comando);
                    noecho();
                    limpieza = true;
                    com = interpretar_comando(comando, archivo);

                    if (com == 1){
                        fclose(file);
                        endwin();
                        return 0;
                    } else if (com == 2){
                        pedir_archivo = false;
                        if (access(archivo, F_OK) == 0){
                            nuevo=crearNodo(pid, archivo);
                            pid++;
                            insertarFinal(listos,nuevo);
                            break;
                        } else {
                            move(25,2);
                            clrtoeol();
                            mvprintw(25,2,"Archivo no existente");
                            pedir_archivo = true;
                            limpieza = true;
                            move(20,2);
                            clrtoeol();
                            refresh();
                        }      
                       
                    } else {
                        move(25,2);
                        clrtoeol();
                        if (com == -1) {
                            mvprintw(25,2, "Error: Falta nombre de archivo.");
                            limpieza = true;
                            continue;
                        } else {
                            mvprintw(25,2,"Error: Comando invalido");
                            limpieza = true;
                            continue;
                        }
                        refresh();
                        usleep(1000000);
                        fclose(file);
                        break;
                    }

                }
                
                ptr = linea;
                while (*ptr == ' ' || *ptr == '\t') ptr++; //Ignorar esapcios y tabulaciones al inicio o final
                token = strtok(ptr, " \n");

                if (tokEND){ //Si hayamos un END...
                    if (token != NULL) { //Pero hay mas cosas despues
                        move(24,10);
                        clrtoeol();
                        mvprintw(24, 10, "Error: Contenido tras END en Renglon %d", num_renglon);
                        nuevo=desencolar(listos);
                        strcpy(nuevo->estado,"terminado");
                        insertarFinal(terminados,nuevo);
                        imprimir_listas(listos);
                        imprimir_listas(terminados);
                        tokEND = false; 
                        limpieza = true;
                        break;
                    }
                    num_renglon++;
                    continue;
                }

                //Para aquellos renglones vacios
                if (token == NULL) {
                    num_renglon++;
                    continue;
                }

                if (token!= NULL && validarToken(instruccion, token)){
                    argumentos = ptr + strlen(token) + 1; //Reconocer lo que esta despues del nemonico

                    if (strcmp(token, "END") == 0){
                        if (instEND()) {
                            tokEND = true;
                        } else {
                            tokEND = false;
                            break;
                        }
                    } else {
                        if (!ejecOperacion(token, argumentos)) {
                            mvprintw(22, 2, "ABORTADO: Error en renglon %d", num_renglon);
                            mvprintw(24,2, "Motivo:");
                            limpieza = true;
                            break; 
                        } else {
                            /*nuevo->PC = num_renglon;
                            strcpy(nuevo->IR,linea);
                            nuevo->registros[0]=registros[0].valor;
                            nuevo->registros[1]=registros[1].valor;
                            nuevo->registros[2]=registros[2].valor;
                            nuevo->registros[3]=registros[3].valor;*/
                        }
                    }

                    usleep(1000000);
                    num_renglon++;
                } else {
                    move(24,2);
                    clrtoeol();
                    mvprintw(24, 2, "Token no valido: [%s]", token);
                    limpieza = true;
                    break;
                }               
            }

            if(!interrumpido){
                move(23, 2); clrtoeol();
                if (tokEND){
                    move(23,2);
                    clrtoeol();
                    mvprintw(23, 2, "Estado: Procesado con éxito.");
                    nuevo=desencolar(listos);
                    strcpy(nuevo->estado,"terminado");
                    insertarFinal(terminados,nuevo);
                    imprimir_listas(listos);
                    imprimir_listas(terminados);
                } else {
                    mvprintw(23, 2, "Estado: Error - Falto END o abortado.");
                    /*nuevo=desencolar(listos);
                    strcpy(nuevo->estado,"terminado");
                    insertarFinal(terminados,nuevo);
                    imprimir_listas(listos);
                    imprimir_listas(terminados);*/
                    limpieza = true;
                }
                fclose(file);
                refresh();
                //move(15, 2); clrtoeol(); 
            } else {
                fclose(file);
                //move(15, 2); clrtoeol();
            }

        } else {
            mvprintw(22, 2, "El archivo NO existe.");
            limpieza = true;
            
        }
    } while(1);
    endwin();
    return 0;
}

int kbhit(void) 
{
    struct timeval tv;
    fd_set read_fd;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&read_fd);
    FD_SET(0, &read_fd);

    if (select(1, &read_fd, NULL, NULL, &tv) == -1)
        return 0;

    if (FD_ISSET(0, &read_fd))
        return 1;

    return 0;
}