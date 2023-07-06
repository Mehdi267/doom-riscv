#ifndef INODE_BLOCK 
#define INODE_BLOCK

#include <stdint.h>

typedef struct {
  uint16_t i_mode;             // File mode
  uint16_t i_uid;              // Owner user ID
  uint32_t i_size;             // File size in bytes
  uint32_t i_atime;            // Last access time
  uint32_t i_ctime;            // Creation time
  uint32_t i_mtime;            // Last modification time
  uint32_t i_dtime;            // Deletion time
  uint16_t i_gid;              // Owner group ID
  uint16_t i_links_count;      // Number of hard links
  uint32_t i_blocks;           // Number of blocks
  uint32_t i_flags;            // File flags
  uint32_t i_osd1;             // Operating system-specific value 1
  uint32_t i_block[15];        // Pointers to data blocks
  uint32_t i_generation;       // File version (used by NFS)
  uint32_t i_file_acl;         // File ACL (access control list)
  uint32_t i_dir_acl;          // Directory ACL
  uint32_t i_faddr;            // Fragment address
  uint8_t i_osd2[12];          // Operating system-specific value 2
} Inode;

#endif