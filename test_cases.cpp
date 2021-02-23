#include "uthread.h"
#include <iostream>
#include <unistd.h>

using namespace std;

typedef struct arguments_for_worker {
    int start;
    int end;
    int yeild_mod;
} arguments_for_worker;

void *worker1(void *arg) {
    int my_tid = uthread_self();
    arguments_for_worker args = *(arguments_for_worker*)arg;
    cout<<"Starting thread : "<<my_tid<<endl;
    for (int i = args.start; i <= args.end; i++) {
        cout<<"Thread : "<<my_tid<<" : "<<i<<endl;
	//for (int j = 0; j < 1000000; j++);
        if ((i-args.start+1) % args.yeild_mod == 0) {
            uthread_yield();
	    cout<<"Current thread : "<<my_tid<<endl;
        }
    }
    return NULL;
}

void *worker2(void *arg) {
    int my_tid = uthread_self();
    arguments_for_worker args = *(arguments_for_worker*)arg;
    if (my_tid == 101){
	    uthread_suspend(102);
    }
    cout<<"Starting thread : "<<my_tid<<endl;
    for (int i = args.start; i <= args.end; i++) {
        cout<<"Thread : "<<my_tid<<" : "<<i<<endl;
	for (int j = 0; j < 1000000; j++);
	// usleep(5);
        // if (i % args.yeild_mod == 0) {
            //uthread_yield();
	    //cout<<"Current thread : "<<my_tid<<endl;
        // }
    }
    return NULL;
}

void test_yield_and_scheduler();

int main(int argc, char *argv[]){
    test_yield_and_scheduler();
    return 0;
}

void test_yield_and_scheduler() {
    // Default to 1 ms time quantum
    int quantum_usecs = 10000; // keeping quantum_usecs large enough to let the code only work based on yield

    int *threads = new int[3];
    // Init user thread library
    int ret = uthread_init(quantum_usecs);
    unsigned long *result;
    if (ret != 0) {
        cerr << "uthread_init FAIL!\n" << endl;
        exit(1);
    }
    
    arguments_for_worker args = {1,5,2};
    threads[0] = uthread_create(worker1, &args);
    arguments_for_worker args1 = {11,15,3};
    threads[1] = uthread_create(worker1, &args1);
    arguments_for_worker args2 = {21,25,4};
    threads[2] = uthread_create(worker1, &args2);

    cout<<"Threads Created\n";
    while(uthread_yield()){
        cout<<"In main thread"<<endl;
    }

    for (int i = 0; i < 3; i ++) {
        uthread_join(threads[i], NULL);
    }
    cout<<"Exiting"<<endl;
    delete[] threads;
}
