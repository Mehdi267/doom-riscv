//#include "../tests/sys_api.h"
#include <stdio.h>
#include "riscv.h" //to use wfi


void cons_echo(int on){
    if(!on) console_dev->echo = false;
    else console_dev->echo = true;
}

void cons_flush(){
    console_dev->top_ptr = 0;
    console_dev->putchar('\n');
}

/**
Si length est nul, cette fonction retourne 0.
Sinon, elle attend que l'utilisateur ait tapé une ligne complète terminée par le caractère 13
puis transfère dans le tableau string soit la ligne entière (caractère 13 non compris), 
si sa longueur est strictement inférieure à length, soit les length premiers caractères de la ligne. 
Finalement, la fonction retourne à l'appelant le nombre de caractères effectivement transmis. 
Les caractères frappés et non prélevés restent dans le tampon associé au clavier 
et seront prélevés aux appels suivants. Le caractère de fin de ligne (13) n'est jamais transmis à l'appelant.
Lorsque length est exactement égal au nombre de caractères frappés, 
fin de ligne non comprise, le marqueur de fin de ligne reste dans le tampon. 
Le prochain appel récupèrera une ligne vide.
*/
unsigned long cons_read(char *string, unsigned long length){
    //requires string -> string + length valid
    if(!length) return 0;
    //lets chars get stored in buffer
    console_dev->ignore = false;
    //make sure buffer is empty => any earlier strike is ignored
    cons_flush();
    //wait until buffer contains n char, or last char is a EOL
    while(console_dev->top_ptr != length && (console_dev->top_ptr == 0 || console_dev->buffer[console_dev->top_ptr-1] != EOL)){
        wfi();
    }
    printf("%i", console_dev->top_ptr);
    console_dev->ignore = true;
    return console_dev->top_ptr;
}