#include "Lock.h"
#include "uthread_private.h"

// TODO

using namespace std;

Lock::Lock(){
    value = 0;
    nextThreadToWakeUp = NULL;
}

void Lock::lock(){
    disableInterrupts();
    if (value == 1) {
        waiting_queue.push_back(running);
        running->setState(BLOCK);
        switchThreads(); // not calling yield as I dont want to add it to the ready queue
        running->setState(RUNNING);
    }
    else {
	// cout<<"Lock is free. Acquiring lock\n";
        value = 1;
    }
    running->increaseLockCount();
    enableInterrupts();
}

void Lock::unlock(){
    disableInterrupts();
    running->decreaseLockCount();
    _unlock();
    enableInterrupts();
}

void Lock::_unlock(){
    TCB* threadToBeUnlocked;
    if (nextThreadToWakeUp != NULL) {
        threadToBeUnlocked = nextThreadToWakeUp;
        nextThreadToWakeUp = NULL; // resetting the state
        running->setState(READY);
        addToReady(running);
        switchToThread(threadToBeUnlocked);
    }
    else if (!waiting_queue.empty()) {
        threadToBeUnlocked = waiting_queue.front();
	waiting_queue.pop_front();
        threadToBeUnlocked->setState(READY);
        addToReady(threadToBeUnlocked);
    }
    else {
        value = 0;
    }
}

void Lock::_signal(TCB* tcb){
    nextThreadToWakeUp = tcb;
}
