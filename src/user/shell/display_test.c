#include "syscall.h"
#include "stdio.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>

#define SCREENWIDTH 320
#define SCREENHEIGHT 200

struct Pixel {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t alpha;
};

void display_test() {
  printf("Size of screen is equal to %ld\n", SCREENWIDTH*SCREENHEIGHT*sizeof(struct Pixel));
  void* frame = malloc(SCREENWIDTH*SCREENHEIGHT*sizeof(struct Pixel)); 
  struct Pixel* data = (struct Pixel*)(frame);
  // Initialize pixel colors
  unsigned char orientation = 6;
  while (1) {
    orientation += 40;
    for (int x = 0; x < SCREENWIDTH; x++) {
      for (int y = 0; y < SCREENHEIGHT; y++) {
        if (x < SCREENWIDTH / 2 && y < SCREENHEIGHT / 2) {
          // Top-left corner: Red
          (data + y * SCREENWIDTH + x)->red = 255;
          (data + y * SCREENWIDTH + x)->green = 0;
          (data + y * SCREENWIDTH + x)->blue = orientation;
        } else if (x >= SCREENWIDTH / 2 && y < SCREENHEIGHT / 2) {
          // Top-right corner: Blue
          (data + y * SCREENWIDTH + x)->red = 0;
          (data + y * SCREENWIDTH + x)->green = orientation;
          (data + y * SCREENWIDTH + x)->blue = 255;
        } else if (x < SCREENWIDTH / 2 && y >= SCREENHEIGHT / 2) {
          // Bottom-left corner: Green
          (data + y * SCREENWIDTH + x)->red = 0;
          (data + y * SCREENWIDTH + x)->green = 255;
          (data + y * SCREENWIDTH + x)->blue = orientation;
        } else {
          // Remaining corner: Any color (e.g., yellow)
          (data + y * SCREENWIDTH + x)->red = 255;
          (data + y * SCREENWIDTH + x)->green = 255+orientation;
          (data + y * SCREENWIDTH + x)->blue = orientation;
        }
        (data + y * SCREENWIDTH + x)->alpha = 255; // Alpha component (transparency)
      }
    }
    // Ensure that orientation doesn't go beyond 255
    if (orientation > 255) {
        orientation = 0;
    }
    char in;
    cons_read(&in, 1);
    upd_data_display(data, 0, 0, SCREENWIDTH, SCREENHEIGHT);
    // sleep(1); // Sleep for 1 second (adjust as needed)
  }
  free(frame);
}
