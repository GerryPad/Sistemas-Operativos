#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "instrucciones.h"

int main(){
    char archivo[64], linea[128]; //Buffers para leer nombre de archivo y linea del archivo.
    size_t len;
    int num_renglon;    
    char *token;
    bool tokEND;
    //tokEND sirve para saber si ya hemos leido la instruccion END o aun no.

    do{
        tokEND = false;
        printf("Nombre de archivo (\"salir\" para terminar): ");
        fgets(archivo, sizeof(archivo), stdin);
        len = strcspn(archivo, "\r\n"); 
        archivo[len] = '\0';

        if (strcmp(archivo, "salir") == 0) break;

        if (access(archivo, F_OK) == 0) {
            printf("El archivo existe\n");        
            FILE *file = fopen(archivo, "rb");
            if (!file) {
                perror("fopen");
                continue;
            }
            
            num_renglon = 1;

            //fgets se detiene al leer un \n o EOF y agrega un \0 al final
            while (fgets(linea, sizeof(linea), file) != NULL) {
                printf("Renglon %d: %s", num_renglon, linea);
                char *ptr = linea;
                while (*ptr == ' ' || *ptr == '\t') ptr++;
                token = strtok(ptr, " \n");

                /*Para comprobar si al hallar un END aun hay cosas, aunque sean validas,
                despues de dicha instrucción.*/
                if (tokEND){
                    if (token != NULL) {
                        printf("Error: Se encontró contenido después END en el renglón %d.\n", num_renglon);
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
                            printf("ABORTADO por error de sintaxis en renglón %d.\n", num_renglon);
                            break; 
                        }
                    }

                    num_renglon++;
                    printf("\n");  

                } else {
                    printf("Token no valido: [%s]\n", token);
                    break;
                }               
            }
            if (tokEND){
                printf("Se termino de procesar el archivo.\n");
            } else {
                printf("Error: Se llego al final sin instrucción END\n");
            }
            fclose(file);
        } else {
            printf("El archivo NO existe\n");
        }
    } while(1);
   return 0;
}