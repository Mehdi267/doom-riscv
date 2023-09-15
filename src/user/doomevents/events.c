#include <stdio.h>
#include <stdlib.h>
#include "syscall.h"
#include "../ulib/ufunc.h"
short input[512];
void* input_page;
int events_handler();

#define NO_EVENT	0xff
#define KEY_RIGHTARROW	0xae
#define KEY_LEFTARROW	0xac
#define KEY_UPARROW	0xad
#define KEY_DOWNARROW	0xaf
#define KEY_ESCAPE	27
#define KEY_ENTER	13
#define KEY_TAB		9
#define KEY_F1		(0x80+0x3b)
#define KEY_F2		(0x80+0x3c)
#define KEY_F3		(0x80+0x3d)
#define KEY_F4		(0x80+0x3e)
#define KEY_F5		(0x80+0x3f)
#define KEY_F6		(0x80+0x40)
#define KEY_F7		(0x80+0x41)
#define KEY_F8		(0x80+0x42)
#define KEY_F9		(0x80+0x43)
#define KEY_F10		(0x80+0x44)
#define KEY_F11		(0x80+0x57)
#define KEY_F12		(0x80+0x58)
#define KEY_BACKSPACE	127
#define KEY_PAUSE	0xff
#define KEY_EQUALS	0x3d
#define KEY_MINUS	0x2d
#define KEY_RSHIFT	(0x80+0x36)
#define KEY_RCTRL	(0x80+0x1d)
#define KEY_RALT	(0x80+0x38)

void main(){
  events_handler();
}

typedef struct event_com{
  int mutex_sem;
  int event;
} event_com;


void save_input(int charv){
  event_com* com = (event_com*) input_page;
  com->event = charv;
  printf("Semaphore going into wait mode\n");
  wait(com->mutex_sem);
  com->event = NO_EVENT;
  printf("I have been awaken\n");
}

int events_handler() {
  input_page = shm_create("doom_events");
  if (input_page == 0){
    return -1;  
  }
  event_com* com = (event_com*) input_page;
  com->event = NO_EVENT;
  com->mutex_sem = screate(0);
  void_call();
  int input_index = 0;  // Index in the input buffer
  char c;
  while (1) {
    cons_read(&c, 1);  // Read a character from the custom terminal
    if (c == 27) {  // Escape character
      cons_read(&c, 1);  // Read a character from the custom terminal
      if (c == '[') {
        // It's an escape sequence; continue reading
        cons_read(&c, 1);  
        // Process based on the detected key character
        switch (c) {
          case 'A':
            save_input(KEY_UPARROW);
            printf("Up arrow key pressed\n");
            break;
          case 'B':
            save_input(KEY_DOWNARROW);
            printf("Down arrow key pressed\n");
            break;
          case 'C':
            save_input(KEY_RIGHTARROW);
            printf("Right arrow key pressed\n");
            break;
          case 'D':
            save_input(KEY_LEFTARROW);
            printf("Left arrow key pressed\n");
            break;
          case 49:
            printf("in 49\n");
            cons_read(&c, 1);
            switch (c){
              case 53:
                save_input(KEY_F5);
                printf("F5 detected\n");
                break;
              case 55:
                save_input(KEY_F6);
                printf("F6 detected\n");
                break;
              case 56:
                save_input(KEY_F7);
                printf("F7 detected\n");
                break;
              case 57:
                save_input(KEY_F8);
                printf("F8 detected\n");
                break;
              default:
                break;
            }
            cons_read(&c, 1);
            break;
          case 50:
            printf("in 50\n");
            cons_read(&c, 1);
            switch (c){
              case 48:
                save_input(KEY_F9);
                printf("F9 detected\n");
                break;
              case 49:
                save_input(KEY_F10);
                printf("F10 detected\n");
                break;
              case 50:
                save_input(KEY_F11);
                printf("F11 detected\n");
                break;
              case 52:
                save_input(KEY_F12);
                printf("F12 detected\n");
                break;
              default:
                break;
            }
            cons_read(&c, 1);
            break;
          // Add cases for other special keys here
          default:
            printf("Unknown escape sequence: %s\n", input);
            break;
        }
      } else if (c == 'O'){
        cons_read(&c, 1);
        switch (c)
        {
          case 'P':
            save_input(KEY_F1);
            printf("F1 detected\n");
            break;
          case 'Q':
            save_input(KEY_F2);
            printf("F2 detected\n");
            break;
          case 'R':
            save_input(KEY_F3);
            printf("F3 detected\n");
            break;
          case 'S':
            save_input(KEY_F4);
            printf("F4 detected\n");
            break; 
          default:
            printf("O detected but not treated\n");
            break;
        }
      }else{
        // Escape caracter detected, but sequence not treated
        printf("Escape character: %c\n", c);
        cons_read(&c, 1);
        printf("next char%c\n", c);
      }
    } else {
     	if (c >= 'A' && c <= 'Z'){
	      c = c - 'A' + 'a';
      }
      printf("Saving %d\n", c);
      // Regular character; store it in the buffer
      save_input(c); 
    }
  }

  return 0;
}
