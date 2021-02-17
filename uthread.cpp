#include "uthread.h"
#include "TCB.h"
#include <cassert>
#include <deque>
#include <set>
#include <map> 

#define MAX_THREAD_LIMIT 10

using namespace std;

// Finished queue entry type
typedef struct finished_queue_entry {
  TCB *tcb;             // Pointer to TCB
  void *result;         // Pointer to thread result (output)
} finished_queue_entry_t;

// Join queue entry type
typedef struct join_queue_entry {
  TCB *tcb;             // Pointer to TCB
  int waiting_for_tid;  // TID this thread is waiting on
} join_queue_entry_t;

// You will need to maintain structures to track the state of threads
// - uthread library functions refer to threads by their TID so you will want
//   to be able to access a TCB given a thread ID
// - Threads move between different states in their lifetime (READY, BLOCK,
//   FINISH). You will want to maintain separate "queues" (doesn't have to
//   be that data structure) to move TCBs between different thread queues.
//   Starter code for a ready queue is provided to you
// - Separate join and finished "queues" can also help when supporting joining.
//   Example join and finished queue entry types are provided above

// Queues
static deque<TCB*> ready_queue;
static deque<finished_queue_entry_t*> finished_queue;
static deque<join_queue_entry_t*> join_queue;

// general data structures
static deque<int> threadIdsAvailable;
static map<int,TCB*> idToTcb;

static TCB *runningThread;


// Interrupt Management --------------------------------------------------------

// Start a countdown timer to fire an interrupt
static void startInterruptTimer()
{
        // TODO
}

// Block signals from firing timer interrupt
static void disableInterrupts()
{
        // TODO
}

// Unblock signals to re-enable timer interrupt
static void enableInterrupts()
{
        // TODO
}


// Queue Management ------------------------------------------------------------

// Add TCB to the back of the ready queue
void addToQueue(deque<TCB*> &queueToModify, TCB *tcb)
{
        queueToModify.push_back(tcb);
}

// Removes and returns the first TCB on the ready queue
// NOTE: Assumes at least one thread on the ready queue
TCB* popFromQueue(deque<TCB*> &queueToModify)
{
        assert(!queueToModify.empty());

        TCB *tcb = queueToModify.front();
        queueToModify.pop_front();
        return tcb;
}

// Removes the thread specified by the TID provided from the ready queue
// Returns 0 on success, and -1 on failure (thread not in ready queue)
int removeFromQueue(int tid, deque<TCB*>& queueToModify)
{
        for (deque<TCB*>::iterator iter = queueToModify.begin(); iter != queueToModify.end(); ++iter)
        {
                if (tid == (*iter)->getId())
                {
                        queueToModify.erase(iter);
                        return 0;
                }
        }

        // Thread not found
        return -1;
}

int isPresentInFinishedQueue(int tid) {
        for (deque<finished_queue_entry_t*>::iterator iter = finished_queue.begin(); iter != finished_queue.end(); ++iter)
        {
                if (tid == (*iter)->tcb->getId())
                {
                        return 1;
                }
        }

        // Thread not found
        return 0;
}

// Helper functions ------------------------------------------------------------

// Switch to the next ready thread
static void switchThreads()
{
        // TODO
    volatile int flag = 0;
    runningThread->saveContext();
    cout << "SWITCH" << endl; //: currentThread =  << runningThread->getId() <<  << flag << endl;

    if (flag == 1) {
	//cout << "flag check for - " << runningThread->getId(); 
        return;
    }

    flag = 1;
    runningThread->setState(State::READY);
    addToQueue(ready_queue, runningThread);

    TCB *nextThread = popFromQueue(ready_queue);
    while(isPresentInFinishedQueue(nextThread->getId())) {
        removeFromQueue(nextThread->getId(), ready_queue); // dont want it to be in the ready queue again.
        nextThread = popFromQueue(ready_queue);
    }
    runningThread = nextThread;
    // cout<<"Loading "<<nextThread->getId()<<endl;
    nextThread->loadContext();
}


// Library functions -----------------------------------------------------------

// The function comments provide an (incomplete) summary of what each library
// function must do

// Starting point for thread. Calls top-level thread function
void stub(void *(*start_routine)(void *), void *arg)
{
        // TODO
    void *return_value = (*start_routine)(arg);
    uthread_exit(return_value);
}

int uthread_init(int quantum_usecs)
{
        // Initialize any data structures
        // Setup timer interrupt and handler
        // Create a thread for the caller (main) thread
    for (int i = 100; i < 100 + MAX_THREAD_LIMIT; i++) {
        threadIdsAvailable.push_back(i);
    }
    TCB *t0 = new TCB(0, State::RUNNING);
    runningThread = t0;
    idToTcb.insert(pair<int, TCB*>(0, t0));
    return 0;
}

// supposed to return a TID
int uthread_create(void* (*start_routine)(void*), void* arg)
{
        // Create a new thread and add it to the ready queue
        int tid = threadIdsAvailable.front();
        threadIdsAvailable.pop_front();

        TCB *tcb = new TCB(tid, start_routine, arg, State::READY);
        idToTcb.insert(pair<int, TCB*>(tid, tcb));
        addToQueue(ready_queue, tcb);
	return tid;
}

int uthread_join(int tid, void **retval)
{
        // If the thread specified by tid is already terminated, just return
        // If the thread specified by tid is still running, block until it terminates
        // Set *retval to be the result of thread if retval != nullptr

}

int uthread_yield(void)
{
        // TODO
    //cout<<"yeid for - "<<runningThread->getId()<<"\n";
    switchThreads();
    if (ready_queue.size() == 0) return 0; // will happen when main is the only thread remaining
    else return 1;
    // return 1; // this needs to be checked.
}

void uthread_exit(void *retval)
{
        // If this is the main thread, exit the program
        // Move any threads joined on this thread back to the ready queue
        // Move this thread to the finished queue

        // Intermediate implementation
    finished_queue_entry_t entry = {runningThread, retval};
    finished_queue.push_back(&entry); // search how to implement generic queues templates interface kind
    uthread_yield(); // adding this as we dont have join and we dont want the process to stop after one of the threads have completed executing.
}

int uthread_suspend(int tid)
{
        // Move the thread specified by tid from whatever state it is
        // in to the block queue
}

int uthread_resume(int tid)
{
        // Move the thread specified by tid back to the ready queue
}

int uthread_self()
{
        // TODO
	return runningThread->getId();
}

int uthread_get_total_quantums()
{
        // TODO
}

int uthread_get_quantums(int tid)
{
        // TODO
}
