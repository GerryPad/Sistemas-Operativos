#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

char *delimitadores = " ,\n";
char *delim2 = ",\n"; //Para no permitir la sintaxis "inst reg , reg" (Es buena idea?)
char *instruccion[] = {"MOV", "ADD", "SUB", "MUL", "DIV", "INC", "DEC", "END", NULL};
typedef struct {
    char *nombre;
    int valor; //Quedate con registros ENTEROS, pero si pueden tener signo
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

//Necesito modificar esto para leer signos negativos
bool esInt(char *s) {
    if (s == NULL || *s == '\0') return false; //Por si ya no hay tokens en el renglon
    for (int i = 0; s[i] != '\0'; i++) {
        if(s[i] == '-' && i == 0) {
            continue;
        }
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

bool instMOV(){
    char *operando1, *operando2, *extra;
    operando1 = strtok(NULL, delim2);
    operando2 = strtok(NULL, delim2);

    if (operando1 == NULL || operando2 == NULL) {
        printf("Error: Instrucción incompleta.\n");
        return false;
    }
     /*Revisar cuando hay mas de dos registros validos.*/
     extra = strtok(NULL, delimitadores);
     if (extra != NULL){
        printf("Error: Demasiados argumentos.\n");
        return false;
     }

    Registro *reg1 = buscaRegistro(operando1);
   

    if (reg1 != NULL){
        Registro *reg2 = buscaRegistro(operando2);
        //printf("Operando dos: %s", operando2);
        if (reg2 != NULL){
            printf("MOV: %s valor original %d, %s valor original %d\n", reg1->nombre, reg1->valor, reg2->nombre, reg2->valor);
            reg1->valor = reg2->valor;
            printf("MOV: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esInt(operando2)) {
            printf("MOV: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = atoi(operando2); //atoi pasa de texto a int
            printf("MOV: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else {
            printf("Error: El segundo operando no es valido.\n");
            return false;
        }
    } else {
        printf("Error: el operando uno de MOV no es un registro\n");
        return false;
    }
    return true;
}

bool instADD(){
    char *operando1, *operando2, *extra;
    operando1 = strtok(NULL, delim2);
    operando2 = strtok(NULL, delim2);

    if (operando1 == NULL || operando2 == NULL) {
        printf("Error: Instrucción incompleta.\n");
        return false;
    }
     /*Revisar cuando hay mas de dos registros validos.*/
     extra = strtok(NULL, delimitadores);
     if (extra != NULL){
        printf("Error: Demasiados argumentos.\n");
        return false;
     }

    Registro *reg1 = buscaRegistro(operando1);
   

    if (reg1 != NULL){
        Registro *reg2 = buscaRegistro(operando2);
        if (reg2 != NULL){
            printf("ADD: %s valor original %d, %s valor original %d\n", reg1->nombre, reg1->valor, reg2->nombre, reg2->valor);
            reg1->valor = reg1->valor + reg2->valor;
            printf("ADD: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esInt(operando2)) {
            printf("ADD: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = reg1->valor + atoi(operando2); //atoi pasa de texto a int, atoi para floats?
            printf("ADD: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else {
            printf("Error: El segundo operando no es valido.\n");
            return false;
        }
    } else {
        printf("Error: el operando uno de ADD no es un registro\n");
        return false;
    }
    return true;
}

bool instSUB(){
    char *operando1, *operando2, *extra;
    operando1 = strtok(NULL, delim2);
    operando2 = strtok(NULL, delim2);

    if (operando1 == NULL || operando2 == NULL) {
        printf("Error: Instrucción incompleta.\n");
        return false;
    }
     /*Revisar cuando hay mas de dos registros validos.*/
     extra = strtok(NULL, delimitadores);
     if (extra != NULL){
        printf("Error: Demasiados argumentos.\n");
        return false;
     }

    Registro *reg1 = buscaRegistro(operando1);
   

    if (reg1 != NULL){
        Registro *reg2 = buscaRegistro(operando2);
        if (reg2 != NULL){
            printf("SUB: %s valor original %d, %s valor original %d\n", reg1->nombre, reg1->valor, reg2->nombre, reg2->valor);
            reg1->valor = reg1->valor - reg2->valor;
            printf("SUB: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esInt(operando2)) {
            printf("SUB: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = reg1->valor - atoi(operando2); //atoi pasa de texto a int, atoi para floats?
            printf("SUB: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else {
            printf("Error: El segundo operando no es valido.\n");
            return false;
        }
    } else {
        printf("Error: el operando uno de SUB no es un registro\n");
        return false;
    }
    return true;
}

bool instMUL(){
    char *operando1, *operando2, *extra;
    operando1 = strtok(NULL, delim2);
    operando2 = strtok(NULL, delim2);

    if (operando1 == NULL || operando2 == NULL) {
        printf("Error: Instrucción incompleta.\n");
        return false;
    }
     /*Revisar cuando hay mas de dos registros validos.*/
     extra = strtok(NULL, delimitadores);
     if (extra != NULL){
        printf("Error: Demasiados argumentos.\n");
        return false;
     }

    Registro *reg1 = buscaRegistro(operando1);
   

    if (reg1 != NULL){
        Registro *reg2 = buscaRegistro(operando2);
        if (reg2 != NULL){
            printf("MUL: %s valor original %d, %s valor original %d\n", reg1->nombre, reg1->valor, reg2->nombre, reg2->valor);
            reg1->valor = reg1->valor * reg2->valor;
            printf("MUL: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esInt(operando2)) {
            printf("MUL: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = reg1->valor * atoi(operando2); //atoi pasa de texto a int, atoi para floats?
            printf("MUL: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else {
            printf("Error: El segundo operando no es valido.\n");
            return false;
        }
    } else {
        printf("Error: el operando uno de MUL no es un registro\n");
        return false;
    }
    return true;
}

bool instDIV(){
    char *operando1, *operando2, *extra;
    operando1 = strtok(NULL, delim2);
    operando2 = strtok(NULL, delim2);
    double divisor;

    if (operando1 == NULL || operando2 == NULL) {
        printf("Error: Instrucción incompleta.\n");
        return false;
    }
     /*Revisar cuando hay mas de dos registros validos.*/
     extra = strtok(NULL, delimitadores);
     if (extra != NULL){
        printf("Error: Demasiados argumentos.\n");
        return false;
     }

    Registro *reg1 = buscaRegistro(operando1);
   

    if (reg1 != NULL){
        Registro *reg2 = buscaRegistro(operando2);
        if (reg2 != NULL){
            if(reg2->valor == 0){
                printf("Error: División por cero.\n");
                return false;
            }
            printf("DIV: %s valor original %d, %s valor original %d\n", reg1->nombre, reg1->valor, reg2->nombre, reg2->valor);
            reg1->valor = reg1->valor / reg2->valor;
            printf("DIV: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esInt(operando2)) {
            divisor = atoi(operando2);
            if (divisor == 0){
                printf("Error: División por cero.\n");
                return false;
            }
            printf("DIV: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = reg1->valor / atoi(operando2); //atoi pasa de texto a int, atoi para floats?
            printf("DIV: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else {
            printf("Error: El segundo operando no es valido.\n");
            return false;
        }
    } else {
        printf("Error: el operando uno de DIV no es un registro\n");
        return false;
    }
    return true;
}

bool instINC(){
    char *operando1, *operando2, *extra;
    operando1 = strtok(NULL, delimitadores);

    if (operando1 == NULL) {
        printf("Error: Instrucción incompleta.\n");
        return false;
    }
     /*Revisar cuando hay mas de dos registros validos.*/
     extra = strtok(NULL, delimitadores);
     if (extra != NULL){
        printf("Error: Demasiados argumentos.\n");
        return false;
     }

    Registro *reg1 = buscaRegistro(operando1);
   
    if (reg1 != NULL){
            printf("INC: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = reg1->valor + 1;
            printf("INC: %s ahora vale %d\n", reg1->nombre, reg1->valor);
    } else {
        printf("Error: el operando uno de INC no es un registro\n");
        return false;
    }
    return true;
}

bool instDEC(){
    char *operando1, *operando2, *extra;
    operando1 = strtok(NULL, delimitadores);

    if (operando1 == NULL) {
        printf("Error: Instrucción incompleta.\n");
        return false;
    }
     /*Revisar cuando hay mas de dos registros validos.*/
     extra = strtok(NULL, delimitadores);
     if (extra != NULL){
        printf("Error: Demasiados argumentos.\n");
        return false;
     }

    Registro *reg1 = buscaRegistro(operando1);
   
    if (reg1 != NULL){
            printf("DEC: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = reg1->valor - 1;
            printf("DEC: %s ahora vale %d\n", reg1->nombre, reg1->valor);
    } else {
        printf("Error: el operando uno de INC no es un registro\n");
        return false;
    }
    return true;
}

//Porque no funciona para cundo hay mas de una linea?
bool instEND(FILE *file){
    char *extra;
    char *delim = " ,\n";
    char buffer[128];

    while (fgets(buffer, sizeof(buffer), file) != NULL){
        extra = strtok(NULL, delimitadores);
        if (extra != NULL){
            printf("Error: Demasiados argumentos o instrucciones despues de END.\n");
            return false;
        }
    }

    printf("Se termino de procesar el archivo.\n");
    return true;
}


bool ejecOperacion(char *instruccion, FILE *file){
    if (strcmp(instruccion, "MOV") == 0){
        return instMOV();
    } else if (strcmp(instruccion, "ADD") == 0){
        return instADD();
    } else if (strcmp(instruccion, "SUB") == 0) {
        return instSUB();
    } else if (strcmp(instruccion, "MUL") == 0) {
        return instMUL();
    } else if (strcmp(instruccion, "DIV") == 0) {
        return instDIV();
    } else if (strcmp(instruccion, "INC") == 0) {
        return instINC();          
    } else if (strcmp(instruccion, "DEC") == 0) {
        return instDEC();
    } else if (strcmp(instruccion, "END") == 0) {
        return instEND(file);
    } else {
        return false;
    }
}


//Deberia leerse una vez y luego ejecutar, o ir ejecutando y abortar si hay error?
//Mejor ir ejecutando y reportar error.

/*La sintaxis Instruccion Registro, (Registro/Valor) es correcta? -> NO
Buscar la forma de no aceptar el espacio*/
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
                    if (!ejecOperacion(token, file)) {
                        printf("ABORTADO por error de sintaxis en renglón %d.\n", num_renglon);
                        break; 
                    }
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