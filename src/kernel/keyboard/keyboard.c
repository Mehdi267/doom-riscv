#include "../drivers/console.h"
#include <stdio.h>
#include "keyboard.h"
#include "queue.h"
#include "../process/scheduler.h"
#include "../process/process.h"
#include <assert.h>

bool is_printable(char c){
    return ((int)c < 127) && ((int)c > 31);
}

void delete_last(){
    //TODO : gérer le cas des TAB
    console_dev->buffer[console_dev->top_ptr-1] = 0;
    console_dev->top_ptr --;
    console_dev->putchar(BSH);
    console_dev->putchar(' ');
    console_dev->putchar(BSH);
}

void handle_keyboard_interrupt(){
    // puts("keyboard interrupt");
    //Dans le pilote de clavier, à chaque fois qu'une touche est appuyée et qu'une séquence de caractères est entrée en tampon, elle doit être affichée en même temps à l'écran sauf si le mode d'écho a été désactivé avec [cons_echo](#cons_echo-configuration-du-terminal).;
    char c = kgetchar();
    if(is_printable(c)){
        kaddtobuffer(c);
        if(console_dev->echo) console_dev->putchar(c);//echo
    }
    else if((int)c == HT){
        //put char space to erase what should be erased
        int nb_spaces = 8 - (console_dev->top_ptr % 8);
        for(int i = 0; i < nb_spaces; i++){
            console_dev->putchar(' ');
            kaddtobuffer(' ');
        }
    }
    else if(c == BS || c ==  BSH){
        if(console_dev->top_ptr != 0 && console_dev->buffer[console_dev->top_ptr-1] != '\r'){
            delete_last();
        }
    }
    else if(c == LF){
        kaddtobuffer(c);
        if(console_dev->echo) console_dev->putchar('\n');//putchar(R) only goes to beginning of line
    }
    else if(c == DL){
        //deletes current line
        while(console_dev->top_ptr != 0 && console_dev->buffer[console_dev->top_ptr-1] != '\r'){
            delete_last();
        }
    }
    else if(c == DW){
        //deletes current word
        while(console_dev->top_ptr != 0 && console_dev->buffer[console_dev->top_ptr-1] == ' ')
            if(console_dev->echo) delete_last();
        while(console_dev->top_ptr != 0 && console_dev->buffer[console_dev->top_ptr-1] != ' ')
            if(console_dev->echo) delete_last();
    }
    else if(c == CR){
        kaddtobuffer('\n');
        if(console_dev->echo) console_dev->putchar('\n');//echo
    }
    //on signale au scheduler qu'il doit débloquer un processus
    process *next = get_peek_element_queue_wrapper(IO_QUEUE);
    // printf("next ADDRESS %p\n", next);
    if (next && next->prio > getprio(getpid())){
      printf("proc is not null %s  pid %d \n", next->process_name, next->pid);
      pop_element_queue_wrapper(IO_QUEUE);
      next->state = ACTIVATABLE;
      add_process_to_queue_wrapper(next, ACTIVATABLE_QUEUE);
      printf("writing car %c \n", console_dev->buffer[console_dev->top_ptr-1]);
    }
}