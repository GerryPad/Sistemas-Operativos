#include <stdio.h>
#include <curses.h>
#include "ncurses.h"

void imprimir_registros(int renglon, char *instruccion){
    mvprintw(3, 2, "%-8s %-15s %-6s %-6s %-6s %-6s", 
        "PC", "IR", "EAX", "EBX", "ECX", "EDX");
    move(5,2);
    clrtoeol();
    mvprintw(5, 2, "%-8d %-15s %-6d %-6d %-6d %-6d", 
        renglon, 
        instruccion, 
        registros[0].valor, 
        registros[1].valor, 
        registros[2].valor, 
        registros[3].valor  
    );
    refresh();
}

void limpia_lineas() {
    move(22,2); clrtoeol();
    move(23,2); clrtoeol();
    move(24,2); clrtoeol();
    move(25,2); clrtoeol();
    move(20,2); clrtoeol();
    refresh();
}