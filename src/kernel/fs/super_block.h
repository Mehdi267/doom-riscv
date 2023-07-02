#ifndef SUPER_BLOCK 
#define SUPER_BLOCK


#include <stdint.h>

//Ext2 super block
//as descriped in this following ext 2 spec: https://www.nongnu.org/ext2-doc/ext2.pdf 
//Some of these elemnts will not be used as our file system will be very minimal
typedef struct {
    /* Superblock Structure */
    uint32_t s_inodes_count;                // Total number of inodes
    uint32_t s_blocks_count;                // Total number of blocks
    uint32_t s_r_blocks_count;              // Number of reserved blocks
    uint32_t s_free_blocks_count;           // Number of free blocks

    /* Disk Organization */
    uint32_t s_free_inodes_count;           // Number of free inodes
    uint32_t s_first_data_block;            // Block number of first data block
    uint32_t s_log_block_size;              // Block size (log2)
    uint32_t s_log_frag_size;               // Fragment size (log2)
    uint32_t s_blocks_per_group;            // Blocks per group
    uint32_t s_frags_per_group;             // Fragments per group
    uint32_t s_inodes_per_group;            // Inodes per group
    uint32_t s_mtime;                       // Mount time
    uint32_t s_wtime;                       // Write time
    uint16_t s_mnt_count;                   // Mount count
    uint16_t s_max_mnt_count;               // Maximal mount count
    uint16_t s_magic;                       // Magic signature
    uint16_t s_state;                       // File system state
    uint16_t s_errors;                      // Behaviour when detecting errors
    uint16_t s_minor_rev_level;             // Minor revision level
    uint32_t s_lastcheck;                   // Last check time
    uint32_t s_checkinterval;               // Check interval
    uint32_t s_creator_os;                  // OS ID
    uint32_t s_rev_level;                   // Revision level
    uint16_t s_def_resuid;                  // Default uid for reserved blocks
    uint16_t s_def_resgid;                  // Default gid for reserved blocks

    /* EXT2_DYNAMIC_REV Specific */
    uint32_t s_first_ino;                   // First non-reserved inode
    uint16_t s_inode_size;                  // Size of inode structure
    uint16_t s_block_group_nr;              // Block group number of this superblock
    uint32_t s_feature_compat;              // Compatible feature set flags
    uint32_t s_feature_incompat;            // Incompatible feature set flags
    uint32_t s_feature_ro_compat;           // Readonly-compatible feature set flags
    uint8_t s_uuid[16];                     // 128-bit UUID for volume
    uint8_t s_volume_name[16];              // Volume name
    uint8_t s_last_mounted[64];             // Directory where the file system was last mounted

    /* Performance Hints */
    uint32_t s_algo_bitmap;                 // Algorithm usage bitmap

    /* Journaling Support */
    uint8_t s_prealloc_blocks;              // Number of blocks to preallocate for files
    uint8_t s_prealloc_dir_blocks;          // Number of blocks to preallocate for directories
    uint16_t padding;                       // Alignment

    /* Journaling Support */
    uint8_t s_journal_uuid[16];             // 128-bit UUID of journal superblock
    uint32_t s_journal_inum;                // Inode number of journal file
    uint32_t s_journal_dev;                 // Device number of journal file

    /* Directory Indexing Support */
    uint32_t s_hash_seed[4];                // Hash seed array for directory hashing
    uint8_t s_def_hash_version;             // Default hash version
    uint8_t reserved[3];                    // Padding - reserved for future expansion

    /* Other options */
    uint32_t s_default_mount_options;       // Default mount options
    uint32_t s_first_meta_bg;               // First metablock block group
    uint8_t unused[760];                    // Unused - reserved for future revisions
} Superblock;


#endif