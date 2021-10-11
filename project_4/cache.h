#ifndef CACHE_H
#define CACHE_H

#include "data_structures.h"

/**
 * Write back policy and lru.
 * Using doubly linked list to simulate lru
 */

struct Cache
{
  int capacity, current_size;
  struct node* head, *tail; 

  /**
   * Get inode corresponding to the inumber
   */
  struct fs_inode* (*get_inode)(struct Cache *this, int inumber);
  /**
   * Remove inode from cache.
   * Write back if necessary
   */
  int (*remove_if_present)(struct Cache *this, int inumber, struct fs_inode* corresponding_inode);
  /**
   * Mark a node as dirty.
   * Makes the cache aware of dirty inodes. Used in write back
   */
  void (*set_dirty)(struct Cache *this, int inumber, int set_dirty, struct fs_inode* corresponding_inode);
  /**
   * Write all dirty inodes back to disk
   */
  void (*flush_cache)(struct Cache *this);
};

/**
 * struct used to create new Cache type object
 */
struct CacheClass {
  /**
   * constructor for cache
   */
  struct Cache* (*new)(int capacity);
  /**
   * destructor for cache
   */
  void (*destroy)(struct Cache* cache);
}; 

extern const struct CacheClass Cache;

#endif
