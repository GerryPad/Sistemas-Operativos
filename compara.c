#include <stdio.h>
#include <string.h>

// Función para comprobar si el elemento existe
int existeElemento(const char *arreglo[], const char *buscado) {
    int i = 0;
    // El bucle se detiene cuando arreglo[i] es NULL
    while (arreglo[i] != NULL) {
        if (strcmp(arreglo[i], buscado) == 0) {
            return 1; // Encontrado
        }
        i++;
    }
    return 0; // No encontrado
}

int main() {
    // Arreglo de punteros char* terminado en NULL
    const char *miArreglo[] = {
        "hola",
        "mundo",
        "lenguaje",
        "c",
        NULL // Centinela indicando el fin
    };

    const char *elemento = "hola ";

    if (existeElemento(miArreglo, elemento)) {
        printf("El elemento '%s' existe.\n", elemento);
    } else {
        printf("El elemento '%s' no existe.\n", elemento);
    }

    return 0;
}
