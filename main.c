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
    nuevo->PC = 1; 
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


//Imprimir primero el ejecutando, despues los listos en el orden que estan en listos y finalmente los terminados en su orden
void imprimir_listas(struct Nodo *cabecera_ejecutando, struct Nodo *cabecera_listos, struct Nodo *cabecera_terminados){
    struct Nodo *aux_e = cabecera_ejecutando->siguiente;
    struct Nodo *aux_l = cabecera_listos->siguiente;
    struct Nodo *aux_te = cabecera_terminados->siguiente;

    mvprintw(7, 2, "%-6s %-8s %-12s %-8s %-15s %-6s %-6s %-6s %-6s", 
        "PID", "File", "Estatus", "PC", "IR", "EAX", "EBX", "ECX", "EDX");


    for(int j = 8; j<20; j++){
        move(j,2);
        clrtoeol();
    } 
 //Esta es la lista de ejecutando
   
    if(aux_e != NULL){
        mvprintw(8, 2, "%-6d %-8s %-12s %-8d %-15s %-6d %-6d %-6d %-6d", 
        aux_e->PID,
        aux_e->archivo,
        aux_e->estado,
        aux_e->PC,
        aux_e->IR,
        aux_e->registros[0],
        aux_e->registros[1],
        aux_e->registros[2],
        aux_e->registros[3] 
        );
    }


    int i=9;
    while(aux_l != NULL){

        if(i>=20 || aux_l == NULL){
            break;
        }

        move(i,2);
        clrtoeol();
        mvprintw(i, 2, "%-6d %-8s %-12s %-8d %-15s %-6d %-6d %-6d %-6d", 
        aux_l->PID,
        aux_l->archivo,
        aux_l->estado,
        aux_l->PC,
        aux_l->IR,
        aux_l->registros[0],
        aux_l->registros[1],
        aux_l->registros[2],
        aux_l->registros[3] 
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
        mvprintw(i, 2, "%-6d %-8s %-12s %-8d %-15s %-6d %-6d %-6d %-6d", 
        aux_te->PID,
        aux_te->archivo,
        aux_te->estado,
        aux_te->PC,
        aux_te->IR,
        aux_te->registros[0],
        aux_te->registros[1],
        aux_te->registros[2],
        aux_te->registros[3] 
        );

        aux_te = aux_te->siguiente;
        i++;
    }
    refresh();
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

//El planificador primitivo, solo devuelve el primero que este en listos
struct Nodo* planificador(struct Nodo *listos, struct Nodo *ejecutando) {
    if (listos->siguiente == NULL){
        return NULL; //Caundo no hay nada en listos
    } 
    struct Nodo *proceso = desencolar(listos);
    strcpy(proceso->estado, "ejecutando");
    insertarFinal(ejecutando, proceso);
    return proceso;
}

int comando_matar(char *comando, int pid) {
    char *arg, *basura, *cmd;

    cmd = strtok(comando, " \n");
    arg = strtok(NULL, " \n\r");

    if (cmd == NULL) return 0; //Para un enter sin comando


    if(strcmp(cmd, "mata") == 0){
        basura = strtok(NULL, " \n");
        if (arg == NULL){
            return -1;
        }

        if(basura != NULL){
            move(24,10);
            clrtoeol();
            mvprintw(24, 10,"Demasiados argumentos.");
            return 0;
        } else {
            pid = atoi(arg);
            return pid;
        }
    }

    return 0;
}


int kbhit(void);        

int main(){

    //Creando nodo de prueba para impresion
    struct Nodo *listos = crearCabecera();
    struct Nodo *terminados = crearCabecera();
    struct Nodo *ejecutando = crearCabecera();
    struct Nodo *nuevo;
    struct Nodo *proceso_actual = NULL; //El que se esta ejecutando
    struct Nodo *proceso_a_terminar = NULL;
    struct Nodo *proceso_a_matar = NULL;


    char archivo[64], linea[128], comando[256], com_mata[256], linea_original[128];; //Buffers para leer nombre y linea del archivo.
    int pc, com, pid=1, pid_kill=0, *ptr_pid = &pid_kill; //com es para hacer un "switch"
    char *token, *ptr, *argumentos;
    bool tokEND, com_valido, interrumpido; //com_valido es para comprobar la existencia del comando
    bool limpieza = false; 
    
    initscr();
    do{
        tokEND = false;

        if(ejecutando->siguiente == NULL){ //Cambiar el uso de la bandera pedir archivo
            if(listos->siguiente != NULL){
                proceso_actual = planificador(listos, ejecutando); //Hacer que el planificador te de el primero de listos

                //cargar su "contexto", de momento pues esta en ceros
                pc = restauraPCB(proceso_actual, archivo); 

            } else {
                com_valido = false; //Suponemos de entrada que el comando no es valido

                while (!com_valido){ //Solicitamos comando hasta que haya uno valido
                
                    move(20,2);
                    clrtoeol();
                    mvprintw(20,2, ">");
                    echo();
                    getstr(comando);
                    noecho();
                    move(20, 2); 
                    clrtoeol(); 
                    refresh();
                
                    com = interpretar_comando(comando, archivo, ptr_pid); 
                    limpieza = true;

                    if (com == 1){ //comando salir
                        endwin();
                        return 0;
                    } else if (com == 2){ //comando ejecuta
                    com_valido = true;
                    nuevo=crearNodo(pid, archivo);
                    pid++;
                    insertarFinal(listos,nuevo);
                    } else if(com == 3){
                         mvprintw(25, 2, "No hay ningun proceso para matar.");
                    }else { //error al ingresar comando
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
                continue;

            }

            
        }

        if(limpieza){
            limpia_lineas();
            limpieza = false;
        }

        if (access(archivo, F_OK) == 0) {
            FILE *file = fopen(archivo, "rb");
            if (!file) {
                perror("fopen");
                continue;
            }

            int quantum = 0;
            bool fin_quantum = false; //para saber porque motivo cerramos proceso
            int i=1;
            while(i<pc && fgets(linea, sizeof(linea), file) != NULL){
                i++;
                continue;
            }
           
            interrumpido=false; //Bandera para cada archivo
            strcpy(linea_original, "---");
            while (fgets(linea, sizeof(linea), file) != NULL) {
                linea[strcspn(linea, "\n\r")] = '\0';
                strcpy(linea_original, linea);//Para imprimir la linea original en PCB
                imprimir_registros(pc, linea);
                /*imprimir_listas(listos);
                imprimir_listas(ejecutando);
                imprimir_listas(terminados);*/
                imprimir_listas(ejecutando, listos, terminados);
                refresh();
                
                ptr = linea;
                while (*ptr == ' ' || *ptr == '\t') ptr++; 
                token = strtok(ptr, " \n");

                if (tokEND){ //Si hayamos un END...
                    if (token != NULL) { //Pero hay mas cosas despues
                        move(24,10);
                        clrtoeol();
                        mvprintw(24, 10, "Error: Contenido tras END en Renglon %d", pc);
                        proceso_a_terminar = desencolar(ejecutando); //Siguiendo la logica de Pedro

                        if (proceso_a_terminar != NULL) {
                            strcpy(proceso_a_terminar->estado, "terminado*");
                            insertarFinal(terminados, proceso_a_terminar);
                        }
                       
                        /*imprimir_listas(listos);
                        imprimir_listas(ejecutando);
                        imprimir_listas(terminados);*/
                        imprimir_listas(ejecutando, listos, terminados);
                        tokEND = false; 
                        limpieza = true;
                        break;
                    }
                    pc++;
                    continue;
                }

                //Para aquellos renglones vacios
                if (token == NULL) {
                    pc++;
                    continue;
                }

                if (token!= NULL && validarToken(instruccion, token)){
                    argumentos = ptr + strlen(token) + 1; //Reconocer lo que esta despues del nemonico

                    if (strcmp(token, "END") == 0){
                        if (instEND()) {
                            tokEND = true;
                        } else {
                            tokEND = false;
                            proceso_a_terminar = desencolar(ejecutando);
                            if (proceso_a_terminar != NULL) {
                                strcpy(proceso_a_terminar->estado, "terminado*");
                                insertarFinal(terminados, proceso_a_terminar);
                            }
                            limpieza=true;
                            break;
                        }
                    } else {
                        if (!ejecOperacion(token, argumentos)) {
                            guardaPCB(proceso_actual,pc,linea_original);
                            mvprintw(22, 2, "ABORTADO: Error en renglon %d", pc);
                            mvprintw(24,2, "Motivo:");

                            //Mover los procesos fallidos a terminados
                            proceso_a_terminar = desencolar(ejecutando);
                            if (proceso_a_terminar != NULL) {
                                strcpy(proceso_a_terminar->estado, "terminado*");
                                insertarFinal(terminados, proceso_a_terminar);
                            }
                            /*imprimir_listas(listos);
                            imprimir_listas(ejecutando);
                            imprimir_listas(terminados);*/
                            imprimir_listas(ejecutando, listos, terminados);

                            limpieza = true;
                            break; 
                        }
                    }
                    usleep(1000000);
                    pc++;
                    quantum++;

                    if(tokEND){
                        continue;
                    }

                    if (quantum == 3) {
                        guardaPCB(proceso_actual, pc, linea_original);
                        proceso_a_terminar = desencolar(ejecutando);
                        if (proceso_a_terminar!= NULL) {
                            strcpy(proceso_a_terminar->estado, "listos");
                            insertarFinal(listos, proceso_a_terminar);
                        }
                        fclose(file);
                        proceso_actual = NULL;
                        fin_quantum = true;
                        limpieza = true;
                        
                        /*imprimir_listas(listos);
                        imprimir_listas(ejecutando);*/
                        imprimir_listas(ejecutando, listos, terminados);
                        refresh();
                        
                        break; 
                    }

                    //Movi el kbhit despues para que no haga falta aumentar pc dentro de kbhit
                    if(kbhit()){ //Cuando haya un teclazo
                        if(limpieza){ //tambien puede que no sea correco guardarlo asi
                            limpia_lineas();
                        }
                        interrumpido=true;
                        getch();

                        refresh();
                        move(20,2);
                        clrtoeol();
                        mvprintw(20, 2, ">");
                        echo();
                        mvscanw(20,3,"%255[^\n]",comando);
                        noecho();
                        limpieza = true;
                        strcpy(com_mata, comando);
                        com = interpretar_comando(comando, archivo, ptr_pid);

                        if (com == 1){
                            fclose(file);
                            endwin();
                            return 0;
                        } else if (com == 2){
                            if (access(archivo, F_OK) == 0){
                                nuevo=crearNodo(pid, archivo);
                                pid++;
                                insertarFinal(listos,nuevo);
                                interrumpido = false;
                                continue; //Para seguir con el proceso actual y que no se cambie por el nuevo
                            } else {
                                move(25,2);
                                clrtoeol();
                                mvprintw(25,2,"Archivo no existente");
                                limpieza = true;
                                move(20,2);
                                clrtoeol();
                                refresh();
                            }      
                        
                        } else if(com == 3){
                            proceso_a_matar = mataPID(ejecutando, pid_kill);
                            if(proceso_a_matar != NULL){
                                strcpy(proceso_a_matar->estado, "terminados**");
                                insertarFinal(terminados,proceso_a_matar);
                                imprimir_listas(ejecutando,listos,terminados);
                                //fclose(file);
                                break; 
                                //proceso_actual = NULL;
                            } else{
                                proceso_a_matar = mataPID(listos, pid_kill);
                                if(proceso_a_matar != NULL){
                                    strcpy(proceso_a_matar->estado, "terminados**");
                                    insertarFinal(terminados,proceso_a_matar);
                                    imprimir_listas(ejecutando,listos,terminados);
                                    //proceso_actual = NULL;
                                } else {
                                    move(25,2);
                                    clrtoeol();
                                    mvprintw(25,2, "El PID asociado al proceso no existe.");
                                }
                            } 
                        }
                        
                        else {
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

                } else {
                    move(24,2);
                    clrtoeol();
                    mvprintw(24, 2, "Token no valido: [%s]", token);
                    proceso_a_terminar = desencolar(ejecutando);
                    if (proceso_a_terminar != NULL) {
                        strcpy(proceso_a_terminar->estado, "terminado");
                        insertarFinal(terminados, proceso_a_terminar);
                    }
                    limpieza = true;
                    break;
                }               
            }

            if(fin_quantum){ //esta bandera evita el doble cierre de archivos y el core dumped
                move(23, 2); clrtoeol();
                mvprintw(23, 2, "Quantum = 3. Cambio de proceso");
                refresh();
            } else if(!interrumpido){
                move(23, 2); clrtoeol();
                if (tokEND){
                    move(23,2);
                    clrtoeol();
                    mvprintw(23, 2, "Estado: Procesado con éxito.");
                    guardaPCB(proceso_actual,pc,linea_original);
                    proceso_a_terminar = desencolar(ejecutando);

                    if (proceso_a_terminar != NULL) {
                        strcpy(proceso_a_terminar->estado, "terminado");
                        insertarFinal(terminados, proceso_a_terminar);
                    }

                    /*imprimir_listas(listos);
                    imprimir_listas(ejecutando);
                    imprimir_listas(terminados);*/
                    imprimir_listas(ejecutando, listos, terminados);
                } else {
                    mvprintw(23, 2, "Estado: Error - Falto END o abortado.");
                    limpieza = true;
                    guardaPCB(proceso_actual,pc,linea_original);
                    proceso_a_terminar = desencolar(ejecutando);

                    if (proceso_a_terminar != NULL) {
                        strcpy(proceso_a_terminar->estado, "terminado*");
                        insertarFinal(terminados, proceso_a_terminar);
                    }
                    proceso_actual = NULL;
                }
                fclose(file);
                proceso_actual = NULL;
                /*imprimir_listas(listos);
                imprimir_listas(ejecutando);
                imprimir_listas(terminados);*/
                imprimir_listas(ejecutando, listos, terminados);
                refresh();
            } else {
                fclose(file);
                if(proceso_actual!=NULL){
                    guardaPCB(proceso_actual,pc,linea_original);
                }

                proceso_a_terminar = desencolar(ejecutando);
                if (proceso_a_terminar != NULL) {
                    strcpy(proceso_a_terminar->estado, "terminado");
                    insertarFinal(terminados, proceso_a_terminar);
                }
                //Si el proceso actual termino, ya no estamos ejecutando nada
                proceso_actual = NULL;
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