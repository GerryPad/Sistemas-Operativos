#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <curses.h>
#include "instrucciones.h"
#include <sys/select.h>

int kbhit(void);

char *comando[]={"ejecuta", "salir", NULL};

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
int procesar_comando(char *strin, char *archivo){
    char *com = strtok(strin," \n\r");
    char *arg = strtok(NULL,"\n");
    if(com == NULL)
        return -1;

    if(arg == NULL){
        return 2;
    }

    for(int i=0;comando[i]!=NULL;i++){
        if(strcmp(comando[i],com)==0){
            strcpy(archivo,arg);
            return i;
        }    
    }
    return -1;
}


int main(){
    char archivo[64], linea[128]; //Buffers para leer nombre de archivo y linea del archivo.
    int num_renglon;    
    char *token;
    bool tokEND;
    bool pedir_archivo = true;
    bool interrumpido;

    int indice; //que comando es el que esta ejecutando
    //char *archtok; //para guardar en caso de que el comando sea correcto
    initscr();
    do{
        tokEND = false;
        if(pedir_archivo){
            move(20, 2); clrtoeol(); // Limpia la línea de petición
            mvprintw(20, 2, "Nombre de archivo ('salir' para terminar): ");
            echo();           //Ver el nombre del archivo mientras lo escribimos

            getstr(archivo);  //Versión ncurses de fgets
            noecho();         //Apagamos el eco para el resto del proceso
        }
        pedir_archivo=true; 
        if (strcmp(archivo, "salir") == 0) break;

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
                //Necesitamos el token sin destruir la linea para despues

                char linea_copia[128];
                strcpy(linea_copia, linea);
                char *temp_token = strtok(linea_copia, " \n\r");
                //Se usa operador ternario para que mvprintw no muera
                imprimir_registros(num_renglon, temp_token ? temp_token : "---");
                refresh();
                if(kbhit()){
                    interrumpido=true;
                    
                    move(15, 2); clrtoeol();//nueva linea para escribir
                    mvprintw(15, 2, "Interrupción: 'salir' o nuevo archivo: ");
                    echo();
                    mvscanw(15,42,"%63[^\n]",archivo); //Ahora se supone que tendra un nuevo nombre
                    noecho();
                    indice =procesar_comando(archivo, archivo);


                    if(indice==-1){
                        mvprintw(16,2,"Comando invalido");
                    }else if(indice==2){
                        mvprintw(16,2,"Falta nombre de archivo");
                    }

                    if(strcmp(comando[indice],"salir")==0){
                        fclose(file);
                        endwin();
                        return 0; //es por si escribe salir termine hasta el ncurses
                    }
                    else if(strcmp(comando[indice],"ejecuta")==0) {
                        //nuevo archivo. 
                        pedir_archivo = false; //amonos ya no es verdadero entonces lo reinicia
                        break; //Regresamos al fgets para el nuevo archivo
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