#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include "disk.h"

#define INODES_PER_BLOCK   128
#define POINTERS_PER_INODE 5
#define POINTERS_PER_BLOCK 1024

extern int numinodes;

struct fs_superblock {
  int magic;          // Magic bytes
  int nblocks;        // Size of the disk in number of blocks
  int ninodeblocks;   // Number of blocks dedicated to inodes
  int ninodes;        // Number of dedicated inodes
};

struct fs_inode {
  int isvalid;                      // 1 if valid (in use), 0 otherwise
  int size;                         // Size of file in bytes
  int direct[POINTERS_PER_INODE];   // Direct data block numbers (0 if invalid)
  int indirect;                     // Indirect data block number (0 if invalid)
};

union fs_block {
  struct fs_superblock super;               // Superblock
  struct fs_inode inode[INODES_PER_BLOCK];  // Block of inodes
  int pointers[POINTERS_PER_BLOCK];         // Indirect block of direct data block numbers
  char data[DISK_BLOCK_SIZE];               // Data block
};

struct node {
  struct node* next;
  struct node* prev;
  int inumber;
  struct fs_inode* inode;
  int is_dirty;
};

#endif