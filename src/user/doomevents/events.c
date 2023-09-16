#include <stdio.h>
#include <stdlib.h>
#include "syscall.h"
#include "../ulib/ufunc.h"
short input[512];
void* input_page;
int old_event;
long time_snap_shot_ms = 0;
int events_handler();
#define GAP_EVENT 500 

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

typedef enum
{
    ev_keydown,
    ev_keyup,
    //Not usd
    ev_mouse,
    ev_joystick
} evtype_t;

typedef struct event_com{
  int mutex_sem;
  int event;
  evtype_t press_type;
} event_com;

long get_cur_time_us(){
  struct timeval	tp;
  struct timezone	tzp;
  gettimeofday(&tp, &tzp);
  return tp.tv_usec;
}

void save_input(int charv){
  long cur_time_us = get_cur_time_us();
  if (old_event != NO_EVENT && 
    (charv != old_event ||
     cur_time_us - time_snap_shot_ms > GAP_EVENT)){
    event_com* com = (event_com*) input_page;
    com->event = old_event;
    com->press_type = ev_keyup;
    printf("======\n");
    printf("[Release]Semaphore going into wait mode\n");
    wait(com->mutex_sem);
    printf("[Release]Semaphore awaken, event released\n");
  }
  old_event = charv;
  time_snap_shot_ms = get_cur_time_us();
  event_com* com = (event_com*) input_page;
  com->event = charv;
  com->press_type = ev_keydown;
  printf("======\n");
  printf("[Press]Semaphore going into wait mode\n");
  wait(com->mutex_sem);
  com->event = NO_EVENT;
  printf("[Press]I have been awaken\n");
}

int events_handler() {
  input_page = shm_create("doom_events");
  if (input_page == 0){
    return -1;  
  }
  event_com* com = (event_com*) input_page;
  com->event = NO_EVENT;
  old_event = com->event; 
  time_snap_shot_ms = get_cur_time_us();
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
            printf("Up arrow key pressed\n");
            save_input(KEY_UPARROW);
            break;
          case 'B':
            printf("Down arrow key pressed\n");
            save_input(KEY_DOWNARROW);
            break;
          case 'C':
            printf("Right arrow key pressed\n");
            save_input(KEY_RIGHTARROW);
            break;
          case 'D':
            printf("Left arrow key pressed\n");
            save_input(KEY_LEFTARROW);
            break;
          case 49:
            printf("in 49\n");
            cons_read(&c, 1);
            switch (c){
              case 53:
                printf("F5 detected\n");
                save_input(KEY_F5);
                break;
              case 55:
                printf("F6 detected\n");
                save_input(KEY_F6);
                break;
              case 56:
                printf("F7 detected\n");
                save_input(KEY_F7);
                break;
              case 57:
                printf("F8 detected\n");
                save_input(KEY_F8);
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
                printf("F9 detected\n");
                save_input(KEY_F9);
                break;
              case 49:
                printf("F10 detected\n");
                save_input(KEY_F10);
                break;
              case 50:
                printf("F11 detected\n");
                save_input(KEY_F11);
                break;
              case 52:
                printf("F12 detected\n");
                save_input(KEY_F12);
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
            printf("F1 detected\n");
            save_input(KEY_F1);
            break;
          case 'Q':
            printf("F2 detected\n");
            save_input(KEY_F2);
            break;
          case 'R':
            printf("F3 detected\n");
            save_input(KEY_F3);
            break;
          case 'S':
            printf("F4 detected\n");
            save_input(KEY_F4);
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
