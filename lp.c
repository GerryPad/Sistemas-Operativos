#include <stdio.h>
#include <stdlib.h>

struct Nodo{
    int PID;
    struct Nodo *siguiente;
};

struct Nodo* crearCabecera(){
    struct Nodo *cabecera = malloc(sizeof(struct Nodo));
    cabecera->siguiente = NULL;
    return cabecera;
}

struct Nodo* crearNodo(int n){
    struct Nodo *nuevo = malloc(sizeof(struct Nodo));
    nuevo->PID = n;
    nuevo->siguiente = NULL;
    return nuevo;
}

void insertarInicio(struct Nodo *cabecera, struct Nodo *nuevo){
    nuevo->siguiente = cabecera->siguiente;
    cabecera->siguiente = nuevo;
}

void insertarFinal(struct Nodo *cabecera, struct Nodo *nuevo){
    struct Nodo *aux = cabecera;

    while(aux->siguiente != NULL){
        aux = aux->siguiente;
    }

    aux->siguiente = nuevo;
}

void imprimirLista(struct Nodo *cabecera){
    struct Nodo *aux = cabecera->siguiente;
    while(aux != NULL){
        printf("%d ", aux->PID);
        aux = aux->siguiente;
    }
    printf("\n");
}

struct Nodo *mataPID(struct Nodo *lista, int PID){
    struct Nodo * aux = lista->siguiente;
    struct Nodo * aux2 = lista;

    while(aux->PID != PID){
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

int main(){

    struct Nodo *lista = crearCabecera();
    struct Nodo *ejecutando = crearCabecera();
    struct Nodo *terminados = crearCabecera();
    struct Nodo *nuevo;

    int n, opcion;

    while(1){
        
        printf(
            "Inicio=1\nFinal=0\nDesencolar listos a ejecutando=2\nDesencolar ejecutando a listos=3\nMata = 4\n");
        printf("Opcion: \n");
        scanf("%d",&opcion);

        if(opcion == 1){
            printf("Ingrese PID: ");
            scanf("%d",&n);
            if(n == -1){
            break;
            }
            struct Nodo *nuevo = crearNodo(n);
            insertarInicio(lista, nuevo);
        }else if(opcion==0){
            printf("Ingrese PID (-1 para salir): ");
            scanf("%d",&n);
             if(n == -1){
            break;
            }
            struct Nodo *nuevo = crearNodo(n);
            insertarFinal(lista, nuevo);
        }else if(opcion==2){
            nuevo=desencolar(lista);
            insertarFinal(ejecutando,nuevo);
        }else if(opcion==3){
            nuevo=desencolar(ejecutando);
            insertarFinal(terminados,nuevo);
        } else if(opcion==4){

            printf("PID a matar: \n");
            scanf("%d", &n);
            nuevo = mataPID(lista, n);
            if(nuevo == NULL) {
                nuevo=desencolar(ejecutando);
                if(nuevo == NULL) {
                    printf("El proceso asociado al PID no existe\n");
                }
                insertarFinal(terminados,nuevo); 
            }

        }
        printf("listos:\n");
        imprimirLista(lista);
        printf("Ejecutando:\n");
        imprimirLista(ejecutando);
        printf("Terminados:\n");
        imprimirLista(terminados);
    }

    return 0;
}