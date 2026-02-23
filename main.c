#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <curses.h>
#include "instrucciones.h"
#include <sys/select.h>
int kbhit(void);
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

int main(){
    char archivo[64], linea[128]; //Buffers para leer nombre de archivo y linea del archivo.
    //size_t len;
    int num_renglon;    
    char *token;
    bool tokEND;

    initscr();
    //tokEND sirve para saber si ya hemos leido la instruccion END o aun no.

    do{
        tokEND = false;

        move(20, 2); clrtoeol(); // Limpia la línea de petición
        mvprintw(20, 2, "Nombre de archivo ('salir' para terminar): ");
        echo();           //Ver el nombre del archivo mientras lo escribimos

        getstr(archivo);  //Versión ncurses de fgets
        noecho();         //Apagamos el eco para el resto del proceso

        /*printf("Nombre de archivo (\"salir\" para terminar): ");
        fgets(archivo, sizeof(archivo), stdin);
        len = strcspn(archivo, "\r\n"); 
        archivo[len] = '\0';*/

        if (strcmp(archivo, "salir") == 0) break;

        if (access(archivo, F_OK) == 0) {
            //printf("El archivo existe\n");        
            FILE *file = fopen(archivo, "rb");
            if (!file) {
                perror("fopen");
                continue;
            }
            
            num_renglon = 1;

            //fgets se detiene al leer un \n o EOF y agrega un \0 al final
            while (fgets(linea, sizeof(linea), file) != NULL) {
                //Necesitamos el token sin destruir la linea para despues
                char linea_copia[128];
                strcpy(linea_copia, linea);
                char *temp_token = strtok(linea_copia, " \n\r");

                //Se usa operador ternario para que mvprintw no muera
                imprimir_registros(num_renglon, temp_token ? temp_token : "---");
                refresh();
                char *ptr = linea;
                while (*ptr == ' ' || *ptr == '\t') ptr++;
                token = strtok(ptr, " \n");

                /*Para comprobar si al hallar un END aun hay cosas, aunque sean validas,
                despues de dicha instrucción.*/
                if (tokEND){
                    if (token != NULL) {
                        //printf("Error: Se encontró contenido después END en el renglón %d.\n", num_renglon);
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
                            //printf("ABORTADO por error de sintaxis en renglón %d.\n", num_renglon);
                            mvprintw(8, 2, "ABORTADO: Error sintaxis en renglon %d", num_renglon);
                            mvprintw(12,2, "Motivo:");
                            break; 
                        }
                    }

                    usleep(1000000);
                    num_renglon++;
                    //printf("\n");  

                } else {
                    //printf("Token no valido: [%s]\n", token);
                    move(12,10);
                    clrtoeol();
                    mvprintw(12, 10, "Token no valido: [%s]", token);
                    break;
                }               
            }
            move(10, 2); clrtoeol();
            if (tokEND){
                //printf("Se termino de procesar el archivo.\n");
                move(12,0);
                clrtoeol();
                mvprintw(10, 2, "Estado: Procesado con éxito.");
            } else {
                //printf("Error: Se llego al final sin instrucción END\n");
                mvprintw(10, 2, "Estado: Error - Falto END o abortado.");
            }
            fclose(file);
            refresh();
            getch(); //Pausa para ver el resultado final antes de pedir otro archivo
        } else {
            //printf("El archivo NO existe\n");
            mvprintw(2, 2, "El archivo NO existe.");
            getch();
            move(2, 2); clrtoeol();
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

