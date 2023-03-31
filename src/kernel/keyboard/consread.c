#include "../drivers/console.h"
#include <stdio.h>


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
int cons_read(char *string, unsigned long length){
    //requires string -> string + length valid
    if(!length) return 0;
    int n = 0;
    int c = -1;
    while(c < 0) c = kgetchar();
    string[0] = (char) c;
    while (c != EOL){
        c = -1;
        while(c < 0) c = kgetchar();
        n++;
        if(n < length && c != EOL) string[n] = (char) c; // Le caractère de fin de ligne (13) n'est jamais transmis à l'appelant.
    }
    return n;
}