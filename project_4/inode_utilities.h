#ifndef INODE_UTILITIES_H
#define INODE_UTILITIES_H

#include "disk.h"
#include "data_structures.h"

int inode_load(int inumber, struct fs_inode* inode) {
  int inode_block_number = 1 + inumber / INODES_PER_BLOCK; 
  if (numinodes == -1 || inode_block_number >= numinodes / INODES_PER_BLOCK) {
    return 0;
  } // if
  union fs_block block;
  disk_read(inode_block_number, block.data);
  *inode = block.inode[inumber % INODES_PER_BLOCK]; 
  return 1;
} // inode_load()

int inode_save(int inumber, struct fs_inode* inode) {
  int inode_block_number = 1 + inumber / INODES_PER_BLOCK; 
  if (numinodes == -1 || inode_block_number >= numinodes / INODES_PER_BLOCK) {
    return 0;
  } // if
  union fs_block block;
  disk_read(inode_block_number, block.data);
  block.inode[inumber % INODES_PER_BLOCK] = *inode;
  disk_write(inode_block_number, block.data);
  return 1;
} // inode_save()

#endif