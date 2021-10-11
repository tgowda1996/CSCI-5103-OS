/*
 * Main program for the virtual memory project.
 * Make all of your modifications to this file.
 * You may add or rearrange any code or data as you need.
 * The header files page_table.h and disk.h explain
 * how to use the page table and disk interfaces.
 */

#include "page_table.h"
#include "disk.h"
#include "program.h"
#include "coremap.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <queue>
#include <vector>
#include <unordered_set>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <ucontext.h>
#include <unistd.h>
#include <climits>

using namespace std;

#define MICRO_TO_SECOND 1000000
#define FAIL -1
// Prototype for test program
typedef void (*program_f)(char *data, int length);

// Prototype for policy
typedef int (*policy)();

// Number of physical frames
int nframes;

// Queue used for FIFO page replacement algorithm
std::queue<int> frame_queue;

// Implementation stats
int page_faults, disk_reads, disk_writes;

// Pointer to disk for access from handlers
struct disk *disk = nullptr;

// data structure containing a mapping from frame number to page number
static vector<CoreMapEntry*> core_map;
// data structure the number of times a particular page was not used. It is updated through the clock algorithm.
static vector<int> not_used;
// set of pages that are resident in memory at any given point of time. Used only while using the custom page fault
// handler
static unordered_set<int> pages_in_mem;


static struct itimerval _timer;
static struct sigaction _sigAction;
static int clock_gran = 1;
static page_table* _pt = NULL;

static void initialize_timer();
static void disable_timer();
static void timer_handler(int signum);
static void set_perms(page_table *pt, int page, int frame, int bits);
static vector<int> not_recently_used(int k);
static vector<int> filter_dirty(vector<int> frames_not_recently_used);
static int get_fifo(vector<int> evict_candidates);
static void disableInterrupts();
static void enableInterrupts();
static void free_up_core_map();
static bool is_page_in_memory(int page, int bits);
static int evict_frame_from_memory(struct page_table *pt, policy get_frame_to_be_evicted);
static int get_frame_to_be_evicted_custom_policy();
static int get_frame_to_be_evicted_random_policy();
static int get_frame_to_be_evicted_fifo_policy();

static void read_from_disk_and_set_pt_entry(page_table *pt, int page, int frame);

// Simple handler for pages == frames
void page_fault_handler_example(struct page_table *pt, int page) {
  cout << "page fault on page #" << page << endl;

  // Print the page table contents
  cout << "Before ---------------------------" << endl;
  page_table_print(pt);
  cout << "----------------------------------" << endl;

  // Map the page to the same frame number and set to read/write
  page_table_set_entry(pt, page, page, PROT_READ | PROT_WRITE);

  // Print the page table contents
  cout << "After ----------------------------" << endl;
  page_table_print(pt);
  cout << "----------------------------------" << endl;
} // page_fault_handler_example()

/**
 * Handles a page fault using the rand policy. It can occur if the write permissions were not set or if the page
 * doesnt exist in memory
 * @param pt
 * @param page
 */
void page_fault_handler_rand(struct page_table *pt, int page) {
  int frame, bits;
  page_table_get_entry(pt, page, &frame, &bits); 
  if (is_page_in_memory(page, bits)) { // resident in memory
    // set write permissions
    assert(core_map[frame]->getPageNumber() == page);
    set_perms(pt, page, frame, bits);
  } else { // not resident in memory
    page_faults++;
    // evict frame at random
    frame = evict_frame_from_memory(pt, &get_frame_to_be_evicted_random_policy);
    // read data into memory and update _pt entry
    read_from_disk_and_set_pt_entry(pt, page, frame);
    core_map[frame]->assignNewPage(page);
  } // else
} // page_fault_handler_rand()

/**
 * Handles a page fault using the fifo policy. It can occur if the write permissions were not set or if the page
 * doesnt exist in memory
 * @param pt
 * @param page
 */
void page_fault_handler_fifo(struct page_table *pt, int page) {
  int frame, bits;
  page_table_get_entry(pt, page, &frame, &bits); 
  if (is_page_in_memory(page, bits)) { // resident in memory
    // set write permissions
    assert(core_map[frame]->getPageNumber() == page);
    set_perms(pt, page, frame, bits);
  } else { // not resident in memory
    page_faults++;
    // evict frame which has been resident in memory longest
    frame = evict_frame_from_memory(pt, &get_frame_to_be_evicted_fifo_policy);
    // read data into memory and update _pt entry
    read_from_disk_and_set_pt_entry(pt, page, frame);
    core_map[frame]->assignNewPage(page);
    frame_queue.push(frame);
  } // else
} // page_fault_handler_fifo()

/**
 * Handles a page fault using the custom policy. It can occur if the write permissions were not set or if the page
 * doesnt exist in memory
 * @param pt
 * @param page
 */
void page_fault_handler_custom(struct page_table *pt, int page) {
  disableInterrupts(); // dont want to run clock algo when updating data structures.
  int frame, bits;
  page_table_get_entry(pt, page, &frame, &bits);
  if (is_page_in_memory(page, bits)){ // resident in memory
    // set appropriate permissions
    assert(core_map[frame]->getPageNumber() == page);
    set_perms(pt, page, frame, bits);
  } else { // not resident in memory
    page_faults++;
    // use custom policy to evict frame from memory
    frame = evict_frame_from_memory(pt, &get_frame_to_be_evicted_custom_policy);
    pages_in_mem.erase(core_map[frame]->getPageNumber());
    // read data into memory and update _pt entry
    read_from_disk_and_set_pt_entry(pt, page, frame);
    core_map[frame]->assignNewPage(page);
    pages_in_mem.insert(page);
    not_used[frame] = 0;
  } // else
  enableInterrupts();
}// page_fault_handler_custom()

int main(int argc, char *argv[]) {
  // Check argument count
  if (argc != 5) {
    cerr << "Usage: virtmem <npages> <nframes> <rand|fifo|custom> <sort|scan|focus>" << endl;
    exit(1);
  } // if

  // Parse command line arguments
  int npages = atoi(argv[1]);
  nframes = atoi(argv[2]);
  const char *algorithm = argv[3];
  const char *program_name = argv[4];

  // Validate the algorithm specified and select corresponding page fault handler
  page_fault_handler_t page_fault_handler;

  //Initialize core_map
  for (int i = 0; i < nframes; i++) {
    core_map.push_back(new CoreMapEntry(i));
  }

  if (!strcmp(algorithm, "rand")) {
    page_fault_handler = page_fault_handler_rand;
  } else if (!strcmp(algorithm, "fifo")) {
    page_fault_handler = page_fault_handler_fifo;
  } else if(!strcmp(algorithm, "custom")) {
    page_fault_handler = page_fault_handler_custom;
    // initialize not_used vector
    for (int i = 0; i < nframes; i++) {
      not_used.push_back(0);
    }
    // initialize timer
    initialize_timer();
  } else {
    cerr << "ERROR: Unknown algorithm: " << algorithm << endl;
    exit(1);
  } // else

  // Validate the program specified
  program_f program = NULL;
  if (!strcmp(program_name, "sort")) {
    if (nframes < 2) {
      cerr << "ERROR: nFrames >= 2 for sort program" << endl;
      exit(1);
    } // if
    program = sort_program;
  } else if (!strcmp(program_name, "scan")) {
    if (nframes < 1 || npages < 1) {
      cerr << "ERROR: nFrames >= 1 and nPages >= 1 for scan program" << endl;
      exit(1);
    } // if
    program = scan_program;
  } else if (!strcmp(program_name, "focus")) {
    if (nframes < 1 || npages < 1) {
      cerr << "ERROR: nFrames >= 1 and nPages >= 1 for focus program" << endl;
      exit(1);
    } // if
    program = focus_program;
  } else {
    cerr << "ERROR: Unknown program: " << program_name << endl;
    exit(1);
  } // else

  // Initializations
  srand(time(NULL));
  page_faults = disk_reads = disk_writes = 0;

  // Create a virtual disk
  disk = disk_open("myvirtualdisk", npages);
  if (!disk) {
    cerr << "ERROR: Couldn't create virtual disk: " << strerror(errno) << endl;
    return 1;
  } // if

  // Create a page table
  _pt = page_table_create(npages, nframes, page_fault_handler);
  if (!_pt) {
    cerr << "ERROR: Couldn't create page table: " << strerror(errno) << endl;
    return 1;
  } // if

  // Run the specified program
  char *virtmem = page_table_get_virtmem(_pt);
  program(virtmem, npages * PAGE_SIZE);

  // Print implementation stats
  cout << page_faults << " page faults, " 
    << disk_reads << " disk reads, " 
    << disk_writes << " disk writes" << endl;

  // Clean up the page table and disk
  disable_timer();
  page_table_delete(_pt);
  disk_close(disk);

  // free up memory allocated to core map
  free_up_core_map();

  return 0;
} // main()


/**
 * Timer Handler. Implements the clock algorithm
 * Tracks number of times a frame was not used and resets as soon as it is used.
 */
static void timer_handler(int signum){
  if (_pt == NULL) return;

  for (int i = 0; i < nframes; i++) {
    if (core_map[i]->getPageNumber() != -1) {
      if (core_map[i]->isUsed()) {
        not_used[i] = 0;
        core_map[i]->setIsUsed(false);
        page_table_set_entry(_pt, core_map[i]->getPageNumber(), i, PROT_NONE);
      }
      else {
        not_used[i] += 1;
      }
    }
  }

}

/**
 * Finds out the frame that was not recently used by the using the info obtained by the clock algorithm
 * @param k Number of misses by the clock algo to consider as a "not recently used" page
 * @return List of pages that were not recently used.
 */
static vector<int> not_recently_used(int k){
  vector<int> frames_not_recently_used;
  for (int i = 0; i < nframes; i++) {
    if (not_used[i] >= k) {
      frames_not_recently_used.push_back(i);
    }
  }
  if (frames_not_recently_used.empty()) {
    for (int i = 0; i < nframes; i++) frames_not_recently_used.push_back(i);
  }
  return frames_not_recently_used;
}

/**
 * Decides frame to be evicted using the fifo principle.
 * @param evict_candidates contains list of frames that can be evicted.
 * @return oldest frame from evict candidates.
 */
static int get_fifo(vector<int> evict_candidates){
  unsigned long long local_min = ULLONG_MAX;
  int frame_to_evict = -1;
  for (auto it = evict_candidates.begin(); it != evict_candidates.end(); ++it) {
    if (core_map[*it]->getTs() < local_min) {
      frame_to_evict = *it;
      local_min = core_map[*it]->getTs();
    }
  }

  return frame_to_evict;
}

/**
 * Filters out frames that are not dirty.
 * @param frames_not_recently_used contains candidate frames that can be eliminated. Obtained from the clock algorithm
 * @return list of candidate frames that can be evicted.
 */
static vector<int> filter_dirty(vector<int> frames_not_recently_used){
  vector<int> evict_candidates;
  for (auto it = frames_not_recently_used.begin(); it != frames_not_recently_used.end(); ++it) {
    // "Too old" logic is used to prevent old dirty frames from hoarding the physical memory when the page to frame
    // ratio is very high.
    if (!core_map[*it]->isDirty() || core_map[*it]->isTooOld(nframes)) {
      evict_candidates.push_back(*it);
    }
  }
  if (evict_candidates.empty()) return frames_not_recently_used;
  else return evict_candidates;
}

/**
 * Initializes timer and handler.
 */
static void initialize_timer(){
  _sigAction.sa_handler = timer_handler;
  if(sigemptyset (&_sigAction.sa_mask) == FAIL)
  {
    cout << "Error: Couldn't empty mask\n";
    exit(1);
  }
  if(sigaddset(&_sigAction.sa_mask, SIGVTALRM))
  {
    cout << "Error: Couldn't set mask to vt alarm\n";
    exit(1);
  }
  _sigAction.sa_flags = 0;
  if(sigaction(SIGVTALRM,&_sigAction,NULL) == FAIL)
  {
    cout << "Error: Couldn't register action\n";
    exit(1);
  }

  //initialize timer
  _timer.it_value.tv_sec = (int)(clock_gran/MICRO_TO_SECOND);
  _timer.it_value.tv_usec = clock_gran % MICRO_TO_SECOND;
  _timer.it_interval.tv_sec = (int)(clock_gran/MICRO_TO_SECOND);
  _timer.it_interval.tv_usec = clock_gran % MICRO_TO_SECOND;

  if (setitimer(ITIMER_VIRTUAL, &_timer, NULL) == FAIL)
  {
    cout << "Error: Couldn't set timer\n";
    exit(1);
  }
}

/**
 * Disables timer.
 */
static void disable_timer(){
  _timer.it_value.tv_sec = 0;
  _timer.it_value.tv_usec = 0;
  _timer.it_interval.tv_sec = 0;
  _timer.it_interval.tv_usec = 0;

  if (setitimer(ITIMER_VIRTUAL, &_timer, NULL) == FAIL)
  {
    cout << "Error: Couldn't disable timer\n";
    exit(1);
  }
}

/**
 * Disable interrupts. We might want to disable the timer interrupt while updating some data structures.
 */
static void disableInterrupts(){
  sigprocmask(SIG_BLOCK,&_sigAction.sa_mask, NULL);
}

/**
 * Enables interrupts. Used to enable interrupts after the data structure updates have been completed.
 */
static void enableInterrupts(){
  sigprocmask(SIG_UNBLOCK,&_sigAction.sa_mask, NULL);
}


/**
 * Frees up the space allocated for core map entries
 */
static void free_up_core_map(){
  for (int i = 0; i < (int) core_map.size(); i++) {
    delete core_map[i];
    core_map[i] = NULL;
  }
}

/**
 * Reads frame from disk and sets the page table entry
 * @param pt page table
 * @param page page that points to the frame
 * @param frame frame where the data is loaded from disk
 */
static void read_from_disk_and_set_pt_entry(page_table *pt, int page, int frame) {
  disk_read(disk, page, &page_table_get_physmem(pt)[frame * PAGE_SIZE]);
  page_table_set_entry(pt, page, frame, PROT_READ);
  disk_reads++;
}

/**
 * Checks if a page is in memory by checking the pages_in_mem data structure and the permission bit for the page
 * @param page the page that we are checking if is in memory or not
 * @param bits contains the permission bits for the required page
 * @return if a page is in memory
 */
static bool is_page_in_memory(int page, int bits){

  if (pages_in_mem.find(page) != pages_in_mem.end() || bits == PROT_READ){
    return true;
  }
  return false;
}

/**
 * Removes a frame from memory and writes back to disk if required
 * @param pt a pointer to the page table
 * @param get_frame_to_be_evicted
 * @return page that was evicted
 */
static int evict_frame_from_memory(struct page_table *pt, policy get_frame_to_be_evicted){
  if (disk_reads < nframes) { // there are empty frames
    return disk_reads;
  }

  int frame = get_frame_to_be_evicted();
  assert(core_map[frame]->getPageNumber() != -1); // current eviction happens only when full

  if (core_map[frame]->getPerms() & PROT_WRITE) {
    disk_write(disk, core_map[frame]->getPageNumber(), &page_table_get_physmem(pt)[frame*PAGE_SIZE]);
    disk_writes++;
  }
  page_table_set_entry(pt, core_map[frame]->getPageNumber(), 0, 0);

  return frame;
}

/**
 * Uses custom policy to determine the frame that needs to be evicted
 * @return frame number that needs to be evicted
 */
static int get_frame_to_be_evicted_custom_policy(){
  return get_fifo(filter_dirty(not_recently_used(1)));
}

/**
 * Uses random policy to determine the frame that needs to be evicted
 * @return frame number that needs to be evicted
 */
static int get_frame_to_be_evicted_random_policy(){
  return rand() % nframes;
}

/**
 * Uses fifo policy to determine the frame that needs to be evicted
 * @return frame number that needs to be evicted
 */
static int get_frame_to_be_evicted_fifo_policy(){
  assert(!frame_queue.empty());
  int evict_frame = frame_queue.front();
  frame_queue.pop();
  return evict_frame;
}

/**
 * Sets appropriate permissions for a given page. This function is called when a page is in memory
 * but a trap was set for bookkeeping purposes.
 * @param pt Pointer to the page table
 * @param page page that resulted in a sigsev
 * @param frame frame that is mapped to @param page
 * @param current permissions
 */
static void set_perms(page_table *pt, int page, int frame, int bits) {
  int bits_to_set;
  core_map[frame]->setIsUsed(true);
  if (bits == PROT_NONE) { // due to the clock algo
    bits_to_set = core_map[frame]->getPerms();
    not_used[frame] = 0; // doing this as we dont have a high resolution alarm
  }
  else {
    bits_to_set = PROT_READ | PROT_WRITE;
    core_map[frame]->setIsDirty(true);
  }
  page_table_set_entry(pt, page, frame, bits_to_set);
}
