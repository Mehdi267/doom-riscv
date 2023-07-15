#ifndef INODE_BLOCK 
#define INODE_BLOCK

#include <stdint.h>
#include <stddef.h>

//Reserved inodes
#define EXT2_BAD_INO 1        // bad blocks inode
#define EXT2_ROOT_INO 2       // root directory inode
#define EXT2_ACL_IDX_INO 3    // ACL index inode (deprecated?)
#define EXT2_ACL_DATA_INO 4   // ACL data inode (deprecated?)
#define EXT2_BOOT_LOADER_INO 5 // boot loader inode
#define EXT2_UNDEL_DIR_INO 6  // undelete directory inode

// i_mode values
#define EXT2_S_IFSOCK 0xC000  // socket
#define EXT2_S_IFLNK 0xA000   // symbolic link
#define EXT2_S_IFREG 0x8000   // regular file
#define EXT2_S_IFBLK 0x6000   // block device
#define EXT2_S_IFDIR 0x4000   // directory
#define EXT2_S_IFCHR 0x2000   // character device
#define EXT2_S_IFIFO 0x1000   // fifo
#define EXT2_S_ISUID 0x0800   // Set process User ID
#define EXT2_S_ISGID 0x0400   // Set process Group ID
#define EXT2_S_ISVTX 0x0200   // sticky bit
#define EXT2_S_IRUSR 0x0100   // user read
#define EXT2_S_IWUSR 0x0080   // user write
#define EXT2_S_IXUSR 0x0040   // user execute
#define EXT2_S_IRGRP 0x0020   // group read
#define EXT2_S_IWGRP 0x0010   // group write
#define EXT2_S_IXGRP 0x0008   // group execute
#define EXT2_S_IROTH 0x0004   // others read
#define EXT2_S_IWOTH 0x0002   // others write
#define EXT2_S_IXOTH 0x0001   // others execute

// i_flags values
#define EXT2_SECRM_FL 0x00000001        // secure deletion
#define EXT2_UNRM_FL 0x00000002         // record for undelete
#define EXT2_COMPR_FL 0x00000004        // compressed file
#define EXT2_SYNC_FL 0x00000008         // synchronous updates
#define EXT2_IMMUTABLE_FL 0x00000010    // immutable file
#define EXT2_APPEND_FL 0x00000020       // append only
#define EXT2_NODUMP_FL 0x00000040       // do not dump/delete file
#define EXT2_NOATIME_FL 0x00000080      // do not update .i_atime
#define EXT2_DIRTY_FL 0x00000100        // Dirty (modified)
#define EXT2_COMPRBLK_FL 0x00000200     // compressed blocks
#define EXT2_NOCOMPR_FL 0x00000400      // access raw compressed data
#define EXT2_ECOMPR_FL 0x00000800       // compression error
#define EXT2_BTREE_FL 0x00001000        // b-tree format directory
#define EXT2_INDEX_FL 0x00001000        // hash indexed directory
#define EXT2_IMAGIC_FL 0x00002000       // AFS directory
#define EXT3_JOURNAL_DATA_FL 0x00004000 // journal file data
#define EXT2_RESERVED_FL 0x80000000     // reserved for ext2 library


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
} inode_t;

#define NB_DIRECT_BLOCKS 12
#define L_DIRECT 12 //size limits
#define NB_ONE_INDIRECT_BLOCKS 1
#define INDIRECT_BLOCKS_INDEX 13
#define L_ONE_INDIRECT 525
#define NB_DOUBLE_INDIRECT_BLOCKS 1
#define L_DOUBLE_INDIRECT 262668
#define NB_TRIPLE_INDIRECT_BLOCKS 1 

// limiting my self to double list atm 
#define MAX_BLOCKS_FILE 262668 

typedef enum file_type{
  EXT2_FT_UNKNOWN = 0, //Unknown File Type
  EXT2_FT_REG_FILE = 1, //Regular File
  EXT2_FT_DIR = 2, //Directory File
  EXT2_FT_CHRDEV = 3, //Character Device
  EXT2_FT_BLKDEV = 4, //Block Device
  EXT2_FT_FIFO = 5, //Buffer File
  EXT2_FT_SOCK = 6, //Socket File
  EXT2_FT_SYMLINK = 7, //Symbolic Link
  EXT2_FT_FREE = 8, //Place holder 
} file_t;

typedef struct Linked_directory_entry_basic {
  uint32_t inode_n;
  uint16_t rec_len;
  uint8_t name_len;
  uint8_t file_type;
} dir_entry_basic;

typedef struct Linked_directory_entry {
  uint32_t inode_n;
  uint16_t rec_len;
  uint8_t name_len;
  uint8_t file_type;
  char* name;
} dir_entry;

//sizeof(uint32_t)+sizeof(uint16_t)+sizeof(uint8_t)+sizeof(uint8_t);
#define SIZE_DIR_NO_NAME 8
//Functions names taken from 
//Andrew S. Tanenbaum -
//Operating Systems. Design and Implementation

/**
 * @brief Get the inode object
 * 
 * @return inode_t* 
 */
inode_t* get_inode(uint32_t inode_number);

typedef enum put_operation_type{
  RELEASE_INODE = 0, 
  SAVE_INODE = 1, 
} put_op;

/**
 * @brief Return an i-node that is no longer needed.
 *
 * This function releases an i-node that is no longer needed by marking it as unused.
 *
 * @param[in] inode The i-node to be released.
 * @param[in] inode_number the inode id, place the value 0 if the node is 
 * is in the inode table
 * @returns int operation status 
 */
int put_inode(inode_t* inode, uint32_t inode_number, put_op op_type);


/**
 * @brief Allocate a new i-node for a new file.
 *
 * This function allocates a new i-node to be used for a new file.
 *
 * @return The newly allocated i-node.
 */
inode_t* alloc_inode();

/**
 * @brief Release an i-node when a file is removed(deleted).
 * This function releases an i-node when a file is removed,
 * making the inode available for reuse.
 *
 * @param[in] inode The i-node to be released.
 * @param[in] inode_number The number of 
 * the i-node to be released.
 */
int free_inode(inode_t* inode, uint32_t inode_number);

/**
 * @brief Duplicate an i-node.
 *
 * This function duplicates an i-node, creating a new i-node with the same contents.
 *
 * @param[in] inode The i-node to be duplicated.
 * @return The duplicated i-node.
 */
inode_t* dup_inode(inode_t* inode);

typedef struct inode_table_elt{
  inode_t* address; //not needed but added for ease of use
  uint32_t inode_id;
  uint32_t inode_usage;
  struct inode_table_elt* next_inode;
  struct inode_table_elt* previous_inode;
} inode_elt;

/**
 * @brief Get the inode from node id object
 * @param node_id the inode id
 * @return inode_t* the data structure of the inode
 */
inode_t* get_inode_from_node_id(uint32_t node_id);

/**
 * @brief Get the inode elt object from the inode
 * @param inode the inode object 
 * @return inode_elt* the adresses of the inode elt
 */
inode_elt* get_inode_t_elt(inode_t* inode);

/**
 * @brief Get a empty data block for the data region of the 
 * file system
 * @return uint32_t the id of the data block
 */
uint32_t get_data_block();

/**
 * @brief Get the inode number from the inode that 
 * was provided in the function argument
 * @param inode inode for which would the inode number
 * @return uint32_t inode number
 */
uint32_t get_inode_number(inode_t* inode);

/**
 * @brief deallocate the block with the number equal
 * to the block_number given in the parameter
 * @param block_number the block number 
 * that we would like to free
 * @return int function status 
 */
int free_data_block(uint32_t data_block);

/**
 * @brief Add a inode to a directory
 * @param dir the inode of the directory
 * @param inode_number the inode that we wish to add
 * @param type the inode type
 * @param name the name
 * @param name_size the name size
 * @return int function status
 */
int add_inode_directory(inode_t* dir, 
  uint32_t inode_number,
  file_t type,
  char* name,
  size_t name_size);

/**
 * @brief Add a blocks to the inode blocks
 * @param inode the inode on which we would
 * like to add a block 
 * @return int function status 
 */
int add_data_block_inode(inode_t* inode);


/**
 * @brief Write the dir_entry struct in a data block
 * the location is the position in which we will write the block
 * in bytes(must be 4 bytes aligned)
 * @param data_block 
 * @param location_b 
 * @param entry 
 * @return int 
 */
int write_dir_entry(char* data_block,
                    uint32_t location_b,
                    dir_entry* entry);

/**
 * @brief Save the entry into the block located at 
 * block_nb 
 * @param entry the dir entry
 * @param location_b the location in the disk where it will be saved
 * @param block_nb the block number
 * @return int 
 */
int save_dir_entry( dir_entry* entry,
                    uint32_t location_b,//Will mostly be zero
                    uint32_t block_nb);

/**
 * @brief free the list of elements in the inode list
 * @param list the inode list
 * @return int funcion status
 */
int free_inode_list(inode_elt* list);

/**
 * @brief Print all of the files that are present in 
 * the directory inode that was given as a function argument
 * @param dir the directory inode
 */
void print_dir_list(inode_t* dir);

/**
 * @brief prints basic entry with no filename
 * @param entry 
 */
void print_dir_entry_basic(dir_entry_basic* entry);

/**
 * @brief prints entry with filename
 * @param entry 
 */
void print_dir_entry(dir_entry_basic* entry);

/**
 * @brief Prints the dir entry object with the filename
 * printed from the pointer;
 * @param entry 
 */
void print_dir_entry_obj(dir_entry* entry);

/**
 * @brief Prints the current inode cache details
 * @param list inode list
 */
void print_cache_details(inode_elt* list);

/**
 * @brief Looks for an inode in the directory inode 
 * given as function argument
 * @param dir the directory that contains the inodes
 * @param name name of the inode that we are looking for
 * @param name_len length of the name
 * @return uint32_t 0 of the inode was not found
 * and the inode number if it was found
 */
uint32_t look_for_inode_dir(inode_t* dir, 
        char* name,
        uint32_t name_len);

/**
 * @brief Removes the inode for the direcotry 
 * inode that has the name given as argument 
 * @param dir the directory inode
 * @param name the name of the inode
 * @param name_len the length of the name of the inode
 * @return uint32_t status
 */
int remove_inode_dir(inode_t* dir, 
        char* name,
        uint32_t name_len);

#endif