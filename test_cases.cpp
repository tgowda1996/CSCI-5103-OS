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
    cout<<"Starting thread : "<<my_tid<<endl;
    for (int i = args.start; i <= args.end; i++) {
        cout<<"Thread : "<<my_tid<<" : "<<i<<endl;
	for (long j = 0; j < 5000000; j++);
    }
    cout << " Finished running thread : " << my_tid << endl;
    return NULL;
}

void test_yield_and_scheduler();
void test_join_based_main();
void test_timer_based_preemption(); 

int main(int argc, char *argv[]){
    // test_yield_and_scheduler();
    // test_join_based_main();
    test_timer_based_preemption();
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

void test_join_based_main() {
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
    
    arguments_for_worker args = {1,1,2};
    threads[0] = uthread_create(worker1, &args);
    arguments_for_worker args1 = {11,15,3};
    threads[1] = uthread_create(worker1, &args1);
    arguments_for_worker args2 = {21,25,4};
    threads[2] = uthread_create(worker1, &args2);

    cout<<"Threads Created\n";
    for (int i = 0; i < 3; i++) {
	    cout << "Joining on - " << threads[i] << "\n";
	    uthread_join(threads[i], NULL);
    }

    cout<<"Exiting"<<endl;
    delete[] threads;
}


void test_timer_based_preemption() {
    // Default to 1 ms time quantum
    int quantum_usecs = 100; // keeping quantum_usecs large enough to let the code only work based on yield

    int *threads = new int[3];
    // Init user thread library
    int ret = uthread_init(quantum_usecs);
    unsigned long *result;
    if (ret != 0) {
        cerr << "uthread_init FAIL!\n" << endl;
        exit(1);
    }
    
    arguments_for_worker args = {1,3,2};
    threads[0] = uthread_create(worker2, &args);
    arguments_for_worker args1 = {11,15,3};
    threads[1] = uthread_create(worker2, &args1);
    arguments_for_worker args2 = {21,25,4};
    threads[2] = uthread_create(worker2, &args2);

    cout<<"Threads Created\n";
    for (int i = 0; i < 3; i++) {
	    cout << "Joining on - " << threads[i] << "\n";
	    uthread_join(threads[i], NULL);
    }

    cout<<"Exiting"<<endl;
    delete[] threads;
}
