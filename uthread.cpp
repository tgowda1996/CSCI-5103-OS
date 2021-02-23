#include "uthread.h"
#include "TCB.h"
#include <cassert>
#include <deque>
#include <set>
#include <map> 

#define MAX_THREAD_LIMIT 100

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
static deque<TCB*> block_queue;
static deque<finished_queue_entry_t*> finished_queue;
static deque<join_queue_entry_t*> join_queue;

// general data structures
static deque<int> threadIdsAvailable;
static map<int,TCB*> idToTcb;
static int quantum;

static TCB *runningThread;


// Interrupt Management --------------------------------------------------------

// Start a countdown timer to fire an interrupt
static void startInterruptTimer()
{
	// TODO implement the scheduler function.
	
   // setting up timer
   struct itimerval it_val;
   it_val.it_value.tv_sec = 0;
   it_val.it_value.tv_usec = runningThread->getQuantum();
   it_val.it_interval.tv_sec = 0;
   it_val.it_interval.tv_usec = 0; //setting interval to zero so that timer doesnt restart on its own
   setitimer(ITIMER_VIRTUAL, &it_val, NULL);
}

// Block signals from firing timer interrupt
static void disableInterrupts()
{
        // TODO
    //cout << "Disabling Interrupts\n";
    sigset_t block_virtual_alarm;
    sigemptyset(&block_virtual_alarm);
    sigaddset(&block_virtual_alarm, SIGVTALRM);

    sigprocmask(SIG_BLOCK, &block_virtual_alarm, NULL);
}

// Unblock signals to re-enable timer interrupt
static void enableInterrupts()
{
        // TODO
    //cout << "Enabling Interrupts\n";
    sigset_t unblock_virtual_alarm;
    sigemptyset(&unblock_virtual_alarm);
    sigaddset(&unblock_virtual_alarm, SIGVTALRM);

    sigprocmask(SIG_UNBLOCK, &unblock_virtual_alarm, NULL);
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

// Removes the thread specified by the TID provided from the ready queue
// Returns 0 on success, and -1 on failure (thread not in ready queue)
int removeFromFinishedQueue(int tid, deque<finished_queue_entry*>& queueToModify)
{
        for (deque<finished_queue_entry*>::iterator iter = queueToModify.begin(); iter != queueToModify.end(); ++iter)
        {
                if (tid == (*iter)->tcb->getId())
                {
                        queueToModify.erase(iter);
                        return 0;
                }
        }

        // Thread not found
        return -1;
}

int removeFromJoinQueue(join_queue_entry_t *entry)
{
        for (deque<join_queue_entry_t*>::iterator iter = join_queue.begin(); iter != join_queue.end(); ++iter)
        {
                if (entry->tcb->getId() == (*iter)->tcb->getId())
                {
                        join_queue.erase(iter);
                        return 0;
                }
        }

        // Thread not found
        return -1;
}

static finished_queue_entry* isPresentInFinishedQueue(int tid) {
        for (deque<finished_queue_entry_t*>::iterator iter = finished_queue.begin(); iter != finished_queue.end(); ++iter)
        {
                if (tid == (*iter)->tcb->getId())
                {
                        return (*iter);
                }
        }

        // Thread not found
        return NULL;
}

static join_queue_entry_t* getThreadWaitingOn(int tid) {
        for (deque<join_queue_entry_t*>::iterator iter = join_queue.begin(); iter != join_queue.end(); ++iter)
        {
                if (tid == (*iter)->waiting_for_tid)
                {
                        return (*iter);
                }
        }

        // Thread not found
        return NULL;
}

// Helper functions ------------------------------------------------------------

// Switch to the next ready thread
static void switchThreads()
{
        // TODO
    volatile int flag = 0;
    // runningThread->saveContext();
    getcontext(&(runningThread->_context));
    //cout << "SWITCH" << endl; //: currentThread =  << runningThread->getId() <<  << flag << endl;

    if (flag == 1) {
	// cout << "flag check for - " << runningThread->getId()<<endl; 
	startInterruptTimer();
	enableInterrupts();
	return;
    }

    disableInterrupts();
    flag = 1;
    if (runningThread->getState() == State::RUNNING){
        runningThread->setState(State::READY);
        addToQueue(ready_queue, runningThread);
    }

    TCB *nextThread = popFromQueue(ready_queue);
    while(isPresentInFinishedQueue(nextThread->getId()) != NULL) {
        removeFromQueue(nextThread->getId(), ready_queue); // dont want it to be in the ready queue again.
        nextThread = popFromQueue(ready_queue);
    }
    runningThread = nextThread;
    runningThread->setState(State::RUNNING); // Setting state before setting context
    cout<<"Loading "<<nextThread->getId()<<endl;
    // nextThread->loadContext();
    setcontext(&(nextThread->_context));
}

static void scheduler_function(int signum) {
	// TODO
    cout<<"Timer interrupt - inside the thread - " << runningThread->getId() << endl;
    uthread_yield();
}


// Library functions -----------------------------------------------------------

// The function comments provide an (incomplete) summary of what each library
// function must do

// Starting point for thread. Calls top-level thread function
void stub(void *(*start_routine)(void *), void *arg)
{
        // TODO
    
    enableInterrupts(); // makes sure that we are enabling interrupts before entering user code.
    startInterruptTimer(); // Start timer before entering user code. For first run.
    void *return_value = (*start_routine)(arg);
    uthread_exit(return_value);
}

int uthread_init(int quantum_usecs)
{
        // Initialize any data structures
        // Setup timer interrupt and handler
        // Create a thread for the caller (main) thread

    quantum = quantum_usecs;
    // registering the handler function for SIGVTALRM
    struct sigaction timer_signal;
    timer_signal.sa_handler = scheduler_function;
    sigemptyset(&timer_signal.sa_mask);
    timer_signal.sa_flags = 0;
    
    sigaction(SIGVTALRM, &timer_signal, NULL);

    for (int i = 100; i < 100 + MAX_THREAD_LIMIT; i++) {
        threadIdsAvailable.push_back(i);
    }
    TCB *t0 = new TCB(0, State::RUNNING);
    idToTcb.insert(pair<int, TCB*>(0, t0));
    t0->increaseQuantum(quantum_usecs);
    getcontext(&t0->_context);
    runningThread = t0;

    startInterruptTimer();

    return 0;
}

// supposed to return a TID
int uthread_create(void* (*start_routine)(void*), void* arg)
{
        // Create a new thread and add it to the ready queue
	
	// Check to make sure that thread limit hasnt been reached
	assert(!threadIdsAvailable.empty());
	disableInterrupts();
        int tid = threadIdsAvailable.front();
        threadIdsAvailable.pop_front();

        TCB *tcb = new TCB(tid, start_routine, arg, State::READY);
	tcb->increaseQuantum(quantum);
        idToTcb.insert(pair<int, TCB*>(tid, tcb));
        addToQueue(ready_queue, tcb);
	enableInterrupts();
	return tid;
}

int uthread_join(int tid, void **retval)
{
        // If the thread specified by tid is already terminated, just return
        // If the thread specified by tid is still running, block until it terminates
        // Set *retval to be the result of thread if retval != nullptr
    void *result;
    finished_queue_entry* finished_entry = isPresentInFinishedQueue(tid);
    if (finished_entry == NULL) {
	cout<< "Thread " << tid << " hasnt finished executing yet. Moving " << runningThread->getId() << " to blocked state\n";
	disableInterrupts();
	runningThread->setState(State::BLOCK);
	join_queue_entry_t jqe = {runningThread, tid};
	join_queue.push_back(&jqe);
	// since in running thread, we dont need to edit the ready queue as we are changing the state.
	enableInterrupts();
        uthread_yield();
	// will reach here when tid is in finished queue
        finished_entry = isPresentInFinishedQueue(tid);
    }

    // If it reached here it means that the thread tid has finished.
    disableInterrupts();
    if (retval != NULL) {
        *retval = finished_entry->result;
    }
    removeFromFinishedQueue(tid, finished_queue);
    removeFromQueue(tid, ready_queue);
    // delete finished_entry->tcb;
    // delete finished_entry;
    idToTcb.erase(tid);
    threadIdsAvailable.push_back(tid);
    enableInterrupts();
    return 1;
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
    disableInterrupts();
    finished_queue_entry_t entry = {runningThread, retval};
    finished_queue.push_back(&entry); // search how to implement generic queues templates interface kind
    runningThread->setState(State::BLOCK);
    cout<<"Thread "<<runningThread->getId()<<" has completed and moving it to block state\n";
    join_queue_entry_t *threadWaitingOnCurrent = getThreadWaitingOn(runningThread->getId());
    if (threadWaitingOnCurrent != NULL){
	cout<<"Changing thread "<<threadWaitingOnCurrent->tcb->getId()<<"'s state to READY and moving it to ready_queue\n";
    	threadWaitingOnCurrent->tcb->setState(State::READY);
	addToQueue(ready_queue, threadWaitingOnCurrent->tcb);
	removeFromJoinQueue(threadWaitingOnCurrent);
    }
    enableInterrupts();
    uthread_yield(); // calling yield as it is in block state and needs to give up processor
}

int uthread_suspend(int tid)
{
        // Move the thread specified by tid from whatever state it is
        // in to the block queue
    
    //check if tid is valid and is currently running. And it should not be in finished queue
    cout<<"suspending " << tid << " called by " << runningThread->getId() << endl;
    if (idToTcb.find(tid) == idToTcb.end() || isPresentInFinishedQueue(tid) != NULL) {
	    return -1;
	    cout << "Returning -1 as some issue" << endl;
    }
    TCB* tcbToBeSuspended = idToTcb[tid];
    disableInterrupts();
    bool wasRunning = runningThread->getState() == State::RUNNING;
    //addToQueue(block_queue, tcbToBeSuspended);
    tcbToBeSuspended->setState(State::BLOCK); 
    removeFromQueue(tid, ready_queue);
    if (wasRunning){
	    cout << "Thread to be suspended is currently running";
	    enableInterrupts();
	    uthread_yield();
	    return 1;
    }
    enableInterrupts();
    return 1;
}

int uthread_resume(int tid)
{
    cout << "Resuming thread " << tid << "\n";
        // Move the thread specified by tid back to the ready queue
    if (idToTcb.find(tid) == idToTcb.end()) {
	    return -1;
    }
    TCB* tcbToBeResumed = idToTcb[tid];
    if (tcbToBeResumed->getState() != State::BLOCK) {
	    return -1;
    }
    cout << "Validation passed\n";
    disableInterrupts();
    //removeFromQueue(tid, block_queue);
    tcbToBeResumed->setState(State::READY);
    addToQueue(ready_queue, tcbToBeResumed);
    enableInterrupts();
    return 1;
}

int uthread_self()
{
        // TODO
	return runningThread->getId();
}

int uthread_get_total_quantums()
{
        // TODO
    return idToTcb.size()*quantum;
    
}

int uthread_get_quantums(int tid)
{
        // TODO
    return idToTcb[tid]->getQuantum();
}
