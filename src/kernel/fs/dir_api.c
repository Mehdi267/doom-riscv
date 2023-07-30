#include "dir_api.h"
#include "inode.h"
#include "inode_util.h"
#include "fs.h"
#include "../process/process.h"
#include "../process/helperfunc.h"
#include "string.h"
#include "../process/fs_bridge.h"


static void navigate_to_parent_directory(char* current_directory) {
  if (current_directory == NULL || strcmp(current_directory, "/") == 0) {
      // Do nothing if the current directory is NULL or already the root directory
      return;
  }
  char *last_separator = strrchr(current_directory, '/');
  ssize_t old_len = strlen(current_directory);
  if (last_separator != NULL) {
    printf("current_directory = %s space = %ld\n", current_directory, (unsigned long) (last_separator-current_directory));
    // Truncate the path at the last separator to go up to the parent directory
    if (current_directory == 
        (char*) current_directory + (last_separator-current_directory)){
      memset(current_directory + (last_separator-current_directory) + 1, 0,
           old_len-(last_separator-current_directory) - 1);
    }else{
      memset(current_directory + (last_separator-current_directory), 0,
           old_len-(last_separator-current_directory) - 1);
    }
    printf("current_directory = %s len = %ld\n",
         current_directory, strlen(current_directory));
  }
}

static int navigate_to_directory(char** main_dir, char* addition){
  if (main_dir == 0 || *main_dir == 0 || addition == 0){
    return -1;
  }
  int len_main_old = strlen(*main_dir); 
  int len_add = strlen(addition);
  if (len_main_old == 1 && is_absolute_directory(*main_dir)){
    char * place_holder = (char *) malloc(len_main_old + len_add + 1);
    if (place_holder == 0){
      return -1;
    }
    memset(place_holder, 0, len_main_old + len_add );
    memcpy(place_holder, *main_dir, len_main_old);
    memcpy(place_holder + len_main_old, addition, len_add);
    place_holder[len_main_old + len_add] = '\0';
    free(*main_dir);
    *main_dir = place_holder;
  } else{
    char * place_holder = (char *) malloc(len_main_old + 1 + len_add + 1);
    if (place_holder == 0){
      return -1;
    }
    memset(place_holder, 0, len_main_old + 1 + len_add + 1);
    memcpy(place_holder, *main_dir, len_main_old);
    char sep[] = "/";
    memcpy(place_holder + len_main_old, sep, 1);
    memcpy(place_holder + len_main_old + 1, addition, len_add);
    place_holder[len_main_old + 1 + len_add] = '\0';
    free(*main_dir);
    *main_dir = place_holder;
  }
  return 0;
}

char * get_final_string(const char* new_directory){
  if (new_directory == 0){
    return 0;
  }
  // Check if the path is absolute or relative and set the starting directory accordingly
  char* temp_directory = 0;
  if (is_absolute_directory(new_directory)){
    temp_directory = (char *)malloc(get_root_dir_name_size()); 
    memcpy(temp_directory, get_root_dir_name(), get_root_dir_name_size()); 
  } else{
    temp_directory = (char *)malloc(get_current_dir_name_size()); //WRONG !!!!
    memcpy(temp_directory, get_current_dir_name(), get_current_dir_name_size()); 
  }
  path_fs* path_data = extract_files(new_directory);
  if (path_data == 0){
    return 0;
  }
  // Process each component of the path
  for (int i = 0; i < path_data->nb_files; i++) {
    if (strcmp(path_data->files[i], ".") == 0) {
        continue;
    } else if (strcmp(path_data->files[i], "..") == 0) {
        // Handle ".." component (parent directory)
        navigate_to_parent_directory(temp_directory);
    } else {
      if (navigate_to_directory(&temp_directory, path_data->files[i]) < 0){
        free(temp_directory);
        free_path_fs(path_data);
      }
    }
  }
  printf("final temp = %s len = %ld\n",
       temp_directory, strlen(temp_directory));
  free_path_fs(path_data);
  char * final_string = (char*)malloc(strlen(temp_directory)+1);
  memcpy(final_string, temp_directory, strlen(temp_directory));
  final_string[strlen(temp_directory)] = '\0';
  printf("Final string = %s\n", final_string);
  free(temp_directory);
  return final_string;
}

int chdir(const char *new_directory){
  if (new_directory == 0){
    return -1;
  }
  char* final_string = get_final_string(new_directory);
  if (final_string == 0){
    return -1;
  }
  inode_t* dir_inode = walk_and_get(final_string, 0);
  if (dir_inode == 0){
    free(final_string);
    return -1;
  }
  if (dir_inode->i_mode != EXT2_S_IFDIR){
    free(final_string);
    PRINT_RED("Inode is not a directory, mode = %d\n", 
          dir_inode->i_mode);
    return -1;
  }
  if (set_current_dir(final_string, strlen(final_string)+1,
                   dir_inode)<0){
    return -1;
  }
  //Not very usefull
  sync_all();
  return 0;
}


int mkdir(const char *dir_name, mode_t mode){
  if (dir_name == 0){
    return -1;
  }
  char* final_string = get_final_string(dir_name);
  if (final_string == 0){
    return -1;
  }
  printf("final_string = %s\n", final_string);
  path_fs* path_data = extract_files(final_string);
  if (path_data == 0){
    return -1;
  }
  inode_t* parent_dir = walk_and_get(final_string, 1);
  if (parent_dir == 0){
    printf("cannot create directory \'%s\': No such file or directory", 
        dir_name);
    free(final_string);
    free_path_fs(path_data);
  }
  inode_t* dir_inode = alloc_inode(); 
  if (dir_inode != 0){
    dir_inode->i_mode = EXT2_S_IFDIR;
    dir_inode->i_links_count = 1;
    if (add_inode_directory(parent_dir, 
          get_inode_number(dir_inode), 
          EXT2_FT_DIR,
          path_data->files[path_data->nb_files -1],
          strlen(path_data->files[path_data->nb_files -1]))<0){
      PRINT_RED("Folder already exists or something went wrong\n");
      free_inode(dir_inode, get_inode_number(dir_inode));
      free(final_string);
      free_path_fs(path_data);
      return -1;
    }
    if (add_dot_directories(dir_inode, parent_dir)<0){
      PRINT_RED("Couldn't add dot directories\n");
      free(final_string);
      free_path_fs(path_data);
      return -1;
    }
    sync_all();
  }
  free(final_string);
  free_path_fs(path_data);
  return 0;
}

char *getcwd(char *buf, size_t size){
  if (buf == 0 || size <= 0){
    return 0;
  }
  if (size < get_current_dir_name_size()){
    return 0;
  }
  char* current_dir_name = get_current_dir_name();
  if (current_dir_name == 0){
    return 0;
  }
  memcpy(buf, current_dir_name, get_current_dir_name_size());
  return buf;
}

int rmdir(const char *path){
  if (path == 0){
    return -1;
  }
  if (strcmp(path, "/") == 0){
    PRINT_RED("Cannot delete root directory\n");    
  }
  char* final_string = get_final_string(path);
  if (final_string == 0){
    return -1;
  }
  inode_t* dir_inode = walk_and_get(final_string, 0);
  inode_t* parent_dir = walk_and_get(final_string, 1);
  if (dir_inode == 0 || parent_dir == 0){
    free(final_string);
    return -1;
  }
  if (dir_inode->i_mode != EXT2_S_IFDIR){
    free(final_string);
    PRINT_RED("Inode is not a directory, mode = %d\n", 
          dir_inode->i_mode);
    return -1;
  }
  path_fs* path_data = extract_files(final_string);
  if (path_data == 0){
    free(final_string);
    return -1;
  }
  if (dir_inode->i_size == BASIC_DOT_DIR_SIZE){
    if (remove_inode_dir(parent_dir, path_data->files[path_data->nb_files -1],
      strlen(path_data->files[path_data->nb_files -1]))<0){
      PRINT_RED("Remove dir from folder failed\n");
      free(final_string);
      free_path_fs(path_data);
      return -1;
    }
    if (free_inode(dir_inode, get_inode_number(dir_inode))<0){
      free(final_string);
      free_path_fs(path_data);
      return -1;
    }
  } else{
    free_path_fs(path_data);
    PRINT_RED("Dir is not empty cannot delete it, size = %d\n", 
          dir_inode->i_size);
  }
  return 0;
}