
#include <stdio.h>
#include <stdlib.h>
#include "syscall.h"
#include "string.h"

void* input_page;
int events_handler();
#define GAP_EVENT 250 
#define NO_EVENT	0xff
#define MAX_EVENTS 100

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
typedef enum {
  ev_keydown,
  ev_keyup
} evtype_t;

typedef struct event_com {
  int event;
  evtype_t press_type;
  unsigned char beingused;
  int next_event_id;
  int before_event_id;
  struct event_com* next;
} event_com;

typedef struct page_struct{
  int write_mutex;
  int reserved;
  int nb_events_in;
  int event_head_id;
  int event_tail_id;
  event_com events[MAX_EVENTS+1];
} page_struct;

typedef struct event_queue {
  event_com* head;
  event_com* tail;
  int mutex;
} event_queue;

event_queue* input_queue;

long get_cur_time_us() {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return tp.tv_usec;
}

void init_event_queue(event_queue* queue) {
  queue->head = NULL;
  queue->tail = NULL;
  queue->mutex = 0;
}

int look_for_empty_slot(){
  page_struct* page = (page_struct*) input_page;
  for (int even = 0; even<MAX_EVENTS; even++){
    if (page->events[even].beingused == 0){
      return even;
    }
  }
}

void process_event(event_queue* queue) {
  page_struct* page = (page_struct*) input_page;
  if (page->nb_events_in == MAX_EVENTS){
    while (page->nb_events_in == MAX_EVENTS){
      sleep(500);
    }
  }
  if (queue->head != NULL) {
    event_com* com = queue->head;
    if (com == queue->tail){
      queue->tail = NULL;
    }
    queue->head = com->next;
    com->beingused = 1;
    wait(page->write_mutex);
    while (page->reserved == 1){sleep(500);}
    page->reserved == 0;
    assert(scount(page->write_mutex) == 0);
    // printf("[Event Manager]Got mutex\n");
    int slot = -1;
    if (page->nb_events_in == 0 && 
      page->event_head_id == -1 
      && page->event_tail_id == -1){
      page->event_head_id = 0;
      page->event_tail_id = 0;
      com->next_event_id = -1;
      com->before_event_id = -1;
      slot = 0;
    }
    else if (page->nb_events_in>0 &&
            page->event_head_id != -1 &&
            page->event_tail_id != -1){
      slot = look_for_empty_slot();
      page->events[page->event_tail_id].next_event_id = slot;
      com->before_event_id = page->event_tail_id;
      com->next_event_id = -1;
      page->event_tail_id = slot;
    }
    else{
      // printf("page->nb_events_in = %d, page->event_head_id = %d, page->event_tail_id = %d\n",
      // page->nb_events_in,
      // page->event_head_id,
      // page->event_tail_id);
      // printf("Soemthing went wrong\n");
      assert(0);
    }
    page->nb_events_in++;
    memcpy(&page->events[slot], com, sizeof(event_com));
    // printf("Processing event: %d (%s)\n", com->event, (com->press_type == ev_keydown) ? "KeyDown" : "KeyUp");
    page->reserved = 0;
    signal(page->write_mutex);
    // printf("[Event Manager]Release mutex\n");
    free(com);
  }
}

void enqueue_event(event_queue* queue, int event, evtype_t press_type) {
  event_com* new_event = (event_com*)malloc(sizeof(event_com));
  if (new_event == NULL) {
    // fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }
  new_event->event = event;
  new_event->press_type = press_type;
  new_event->next = NULL;
  if (queue->tail == NULL) {
    // The queue is empty
    queue->head = new_event;
    queue->tail = new_event;
  } else {
    // Append to the tail
    queue->tail->next = new_event;
    queue->tail = new_event;
  }
  process_event(queue);
}

int old_event = NO_EVENT;
long time_snap_shot_ms = 0;

void save_input(int charv) {
  long cur_time_us = get_cur_time_us();
  if (old_event != NO_EVENT && (charv != old_event || cur_time_us - time_snap_shot_ms > GAP_EVENT)) {
    enqueue_event(input_queue, old_event, ev_keyup);
    // printf("[Release]Enqueued KeyUp event: %d\n", old_event);
  }

  old_event = charv;
  time_snap_shot_ms = cur_time_us;
  enqueue_event(input_queue, charv, ev_keydown);
  // printf("[Press]Enqueued KeyDown event: %d\n", charv);
}

int events_handler() {
  input_page = shm_create("doom_events");
  if (input_page == 0){
    return -1;  
  }
  memset(input_page, 0, 4096);
  set_in_mode(getpid(), RAW_INPUT);
  time_snap_shot_ms = get_cur_time_us();
  input_queue = (event_queue*) malloc(sizeof(event_queue));
  init_event_queue(input_queue);
  page_struct* page = (page_struct*) input_page;
  page->write_mutex = screate(1);
  page->nb_events_in = 0;
  page->event_head_id = -1;
  page->event_tail_id = -1;
  assert(sizeof(page_struct)<4096);
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
            // printf("Up arrow key pressed\n");
            save_input(KEY_UPARROW);
            break;
          case 'B':
            // printf("Down arrow key pressed\n");
            save_input(KEY_DOWNARROW);
            break;
          case 'C':
            // printf("Right arrow key pressed\n");
            save_input(KEY_RIGHTARROW);
            break;
          case 'D':
            // printf("Left arrow key pressed\n");
            save_input(KEY_LEFTARROW);
            break;
          case 49:
            // printf("in 49\n");
            cons_read(&c, 1);
            switch (c){
              case 53:
                // printf("F5 detected\n");
                save_input(KEY_F5);
                break;
              case 55:
                // printf("F6 detected\n");
                save_input(KEY_F6);
                break;
              case 56:
                // printf("F7 detected\n");
                save_input(KEY_F7);
                break;
              case 57:
                // printf("F8 detected\n");
                save_input(KEY_F8);
                break;
              default:
                break;
            }
            cons_read(&c, 1);
            break;
          case 50:
            // printf("in 50\n");
            cons_read(&c, 1);
            switch (c){
              case 48:
                // printf("F9 detected\n");
                save_input(KEY_F9);
                break;
              case 49:
                // printf("F10 detected\n");
                save_input(KEY_F10);
                break;
              case 50:
                // printf("F11 detected\n");
                save_input(KEY_F11);
                break;
              case 52:
                // printf("F12 detected\n");
                save_input(KEY_F12);
                break;
              default:
                break;
            }
            cons_read(&c, 1);
            break;
          // Add cases for other special keys here
          default:
            // printf("Unknown escape sequence: %s\n", c);
            break;
        }
      } else if (c == 'O'){
        cons_read(&c, 1);
        switch (c)
        {
          case 'P':
            // printf("F1 detected\n");
            save_input(KEY_F1);
            break;
          case 'Q':
            // printf("F2 detected\n");
            save_input(KEY_F2);
            break;
          case 'R':
            // printf("F3 detected\n");
            save_input(KEY_F3);
            break;
          case 'S':
            // printf("F4 detected\n");
            save_input(KEY_F4);
            break; 
          default:
            // printf("O detected but not treated\n");
            break;
        }
      }else{
        // Escape caracter detected, but sequence not treated
        // printf("Escape character: %c\n", c);
        cons_read(&c, 1);
        // printf("next char%c\n", c);
      }
    } 
    //Special temp bindings since some characters
    //cannot be read from console
    else if (c == 'a'|| c == 'A'){
      // printf("KEY_RCTRL \n", c);
      save_input(KEY_RCTRL);
    }
    else if (c == 'z'|| c == 'Z'){
      // printf("KEY_RCTRL \n", c);
      save_input(KEY_RALT);
    } 
    else if (c == 'p'|| c == 'P'){
      // printf("KEY_Escape \n", c);
      save_input(KEY_ESCAPE);
    } 
    else {
     	if (c >= 'A' && c <= 'Z'){
	      c = c - 'A' + 'a';
      }
      // printf("Saving %d\n", c);
      // Regular character; store it in the buffer
      save_input(c); 
    }
  }

  return 0;
}
