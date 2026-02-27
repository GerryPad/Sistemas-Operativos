#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <curses.h>
#include "instrucciones.h"
#include <sys/select.h>

int kbhit(void);

char *comandos[]={"ejecuta", "salir", NULL};

void imprimir_registros(int renglon, char *instruccion){
    mvprintw(3,2, "PC      IR    EAX    EBX    ECX    EDX");
    move(5,2);
    clrtoeol();
    mvprintw(5, 2, "%-4d  %-6s  %-6d  %-6d  %-6d  %-6d", 
        renglon, 
        instruccion, 
        registros[0].valor, 
        registros[1].valor, 
        registros[2].valor, 
        registros[3].valor  
    );
    refresh();
}

int interpretar_comando(char *comando, char *archivo) {
    char *arg, *cmd = strtok(comando, " \n");

    if (cmd == NULL) return 0; //Para un enter sin comando

    if (strcmp(cmd, "salir") == 0) {
        return 1; 
    }

    if (strcmp(cmd, "ejecuta") == 0) {
        arg = strtok(NULL, " \n\r");
        if (arg != NULL) {
            strcpy(archivo, arg); 
            return 2;
        } else {
            return -1; //Cuando no hay nombre de archivo
        }
    }

    return 0; //Por default suponemos que el comando es invalido
}

int main(){
    char archivo[64], linea[128], comando[256]; //Buffers para leer nombre de archivo y linea del archivo.
    int num_renglon, com; //com es para hacer un "switch"
    char *token;
    bool tokEND;
    bool pedir_archivo = true, com_valido; //com_valido es para comprobar la existencia del comando
    bool interrumpido;
    
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

                if (com == 1){
                    endwin();
                    return 0;
                } else if (com == 2){
                   com_valido = true;
                } else {
                    move(16,2);
                    clrtoeol();
                    if (com == -1) {
                        mvprintw(16,2, "Error: Falta nombre de archivo.");
                    } else {
                        mvprintw(16,2,"Error: Comando invalido");
                    }
                    refresh();
                    usleep(1000000);
                }
            }
        }

        pedir_archivo=true; 

        if (access(archivo, F_OK) == 0) {
            FILE *file = fopen(archivo, "rb");
            if (!file) {
                perror("fopen");
                continue;
            }
            
            num_renglon = 1;
            interrumpido=false; //Bandera para cada archivo
            //fgets se detiene al leer un \n o EOF y agrega un \0 al final
            while (fgets(linea, sizeof(linea), file) != NULL) {
                char linea_copia[128];
                strcpy(linea_copia, linea);
                char *temp_token = strtok(linea_copia, " \n\r");
                //Se usa operador ternario para que mvprintw no muera
                imprimir_registros(num_renglon, temp_token ? temp_token : "---");
                refresh();
                if(kbhit()){
                    interrumpido=true;
                    flushinp();
                    move(15, 2); clrtoeol();//nueva linea para escribir
                    mvprintw(15, 2, "Interrupción: 'salir' o nuevo archivo: ");
                    echo();
                    //getstr(comando);
                    mvscanw(15,42,"%255[^\n]",comando); //Si usaba esto el espacio entre ejecuta y el nombre causa violacion de segmento
                    noecho();

                    com = interpretar_comando(comando, archivo);

                    if (com == 1){
                        fclose(file);
                        endwin();
                        return 0;
                    } else if (com == 2){
                        pedir_archivo = false;
                        if (access(archivo, F_OK) == 0){
                            break;
                        } else {
                            move(16,2);
                            clrtoeol();
                            mvprintw(16,2,"Archivo no existente");
                            pedir_archivo = true;
                            //continue;
                        }      
                       
                    } else {
                        move(16,2);
                        clrtoeol();
                        if (com == -1) {
                            mvprintw(16,2, "Error: Falta nombre de archivo.");
                            continue;
                        } else {
                            mvprintw(16,2,"Error: Comando invalido");
                            continue;
                        }
                        refresh();
                        usleep(1000000);
                        fclose(file);
                        break;
                    }

                }
                
                char *ptr = linea;
                while (*ptr == ' ' || *ptr == '\t') ptr++;
                token = strtok(ptr, " \n");

                /*Para comprobar si al hallar un END aun hay cosas, aunque sean validas,
                despues de dicha instrucción.*/
                if (tokEND){
                    if (token != NULL) {
                        move(12,10);
                        clrtoeol();
                        mvprintw(12, 10, "Error: Contenido tras END en Renglon %d", num_renglon);
                        tokEND = false; 
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

                /*Comprobar que la instruccion exista, si es END verificamos que no
                haya nada mas despues de END, si no lo es, se ejecuta la instruccion
                correspondiente.*/
                if (token!= NULL && validarToken(instruccion, token)){
                    char *argumentos = ptr + strlen(token) + 1;

                    if (strcmp(token, "END") == 0){
                        if (instEND()) {
                            tokEND = true;
                        } else {
                            tokEND = false;
                            break;
                        }
                    } else {
                        if (!ejecOperacion(token, argumentos)) {
                            mvprintw(8, 2, "ABORTADO: Error sintaxis en renglon %d", num_renglon);
                            mvprintw(12,2, "Motivo:");
                            break; 
                        }
                    }

                    usleep(1000000);
                    num_renglon++;
                } else {
                    move(12,10);
                    clrtoeol();
                    mvprintw(12, 10, "Token no valido: [%s]", token);
                    break;
                }               
            }
            //AQUI VA LO NUEVO
            if(!interrumpido){
                move(10, 2); clrtoeol();
                if (tokEND){
                    move(12,0);
                    clrtoeol();
                    mvprintw(10, 2, "Estado: Procesado con éxito.");
                    //Cambiar a presiona cualquier tecla para continuar o eliminar la espera de un teclazo
                } else {
                    mvprintw(10, 2, "Estado: Error - Falto END o abortado.");
                }
                fclose(file);//Borre el otro de tokend porque cerraba el archivo 2 veces entonces solo debe ser en la variable de no interrumpido
                refresh();
                //getch();
                //getstr(archivo);
                move(15, 2); clrtoeol(); //por si acaso limpiamos la linea y que no se encime
            } else {
                //Si fue interrumpido entonces se cierra
                fclose(file);
                move(15, 2); clrtoeol();
            }

        } else {
            // Este es el else de if (access(archivo, F_OK) == 0)
            mvprintw(11, 2, "El archivo NO existe.");
            getch();
            move(11, 2); clrtoeol();
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