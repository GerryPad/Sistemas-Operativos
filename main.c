#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <curses.h>
#include "instrucciones.h"
#include "ncurses.h"
#include <sys/select.h>

int kbhit(void);        

int main(){
    char archivo[64], linea[128], comando[256]; //Buffers para leer nombre y linea del archivo.
    int num_renglon, com; //com es para hacer un "switch"
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
                limpieza = true;

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
                imprimir_registros(num_renglon, linea);
                refresh();
                if(kbhit()){
                    if(limpieza){
                        limpia_lineas();
                    }
                    interrumpido=true;
                    getch();
                    move(15, 0); clrtoeol();
                    refresh();
                    mvprintw(15, 2, "Interrupción: 'salir' o nuevo archivo:");
                    echo();
                    mvscanw(15,42,"%255[^\n]",comando);
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
                            limpieza = true;
                            move(15,40);
                            clrtoeol();
                            refresh();
                        }      
                       
                    } else {
                        move(16,2);
                        clrtoeol();
                        if (com == -1) {
                            mvprintw(16,2, "Error: Falta nombre de archivo.");
                            limpieza = true;
                            continue;
                        } else {
                            mvprintw(16,2,"Error: Comando invalido");
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
                while (*ptr == ' ' || *ptr == '\t') ptr++;
                token = strtok(ptr, " \n");

                if (tokEND){
                    if (token != NULL) {
                        move(12,10);
                        clrtoeol();
                        mvprintw(12, 10, "Error: Contenido tras END en Renglon %d", num_renglon);
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
                    argumentos = ptr + strlen(token) + 1;

                    if (strcmp(token, "END") == 0){
                        if (instEND()) {
                            tokEND = true;
                        } else {
                            tokEND = false;
                            break;
                        }
                    } else {
                        if (!ejecOperacion(token, argumentos)) {
                            mvprintw(8, 2, "ABORTADO: Error en renglon %d", num_renglon);
                            mvprintw(12,2, "Motivo:");
                            limpieza = true;
                            break; 
                        }
                    }

                    usleep(1000000);
                    num_renglon++;
                } else {
                    move(12,2);
                    clrtoeol();
                    mvprintw(12, 2, "Token no valido: [%s]", token);
                    limpieza = true;
                    break;
                }               
            }

            if(!interrumpido){
                move(10, 2); clrtoeol();
                if (tokEND){
                    move(10,2);
                    clrtoeol();
                    mvprintw(10, 2, "Estado: Procesado con éxito.");
                } else {
                    mvprintw(10, 2, "Estado: Error - Falto END o abortado.");
                    limpieza = true;
                }
                fclose(file);
                refresh();
                move(15, 2); clrtoeol(); 
            } else {
                fclose(file);
                move(15, 2); clrtoeol();
            }

        } else {
            mvprintw(11, 2, "El archivo NO existe.");
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