#include "cache.h"
#include "inode_utilities.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static void find_node_with_inumber(int, struct fs_inode**, struct node**);

static void insert_ele_in_the_end(struct node* tail, struct node* ele){
  ele->prev = tail->prev;
  ele->next = tail;
  tail->prev->next = ele;
  tail->prev = ele;
}

static int insert_in_the_end(struct node* tail, struct fs_inode* inode, int inumber){
  struct node* ele = malloc(sizeof(struct node));
  if (!ele) {
    printf("System out of memory\n");
    return 0;
  }
  ele->inode = inode;
  ele->inumber = inumber;
  insert_ele_in_the_end(tail, ele);
  return 1;
}

/**
 * Removes node from cache and writes it back to disk if dirty
 * The move parameter when set to 1  
 */
static void remove_node(struct node* ele, int move) {
  ele->prev->next = ele->next;
  ele->next->prev = ele->prev;

  if (!move && ele->is_dirty) {
    inode_save(ele->inumber, ele->inode);
  }
}

static struct fs_inode* get_inode(struct Cache *this, int inumber){
  struct node* temp = this->head;
  struct node* head = this->head;
  struct node* tail = this->tail;
  struct fs_inode* inode = NULL;

  find_node_with_inumber(inumber, &inode, &temp);
  // found. Re arrange ll in order facilitate LRU
  if (inode != NULL) {
    // remove temp
    remove_node(temp, 1);

    // insert temp in the end of ll
    insert_ele_in_the_end(tail, temp);
    return inode;
  }

  struct node* inode_to_be_removed;
  inode = malloc(sizeof(struct fs_inode));
  if (!inode) {
    printf("System out of memory\n");
    return NULL;
  }

  int res = inode_load(inumber, inode);
  if (!res) {
    return NULL;
  }

  res = insert_in_the_end(tail, inode, inumber);
  if (!res) {
    printf("System out of memory\n");
    return NULL;
  }
  this->current_size += 1;

  if (this->current_size > this->capacity) {
    inode_to_be_removed = head->next;
    remove_node(inode_to_be_removed, 0);
    if (this->capacity) { // dont want to remove when cache size is zero
      free(inode_to_be_removed->inode);
    }
    free(inode_to_be_removed);
    this->current_size -= 1;
  }

  return inode;
}

static int remove_if_present(struct Cache *this, int inumber, struct fs_inode* corresponding_inode){
  struct node* temp = this->head;
  struct fs_inode* inode = NULL;

  find_node_with_inumber(inumber, &inode, &temp);

  if (inode) {
    remove_node(temp, 0);
    free(temp->inode);
    free(temp);
  }
  else { // cache with size 0
    free(corresponding_inode);
  }

  struct fs_inode empty_inode = {0};
  return inode_save(inumber, &empty_inode);
}

static void set_dirty(struct Cache *this, int inumber, int is_dirty, struct fs_inode* corresponding_inode){
  struct node* temp = this->head;
  struct fs_inode* inode = NULL;

  find_node_with_inumber(inumber, &inode, &temp);

  if (inode) {
    assert(corresponding_inode == inode);
    temp->is_dirty = is_dirty;
  }
  else { // cache with size 0
    inode_save(inumber, corresponding_inode);
  }
}

static void flush_cache(struct Cache *this) {
  struct node* head = this->head, *tail = this->tail;
  struct node* temp = head->next;
  while (temp != tail) {
    if (temp->is_dirty) {
      inode_save(temp->inumber, temp->inode);
      temp->is_dirty = 0;
    }
    temp = temp->next;
  }
}

static struct Cache* new(int capacity){
  struct Cache* cache = malloc(sizeof (struct Cache));
  if (!cache) {
    printf("System out of memory\n");
    return NULL;
  }
  cache->capacity = capacity;
  cache->current_size = 0;
  cache->head = malloc(sizeof (struct node));
  cache->tail = malloc(sizeof (struct node));
  if (!cache->head || !cache->tail) {
    printf("System out of memory\n");
    return NULL;
  }
  cache->head->next = cache->tail;
  cache->tail->prev = cache->head;
  cache->head->prev = NULL;
  cache->tail->next = NULL;
  cache->head->inumber = -1;
  cache->tail->inumber = -1;

  cache->get_inode = &get_inode;
  cache->remove_if_present = &remove_if_present;
  cache->set_dirty = &set_dirty;
  cache->flush_cache = &flush_cache;

  return cache;
}

static void destroy(struct Cache* cache){
  struct node* temp = cache->head->next, *next;
  while(temp != cache->tail) {
    next = temp->next;
    remove_node(temp, 0);
    if (temp->inode){
      free(temp->inode);
    }
    free(temp);
    temp = next;
  }
  free(cache->head);
  free(cache->tail);
}

// inode and node_position get updated
static void find_node_with_inumber(int inumber, struct fs_inode** inode, struct node** node_position){
  while(*node_position) {
    if ((*node_position)->inumber == inumber) {
      *inode = (*node_position)->inode;
      break;
    }
    (*node_position) = (*node_position)->next;
  }
}

const struct CacheClass Cache = {.new = &new, .destroy = &destroy};
