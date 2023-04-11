/* VGA fonts */

#include "stdint.h"
#include "../process/helperfunc.h"
#include "font.h"
#include "vga.h"

void render(char *bitmap, uint32_t x, uint32_t y, uint32_t color)
{
	#ifdef VIRTMACHINE
	#else
	uint32_t dx, dy;
		for (dy = 0; dy < 8; dy++)
		{
			for (dx = 0; dx < 8; dx++)
			{
				if(bitmap[dy] & 1 << dx){
					*(volatile uint32_t *) ((uint32_t *) 0x80000000 + (y + dy) * 1280 + x + dx) = color;
				}
			}
		}
	#endif
}

