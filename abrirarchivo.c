#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

char *delimitadores = " ,\n";
char *instruccion[] = {"MOV", "ADD", "SUB", "MUL", "DIV", "INC", "DEC", "END", NULL};
typedef struct {
    char *nombre;
    int valor; //Quien sabe si luego tendran que ser flotantes?
} Registro;

Registro registros[] = {
    {"EAX", 0},
    {"EBX", 0},
    {"ECX", 0},
    {"EDX", 0},
    {NULL, 0}
};

bool validarToken(char *arr[], char *tok){
    if (tok == NULL) return false; //Necesario para lineas vacias
    for (int i = 0; arr[i] != NULL; i++){
        if (strcmp(arr[i], tok) == 0){ //Usar strcmp para comparar cadenas
            return true;
        } 
    }
    return false;
}

bool esNumero(char *s) {
    if (s == NULL || *s == '\0') return false;
    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] < '0' || s[i] > '9') return false;
    }
    return true;
}

Registro* buscaRegistro(char *nombre){
    for (int i = 0; registros[i].nombre != NULL; i++){
        if (strcmp(registros[i].nombre, nombre) == 0){
            return &registros[i];
        }
    }
    return NULL;
}

void instMOV(){
    char *operando1, *operando2;
    operando1 = strtok(NULL, delimitadores);
    operando2 = strtok(NULL, delimitadores);

    if (operando1 == NULL || operando2 == NULL) {
        printf("Error: Instrucción incompleta.\n");
        return;
    }

    Registro *reg1 = buscaRegistro(operando1);

    if (reg1 != NULL){
        Registro *reg2 = buscaRegistro(operando2);
        if (reg2 != NULL){
            printf("MOV: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = reg2->valor;
            printf("MOV: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esNumero(operando2)) {
            printf("MOV: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = atoi(operando2); //atoi pasa de texto a int
            printf("MOV: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else {
            printf("Error: El segundo operando no es valido.\n");
        }
    } else {
        printf("Error: el operando uno de MOV no es un registro\n");
    }
}

void ejecOperacion(char *instruccion){
    if (strcmp(instruccion, "MOV") == 0){
        instMOV();
    } else if (strcmp(instruccion, "ADD")){

    } else if (strcmp(instruccion, "SUB")) {

    } else if (strcmp(instruccion, "MUL")) {

    } else if (strcmp(instruccion, "DIV")) {

    } else if (strcmp(instruccion, "INC")) {
                        
    } else if (strcmp(instruccion, "DEC")) {

    } else if (strcmp(instruccion, "END")) {

    } else {

    }
}

int main(){
    char archivo[64], buffer[128];
    size_t len;
    int num_renglon;    
    char *token;
    bool validTok;

    do{
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
            while (fgets(buffer, sizeof(buffer), file) != NULL) {
                printf("Renglon %d: %s", num_renglon, buffer);
                token = strtok(buffer, delimitadores);
                if(validarToken(instruccion, token)){
                    ejecOperacion(token);

                    /*while (token != NULL) {
                        printf("[%s] ", token);
                        token = strtok(NULL, delimitadores); //OJO: usar buffer como primer parametro causa bucle infinito
                    }*/
                    /*strtok usa un puntero interno, el primer parametro indica que el puntero
                    debe iniciarse en la direccion de dicho parametro; en caso de no colocar para-
                    metro (NULL) el puntero utiliza la ultima posicion en la que se quedo.*/
                    num_renglon++;
                    printf("\n");  
                } else {
                    printf("Token no valido: [%s]\n", token);
                    break;
                }               
            }
            fclose(file);
        } else {
            printf("El archivo NO existe\n");
        }
    } while(1);
   return 0;
}