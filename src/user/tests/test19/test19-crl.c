/*******************************************************************************
 * Test 19
 *
 * Test du clavier.
 ******************************************************************************/

#include "sysapi.h"

static void mega_cycles(int n) {
	unsigned long long t1, t2;
	int i = 0;

	t1 = csr_read(cycle);
	for (i = 0; i < n; i++) {
		do {
			test_it();
			t2 = csr_read(cycle);
		} while ((t2 - t1) < 1000000);
		t1 += 1000000;
	}
}

int main(void *arg) {
	char ch[101];
	int i;
	unsigned long nbr_of_chars;
	int pid1, pid2, pid3, pid4;

	(void) arg;

	printf("cons_read bloquant, entrez des caractères : ");
	nbr_of_chars = cons_read(ch, 100);
	ch[nbr_of_chars] = 0;
	printf("%lu chars : %s\n", nbr_of_chars, ch);
	assert(nbr_of_chars != 0);

	printf("Frappez une ligne de 5 caractères : ");
	nbr_of_chars = cons_read(ch, 5);
	ch[5] = 0;
	// On devrait avoir seulement 5 caractères. Le caractère '\r' est encore dans le buffer.
	printf("%lu chars : %s\n", nbr_of_chars, ch);
	assert(nbr_of_chars == 5);
	// A la prochaine lecture on ne devrait rien lire car on récupère le caractère de fin de ligne '\r'.
	nbr_of_chars = cons_read(ch, 5);
	assert(nbr_of_chars == 0);

	// Lecture inutile car on ne souhaite pas de caractère. On doit récupérer la main directtement.
	assert(cons_read(ch, 0) == 0);

	printf("\nCe  tests suivants supposent un tampon clavier de l'ordre  de 20 caractères.\n"
		   "Entrez des caractères tant qu'il apparaissent à l'écran, frappez encore quelques touches supplémentaires puis enter : ");
	nbr_of_chars = cons_read(ch, 100);
	ch[nbr_of_chars] = 0;
	printf("\n%lu chars : %s\n", nbr_of_chars, ch);
	if (nbr_of_chars < 20) {
		printf("Moins de 20 caractères ont été saisis. Test fail.\n");
		exit(1);
	}

	printf("\nEntrées sorties par anticipation, les caractère seront lus par 4 processus fils avec la même prio. Frappez 4 lignes en veillant a ne pas dépasser la taille du tampon clavier : ");
	for (i = 0; i < 80; i++) {
		mega_cycles(400);
		printf(".");
	}
	printf("Fini.\n");
	pid1 = start("cons_rd0", 4000, 129, (void *) 1);
	pid3 = start("cons_rd0", 4000, 129, (void *) 3);
	pid2 = start("cons_rd0", 4000, 129, (void *) 2);
	pid4 = start("cons_rd0", 4000, 129, (void *) 4);
	waitpid_old(pid2, 0);
	waitpid_old(pid3, 0);
	waitpid_old(pid1, 0);
	waitpid_old(pid4, 0);

	printf("Enfin on teste que dans le cas d'entrées bloquantes, les 4 processus fils seront servis dans l'ordre de leurs priorités. Entrez quatre lignes : ");
	pid4 = start("cons_rdN", 4000, 129, 0);
	pid1 = start("cons_rdN", 4000, 130, 0);
	pid3 = start("cons_rdN", 4000, 131, 0);
	pid2 = start("cons_rdN", 4000, 132, 0);
	waitpid_old(pid2, 0);
	waitpid_old(pid3, 0);
	waitpid_old(pid1, 0);
	waitpid_old(pid4, 0);

	return 0;
}
