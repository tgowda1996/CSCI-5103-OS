#include "CondVar.h"
#include "uthread_private.h"

using namespace std;
// TODO

CondVar::CondVar(){
    // empty constructor
}

void CondVar::wait(Lock &lock) {
    disableInterrupts();
    // cout << "Inside condition variable\n";
    cond_var_waiting_queue.push_back(running);
    running->setState(BLOCK);
    // assert(nextThreadToWakeUp == NULL);
    lock._unlock();
    // cout<<"thread " << running->getId() << " waiting on condition variable\n"; 
    switchThreads();
    // lock is still set to busy
    running->setState(RUNNING);
    // assert(wake_up_order.empty() == false);
//    if (!wake_up_order.empty()) { // should not be empty. add assert
    lock._signal(wake_up_order.front());
    wake_up_order.pop_front();
//    }
    enableInterrupts();
}

void CondVar::signal() {
    disableInterrupts();
    if (!cond_var_waiting_queue.empty()) {
    	TCB* next = cond_var_waiting_queue.front();
    	cond_var_waiting_queue.pop_front();
    	wake_up_order.push_back(running);
    	running->setState(BLOCK);
    	switchToThread(next);
    }
    enableInterrupts();
}

void CondVar::broadcast() {
    disableInterrupts();
    if (!cond_var_waiting_queue.empty()){
    	TCB* next = cond_var_waiting_queue.front();
    	cond_var_waiting_queue.pop_front();
    	while(!cond_var_waiting_queue.empty()) {
       		wake_up_order.push_back(cond_var_waiting_queue.front());
		cond_var_waiting_queue.pop_front();
    	}
    	wake_up_order.push_back(running);
    	running->setState(BLOCK);
    	switchToThread(next);
    }
    enableInterrupts();
}
