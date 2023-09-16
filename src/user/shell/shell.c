#include "syscall.h"
#include "../ulib/ufunc.h"
#include "stdio.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>

long int ret = 0;


void dis(const char *filename) {
    int fd = open(filename, O_RDONLY, 0);
    if (fd == -1) {
        printf("Error opening file\n");
        return;
    }

    char buffer[1024]; // Buffer to hold read data
    while (my_fgets(buffer, sizeof(buffer), fd) != NULL) {
        printf("%s", buffer); // Print each line read from the file
    }

    close(fd); // Close the file when done
}

int dis_bin(char *prog) {
  const char *filename = prog;
  int fd = open(filename, O_RDONLY, 0);
  if (fd == -1) {
    printf("Error opening file\n");
    return 1;
  }

  char buffer[4]; // A RISC-V 32-bit instruction is 4 bytes
  char *result;
  int i = 0; 
  
  while ((result = my_fgets(buffer, sizeof(buffer), fd)) != NULL) {
    printf("%08x\n", *(unsigned int *)buffer); // Print 32-bit value as hex
    i++;
  }
  
  printf("Number of instructions: %d\n", i);
  
  close(fd);

  return 0;
}


/**
 * @brief if cmd is a builtin, executes the builtin and returns 0, returns 1 if not
 */
int builtin_cmd(char *cmd) {
  if (!strcmp(cmd, "echo $?")) {
  printf("value: %ld\n", ret);
  return 0;
  }
  return 1;
}

#define SCREENWIDTH 320
#define SCREENHEIGHT 200


struct Pixel {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t alpha;
};
void display_test() {
  printf("size of screen %ld\n", SCREENWIDTH*SCREENHEIGHT*sizeof(struct Pixel));
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

int main() {
  //stdin
  if (open("/dev/terminal", O_RDONLY, 0) == 0){
    // assert(open("/dev/terminal", O_RDONLY, 0) == 0);
    //stdout
    assert(open("/dev/terminal", O_WRONLY, 0) == 1);
    //stderr
    assert(dup2(1, 2) == 2);
    open(".doomrc", O_CREAT, 0);
    // #ifdef VIRTMACHINE
    //Loads all of the elfs into the disk
    // ld_progs_into_disk();
    // #endif
  }
  // display_test();
  char cmd[21];
  cmd[20] = 0;
  #define CURR_DIR_SIZE 50
  char current_dir[CURR_DIR_SIZE];
  int pid;
  char mkdirprog[] = "mkdir";
  char lsprog[] = "ls";
  char cdprog[] = "cd";
  char rmdirprog[] = "rmdir";
  char unlinkprog[] = "unlink";
  char disprog[] = "dis";
  char dis_binprog[] = "dis_bin";
  char exitshell[] = "exit";
  char voidsysprog[] = "voidsys";
  while (1) {
  printf("shell$");
  if (getcwd(current_dir, CURR_DIR_SIZE) != 0){
    printf("%s", current_dir);
  }
  printf("#");
  // my_fgets(cmd, 20, 0);
  cons_read(cmd, 20);
  if (memcmp(cmd, "ls", strlen(lsprog)) == 0){
    char curr_path[] = ".";
    print_dir_elements(curr_path);
  }
  else if (memcmp(cmd, "mkdir", strlen(mkdirprog)) == 0){
    char* curr_path = cmd + strlen(mkdirprog) + 1;
    printf("curr_path = %s\n", curr_path);
    mkdir(curr_path, 0);
  }
  else if (memcmp(cmd, "cd", strlen(cdprog)) == 0){
    char* curr_path = cmd + strlen(cdprog) + 1;
    printf("curr_path = %s\n", curr_path);
    chdir(curr_path);
  }
  else if (memcmp(cmd, "rmdir", strlen(rmdirprog)) == 0){
    char* curr_path = cmd + strlen(rmdirprog) + 1;
    printf("curr_path = %s\n", curr_path);
    rmdir(curr_path);
  }
  else if (memcmp(cmd, "unlink", strlen(unlinkprog)) == 0){
    char* curr_path = cmd + strlen(unlinkprog) + 1;
    printf("curr_path = %s\n", curr_path);
    unlink(curr_path);
  }
  else if (memcmp(cmd, "dis", strlen(disprog)) == 0){
    char* curr_path = cmd + strlen(disprog) + 1;
    dis(curr_path);
  }
  else if (memcmp(cmd, "dis_bin", strlen(dis_binprog)) == 0){
    char* curr_path = cmd + strlen(dis_binprog) + 1;
    dis_bin(curr_path);
  }
  else if (memcmp(cmd, "exit", strlen(exitshell)) == 0){
    exit(0);
  }
  else if (memcmp(cmd, "voidsys", strlen(voidsysprog)) == 0){
    void_call();
  }
  else if (builtin_cmd(cmd) != 0) {
    pid = start(cmd, 800000, 128, NULL);
    if (pid == -1) {
    printf("shell: program not found: %s\n", cmd);
    ret = -1;
    } else {
    waitpid_old(pid, &ret);
    }
  }
  memset(cmd, 0, 20);
  }
}
