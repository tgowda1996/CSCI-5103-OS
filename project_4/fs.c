#include "fs.h"
#include "disk.h"
#include "data_structures.h"
#include "cache.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define FS_MAGIC           0xf0f03410

// Returns the number of dedicated inode blocks given the disk size in blocks
#define NUM_INODE_BLOCKS(disk_size_in_blocks) (1 + (disk_size_in_blocks / 10))

int* block_freemap;
int* inode_freemap;
int numblocks = -1;
int numinodes = -1;
struct Cache* cache;

int is_arg_valid(int, int, int);
int is_mounted();
int find_free_block();
int create_new_data_block(struct fs_inode*, int, int*);
int get_block_number_for_offset(struct fs_inode*, int);
int min(int, int);
int inode_load(int inumber, struct fs_inode* inode);
int inode_save(int inumber, struct fs_inode* inode);
void copy_data(char *, const char *, int);

void print_block_freemap() {
  printf("  block_freemap:\n");
  for (int i = 0; i < numblocks; i++) {
    printf("    Block %d -> %d\n", i, block_freemap[i]);
  } // for
} // print_freemap()

void print_inode_freemap() {
  printf("  inode_freemap:\n");
  for (int i = 0; i < numinodes; i++) {
    printf("    inode %d -> %d\n", i, inode_freemap[i]);
  } // for
} // print_freemap()

void fs_debug() {
  union fs_block super_block;
  // superblock
  disk_read(0, super_block.data);
  printf("superblock:\n");
  if (super_block.super.magic != FS_MAGIC) {
    printf("    magic number is invalid\n");
    return;
  } // if
  printf("    %d blocks\n",super_block.super.nblocks);
  printf("    %d inode blocks\n",super_block.super.ninodeblocks);
  printf("    %d inodes\n",super_block.super.ninodes);
  // inodes
  union fs_block inode_block;
  if (cache) {
    cache->flush_cache(cache);
  }
  for (int i = 0; i < super_block.super.ninodeblocks; i++) {
    assert(i + 1 < super_block.super.nblocks);
    disk_read(i + 1, inode_block.data);
    for (int j = 0; j < INODES_PER_BLOCK; j++) {
      struct fs_inode inode = inode_block.inode[j];
      if (inode.isvalid) { // in use
        printf("inode %d:\n", j + i * INODES_PER_BLOCK);
        printf("    size: %d bytes\n",inode.size);
        // direct blocks
        printf("    direct blocks:");
        for (int k = 0; k < POINTERS_PER_INODE; k++) {
          if (inode.direct[k]) { // valid
            printf(" %d", inode.direct[k]);
          } else {
            break;
          } // else
        } // for k
        printf("\n");
        // indirect blocks
        if (inode.indirect) { // valid
          printf("    indirect block: %d\n", inode.indirect);
          union fs_block pointer_block;
          disk_read(inode.indirect, pointer_block.data);
          printf("    indirect data blocks:");
          for (int k = 0; k < POINTERS_PER_BLOCK; k++) {
            if (pointer_block.pointers[k]) {
              printf(" %d", pointer_block.pointers[k]);
            } else {
              break;
            } // else
          } // for k
          printf("\n");
        } // if inode.indirect 
      } // if inode.isvalid
    } // for j
  } // for i
} // fs_debug()

int fs_format() {
  if (block_freemap || inode_freemap) { // mounted
    printf("Unmount before formatting\n");
    return 0;
  } // if
  // initialize the superblock
  union fs_block block;
  block.super.magic = FS_MAGIC;
  block.super.nblocks = disk_size();
  int num_inode_blocks = NUM_INODE_BLOCKS(block.super.nblocks);
  block.super.ninodeblocks = num_inode_blocks;
  block.super.ninodes = num_inode_blocks * INODES_PER_BLOCK;
  // initialize the inodes 
  union fs_block inode = {0};
  for (int i = 0; i < num_inode_blocks; i++) {
    disk_write(i + 1, inode.data);
  } // for
  // write superblock to disk
  disk_write(0, block.data); 
  return 1;
} // fs_format()

int fs_mount(int cache_capacity) {
  // read superblock
  union fs_block super_block;
  disk_read(0, super_block.data);
  if (super_block.super.magic != FS_MAGIC) {
    return 0;
  } // if
  // initialize freemaps
  numblocks = super_block.super.nblocks;
  numinodes = super_block.super.ninodes;
  block_freemap = (int *) calloc(numblocks, sizeof(int));
  if (block_freemap == NULL) {
    perror("Error - failed to allocate memory for block_freemap");
    return 0;
  } // if
  inode_freemap = (int *) calloc(numinodes, sizeof(int));
  if (inode_freemap == NULL) {
    perror("Error - failed to allocate memory for inode_freemap");
    return 0;
  } // if
  block_freemap[0] = 1;
  cache = Cache.new(cache_capacity);

  if (!cache) { // Out of memory
    return 0;
  }
  // traverse all valid inodes
  union fs_block inode_block;
  for (int i = 0; i < super_block.super.ninodeblocks; i++) {
    assert(i + 1 < super_block.super.nblocks);
    disk_read(i + 1, inode_block.data);
    block_freemap[i + 1] = 1;
    for (int j = 0; j < INODES_PER_BLOCK; j++) {
      struct fs_inode inode = inode_block.inode[j];
      if (inode.isvalid) { // in use
        inode_freemap[j + i * INODES_PER_BLOCK] = 1;
        // direct blocks
        for (int k = 0; k < POINTERS_PER_INODE; k++) {
          if (inode.direct[k]) { // valid
            block_freemap[inode.direct[k]] = 1;
          } else {
            break;
          } // else
        } // for k
        // indirect blocks
        if (inode.indirect) { // valid
          block_freemap[inode.indirect] = 1;
          union fs_block pointer_block;
          disk_read(inode.indirect, pointer_block.data);
          for (int k = 0; k < POINTERS_PER_BLOCK; k++) {
            if (pointer_block.pointers[k]) {
              block_freemap[pointer_block.pointers[k]] = 1;
            } else {
              break;
            } // else
          } // for k
        } // if inode.indirect 
      } // if inode.isvalid
    } // for j
  } // for i
  return 1;
} // fs_mount()

int fs_unmount() {
  if (block_freemap || inode_freemap) { // mounted
    Cache.destroy(cache);
    free(block_freemap);
    block_freemap = NULL;
    numblocks = -1;
    free(inode_freemap);
    inode_freemap = NULL;
    numinodes = -1;
    return 1;
  } // if
  printf("Not currently mounted\n");
  return 0;
} // fs_unmount()

int fs_create() {
  if (!is_mounted()){
    printf("Must be mounted to perform create\n");
    return -1;
  }

  // find a free inode
  for (int i = 0; i < numinodes; i++) {
    if (!inode_freemap[i]) {
      // allocate inode
      struct fs_inode inode = {0};
      inode.isvalid = 1;
      int res = inode_save(i, &inode);
      if (!res) {
        return -1;
      } // if
      inode_freemap[i] = 1;
      return i;
    } // if
  } // for

  return -1; // all inodes are full
} // fs_create()

int fs_delete(int inumber) {
  if (!is_mounted()){
    printf("Must be mounted to perform delete\n");
    return 0;
  }

  struct fs_inode* inode = cache->get_inode(cache, inumber);
  int res;
  if (!inode || !inode->isvalid) {
    return 0;
  } // if
  // free direct pointers
  for (int i = 0; i < POINTERS_PER_INODE; i++) {
    if (inode->direct[i]) {
      block_freemap[inode->direct[i]] = 0; // was a bug
    } else {
      break;
    } // else
  } // for
  // free indirect pointers
  if (inode->indirect) {
    union fs_block pointer_block;
    disk_read(inode->indirect, pointer_block.data);
    for (int i = 0; i < POINTERS_PER_BLOCK; i++) {
      if (pointer_block.pointers[i]) {
        block_freemap[pointer_block.pointers[i]] = 0;
      } else {
        break;
      } // else
    } // for
    block_freemap[inode->indirect] = 0;
  } // if
  // invalidate inode
  res = cache->remove_if_present(cache, inumber, inode); // this makes sure that the inode is zeroed.
  if (!res) {
    return 0;
  } // if
  inode_freemap[inumber] = 0;
  return 1;
} // fs_delete()

int fs_getsize(int inumber) {
  if (!is_mounted()){
    printf("Must be mounted to perform getsize\n");
    return -1;
  }

  struct fs_inode *inode = cache->get_inode(cache, inumber);
  if (!inode || !inode->isvalid) {
    return -1;
  } // if
  return inode->size;
} // fs_getsize()

int fs_read(int inumber, char *data, int length, int offset) {
  if (!is_mounted()){
    printf("Must be mounted to perform fs_read\n");
    return 0;
  }

  if (!is_arg_valid(inumber, length, offset)) {
    return 0;
  }

  struct fs_inode* inode = cache->get_inode(cache, inumber);
  if (!inode || !inode->isvalid) {
    printf("Invalid inode\n");
    return 0;
  }

  union fs_block data_block;
  int buffer_start_index, block_number;
  int bytes_read = 0, nbytes_to_read = 0, total_bytes_to_be_read = min(inode->size-offset, length);
  while(total_bytes_to_be_read > 0) {
    buffer_start_index = offset%DISK_BLOCK_SIZE;
    block_number = get_block_number_for_offset(inode, offset);

    // no mapping exists and hence no more data to read. EOF reached.
    if (block_number == 0) {
      return bytes_read;
    }

    nbytes_to_read = min(total_bytes_to_be_read, DISK_BLOCK_SIZE-buffer_start_index);
    disk_read(block_number, data_block.data);
    copy_data(data+bytes_read, data_block.data+buffer_start_index, nbytes_to_read);

    bytes_read += nbytes_to_read;
    length -= nbytes_to_read;

    offset += nbytes_to_read;
    total_bytes_to_be_read -= nbytes_to_read;
  }

  return bytes_read;
} // fs_read()

int fs_write(int inumber, const char *data, int length, int offset) {
  if (!is_mounted()){
    printf("Must be mounted to perform fs_write\n");
    return 0;
  }

  if (!is_arg_valid(inumber, length, offset)) {
    return 0;
  }

  struct fs_inode* inode = cache->get_inode(cache, inumber);
  if (!inode) {
    printf("Invalid inode\n");
    return 0;
  }

  int write_inode = !inode->isvalid;
  inode->isvalid = 1;

  int buffer_start_index, block_number, original_offset = offset;
  int bytes_written = 0, bytes_to_be_written = 0;
  while(length > 0) {
    union fs_block data_block;
    buffer_start_index = offset%DISK_BLOCK_SIZE;
    block_number = get_block_number_for_offset(inode, offset);

    // no mapping exists. We need to create a block.
    if (block_number == 0) {
      block_number = create_new_data_block(inode, offset, &write_inode);
    }

    if (block_number == 0) {
      break; // error encountered. something like out of memory.
    }

    bytes_to_be_written = min(DISK_BLOCK_SIZE-buffer_start_index, length);
    disk_read(block_number, data_block.data);
    copy_data(data_block.data+buffer_start_index, data+bytes_written, bytes_to_be_written);
    disk_write(block_number, data_block.data);

    bytes_written += bytes_to_be_written;
    length -= bytes_to_be_written;
    offset += bytes_to_be_written;
  }

  int total_bytes_written = bytes_written+original_offset;
  write_inode = total_bytes_written > inode->size ? 1 : write_inode;
  inode->size = (total_bytes_written > inode->size) ? total_bytes_written : inode->size;
  if (write_inode) {
    cache->set_dirty(cache, inumber, write_inode, inode);      
  }

  return bytes_written;
} // fs_write()


/**
 * Copy length data from source to destination
 */
void copy_data(char *destination, const char *source, int length) {
  for (int i = 0; i < length; i++) {
    destination[i] = source[i];
  }
}


/**
 * Returns a block number for offset if block is allocated.
 * if no mapping exists, it returns 0
 */
int get_block_number_for_offset(struct fs_inode* inode, int offset){
  int pointer_number = offset/DISK_BLOCK_SIZE;
  union fs_block indirect_mapping_block;

  if (pointer_number < POINTERS_PER_INODE) {
    return inode->direct[pointer_number];
  }
  else if (!inode->indirect) {
    return inode->indirect;
  }
  else {
    disk_read(inode->indirect, indirect_mapping_block.data);
    return indirect_mapping_block.pointers[pointer_number-POINTERS_PER_INODE];
  }
}

/**
 * Used to assign a new block to the inode
 */
int create_new_data_block(struct fs_inode* inode, int offset, int* write_inode){
  int pointer_number = offset/DISK_BLOCK_SIZE;
  int block_number, block_number_for_indirect;
  union fs_block indirect_mapping_block = {0};

  block_number = find_free_block();
  if (!block_number) {
    printf("Out of memory\n");
    return 0;
  }

  if (pointer_number < POINTERS_PER_INODE) {
    inode->direct[pointer_number] = block_number;
    *write_inode = 1;
  } 
  else {
    block_number_for_indirect = inode->indirect;
    if (!block_number_for_indirect) {
      block_number_for_indirect = block_number;
      block_freemap[block_number_for_indirect] = 1;
      block_number = find_free_block();
    }

    if (!block_number) {
      block_freemap[block_number_for_indirect] = 0;
      printf("Out of memory\n");
      return 0;
    }

    if (inode->indirect != block_number_for_indirect) {
      inode->indirect = block_number_for_indirect;
      *write_inode = 1;
    }
    else {
      disk_read(block_number_for_indirect, indirect_mapping_block.data);
    }
    indirect_mapping_block.pointers[pointer_number-POINTERS_PER_INODE] = block_number;
    disk_write(block_number_for_indirect, indirect_mapping_block.data);
  }

  block_freemap[block_number] = 1;
  union fs_block data_block = {0};
  disk_write(block_number, data_block.data);
  return block_number;
}

/**
 * Return block number of the free block
 */
int find_free_block(){
  for (int i = 1+NUM_INODE_BLOCKS(numblocks); i < numblocks; i++) {
    if (block_freemap[i] == 0) return i;
  }
  return 0;
}

/**
 * Check if disk is mounted
 */
int is_mounted(){
  return (block_freemap || inode_freemap);
}

/**
 * Validates arguments passed to read and write
 */
int is_arg_valid(int inumber, int length, int offset) {
  if (length < 0 || offset < 0) {
    printf("Negative parameters not allowed\n");
    return 0;
  }

  if (inumber >= numinodes) {
    printf("Invalid inumber\n");
    return 0;
  }

  if (!inode_freemap[inumber]) {
    printf("fs_create not called for inumber %d\n", inumber);
    return 0;
  }

  if (offset > (POINTERS_PER_INODE*DISK_BLOCK_SIZE + POINTERS_PER_BLOCK*DISK_BLOCK_SIZE)) {
    printf("Invalid offset\n");
    return 0;
  }

  return 1;
}

/**
 * Return min between two numbers
 */
int min(int x, int y){
  return (x <= y ? x : y);
}

