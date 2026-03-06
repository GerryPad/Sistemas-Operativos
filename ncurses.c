#include <stdio.h>
#include <curses.h>
#include "ncurses.h"

void imprimir_registros(int renglon, char *instruccion){
    mvprintw(3, 2, "%-6s %-22s %-6s %-6s %-6s %-6s", 
        "PC", "IR", "EAX", "EBX", "ECX", "EDX");
    move(5,2);
    clrtoeol();
    mvprintw(5, 2, "%-6d  %-19s  %-6d  %-6d  %-6d  %-6d", 
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
    move(16,2); clrtoeol();
    move(8,2); clrtoeol();
    move(12,2); clrtoeol();
    move(10,2); clrtoeol();
    move(11,2); clrtoeol();
    move(17,2); clrtoeol();
    refresh();
}