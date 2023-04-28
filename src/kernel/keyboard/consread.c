//#include "../tests/sys_api.h"
#include <stdio.h>
#include "riscv.h" //to use wfi
#include "stdbool.h"
#include "../drivers/console.h"
#include "../process/scheduler.h"
#include "encoding.h"
#include <assert.h>
#include <queue.h>
#include "../process/helperfunc.h"
#include <string.h>

void cons_echo(int on){
    //active (on!=0) ou desactive (on=0) l'echo sur la console
    if(!on) console_dev->echo = false;
    else console_dev->echo = true;
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
    //puts("cons read called");
    cons_echo(1);
    if(!length) return 0;
    //lets chars get stored in buffer
    console_dev->ignore = false;
    process* proc = get_current_process();
    //wait until buffer contains n char, or last char is a EOL
    while(console_dev->top_ptr ==0 || console_dev->buffer[console_dev->top_ptr-1] != '\n'){//on attend que l'utilisateur finisse sa ligne
        printf("reading caracters %c \n", console_dev->buffer[console_dev->top_ptr-1]);
        proc->state = BLOCKEDIO;
        add_process_to_queue_wrapper(proc, IO_QUEUE);
        scheduler();
    }
    //length of line (without \n) is equal to top_ptr
    unsigned long nb_char;
    if(console_dev->top_ptr < length){
        nb_char = console_dev->top_ptr - 1; //not taking EOL into account
        printf("%li\n", nb_char);
        memcpy(string, console_dev->buffer, nb_char); //EOL is not transmitted
        string[nb_char] = NULL;
        //in that case, we empty the buffer
        console_dev->top_ptr = 0;
        console_dev->buffer[0] = ' ';
    }
    else{
        nb_char = length;
        memcpy(string, console_dev->buffer, nb_char);
        string[nb_char] = NULL;
        //saving the rest of the buffer
        unsigned long to_save = console_dev->top_ptr - length;
        char saved[to_save];
        memcpy(saved, console_dev->buffer + length, to_save);
        memcpy(console_dev->buffer, saved, to_save);
        console_dev->top_ptr = to_save;
    }
    console_dev->ignore = true;
    return nb_char;
}