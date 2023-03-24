#include "stdio.h"
#include "stdint.h"
#include "../process/helperfunc.h"
#include "vga.h"
#include "font.h"

char *splash[][2] = {
		{"\x1b[91m", " oooooo    oooooo        MMM         ooooo    oooooo                       oMMMMMM      "},
		{"\x1b[93m", " MMMMMM    MMMMMM        °°°         MMMMMo   MMMMMM                       °°°oMMM      "},
		{"\x1b[33m", "   MMMM    MMM°                      °MMMMMo    °MMM                           MMM      "},
		{"\x1b[91m", "    °MMMooMMM°       oMMMMMM          MMM°MMM    MMM  oMMMMM   oMMMMM          MMM      "},
		{"\x1b[95m", "      °MMMM°          °°°MMM          MMM °MMMo  MMM   °°MMM    °°MMM          MMM      "},
		{"\x1b[94m", "      MMMMMo             MMM          MMM   MMMo MMM     MMM      MMM          MMM      "},
		{"\x1b[91m", "    oMMM°°MMMo           MMM          MMM    MMMMMMM     MMM      MMM          MMM      "},
		{"\x1b[33m", "  oMMM°    °MMMo         MMM          MMM     °MMMMM     MMMo    oMMM          MMM      "},
		{"\x1b[93m", "oMMMMMMo  oMMMMMMo  MMMMMMMMMMMMMM  MMMMMMMM   °MMMM     °MMMMMMMMMMMMM   MMMMMMMMMMMMMM"},
		{"\x1b[91m", "YYYYYYYY  YYYYYYYY  YYYYYYYYYYYYYY  YYYYYYYY     YYY        YYYYY YYYY    YYYYYYYYYYYYYY"},
		{"\x1b[0m",  "= Initializating XiNul, Ensimag' educational RISC-V proto-os ="}
};


void splash_screen(void)
{
	for (int i = 0; i < 11; i++)
		printf("%s%s\n", splash[i][0], splash[i][1]);
}

void splash_vga_screen(void)
{
	for (int i = 0; i < 1280 * 768; i++){	
		*(volatile uint32_t *) ((uint32_t *) 0x80000000 + i) = 0x00000000;
	}
	uint32_t color[] = {0x00ff0000, 0x00ffff00, 0x00ffff00, 0x00ff0000, 0x00ff00ff, 0x000000ff, 0x00ff0000, 0x00ffff00,
	                    0x00ffff00, 0x00ff0000, 0x0000ff00};
	for (int y = 0; y < 11; y++)
	{
		uint32_t delta = 0;
		for (int x = 0; splash[y][1][x] != 0; x++)
			if(splash[y][1][x] != 0xc2)
				render(font8x8_basic[(int) splash[y][1][x]], 100 + (x - delta) * 8, 100 + y * 12, color[y]);
			else
				delta++;
	}
}


