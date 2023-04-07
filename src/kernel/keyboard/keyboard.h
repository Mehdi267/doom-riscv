#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#define BSH 8
#define BS 127
#define R 13 //retour chariot
#define DL 21 //delete line
#define DW 23 //delete word
#define HT 9 //tab
#define LF 10 //next line, col 0
#define CR 13 //Déplace le curseur sur la ligne actuelle, colonne 0

void handle_keyboard_interrupt();

#endif