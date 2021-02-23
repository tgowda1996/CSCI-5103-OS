#include "uthread.h"
#include <iostream>
#include <unistd.h>

using namespace std;

typedef struct arguments_for_worker {
    int start;
    int end;
    int yeild_mod;
    int thread_to_be_suspended;
} arguments_for_worker;

typedef struct metadata_tester_args {
    int tid;
    int quantum;
} metadata_tester_args;

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

void *worker3(void *arg) {
    int my_tid = uthread_self();
    metadata_tester_args args = *(metadata_tester_args*)arg;
    cout << "Thread " << my_tid << " uthread self and tid from the main thread are equal? " << (my_tid==args.tid ? "True\n" : "False\n");
    cout << "Thread " << my_tid << " uthread get quantums and quantum from the main thread are equal? " <<(uthread_get_quantums(my_tid)==args.quantum ? "True\n" : "False\n");
    return NULL;
}

void *worker4(void *arg) {
    int my_tid = uthread_self();
    arguments_for_worker args = *(arguments_for_worker*)arg;
    if (args.thread_to_be_suspended != -1){
	    cout << "thread " << my_tid << " is suspending thread " << args.thread_to_be_suspended << "\n";
	    uthread_suspend(args.thread_to_be_suspended);
    }
    cout<<"Starting thread : "<<my_tid<<endl;
    for (int i = args.start; i <= args.end; i++) {
        cout<<"Thread : "<<my_tid<<" : "<<i<<endl;
	for (int j = 0; j < 5000000; j++);
	// usleep(5);
        // if (i % args.yeild_mod == 0) {
            //uthread_yield();
	    //cout<<"Current thread : "<<my_tid<<endl;
        // }
    }
    return NULL;
}

void test_yield_and_scheduler();
void test_join_based_main();
void test_timer_based_preemption();
void test_self_and_quantum_apis();
void test_suspend_by_different_thread_and_resume();
void test_suspend_by_same_thread_and_resume();


int main(int argc, char *argv[]){
    //test_yield_and_scheduler();
    //test_join_based_main();
    //test_timer_based_preemption();
    //test_self_and_quantum_apis();
    //test_suspend_by_different_thread_and_resume();
    test_suspend_by_same_thread_and_resume();
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

void test_self_and_quantum_apis() {
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
    
    metadata_tester_args args = {0,quantum_usecs};
    threads[0] = uthread_create(worker3, &args);
    args.tid = threads[0];
    metadata_tester_args args1 = {0,quantum_usecs};    
    threads[1] = uthread_create(worker3, &args1);
    args1.tid = threads[1];

    cout<<"Threads Created\n";
    cout << "Total quantums expected " << quantum_usecs*3 << " library output - " << uthread_get_total_quantums() << "\n";
    for (int i = 0; i < 2; i++) {
	    cout << "Joining on - " << threads[i] << "\n";
	    uthread_join(threads[i], NULL);
            cout << "Total quantums expected " << quantum_usecs*(3-i-1) << " library output - " << uthread_get_total_quantums() << "\n";

    }

    cout<<"Exiting"<<endl;
    delete[] threads;
}

void test_suspend_by_different_thread_and_resume() {
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
    
    arguments_for_worker args = {1,3,2,-1};
    threads[0] = uthread_create(worker4, &args);
    arguments_for_worker args1 = {11,15,3,-1};
    threads[1] = uthread_create(worker4, &args1);
    arguments_for_worker args2 = {21,25,4,-1};
    threads[2] = uthread_create(worker4, &args2);
    args1.thread_to_be_suspended = threads[2];

    cout<<"Threads Created\n";
    for (int i = 0; i < 3; i++) {
	    cout << "Joining on - " << threads[i] << "\n";
	    uthread_join(threads[i], NULL);
	    if (i == 1) {
		    cout<<"Resuming thread " << threads[2] << "\n";
		    uthread_resume(threads[2]);
            }
    }

    cout<<"Exiting"<<endl;
    delete[] threads;
}

void test_suspend_by_same_thread_and_resume() {
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
    
    arguments_for_worker args = {1,5,2,-1};
    threads[0] = uthread_create(worker4, &args);
    arguments_for_worker args1 = {11,15,3,-1};
    threads[1] = uthread_create(worker4, &args1);
    arguments_for_worker args2 = {21,25,4,-1};
    threads[2] = uthread_create(worker4, &args2);
    args1.thread_to_be_suspended = threads[1];

    cout<<"Threads Created\n";
    for (int i = 0; i < 3; i++) {
	    cout << "Joining on - " << threads[i] << "\n";
	    uthread_join(threads[i], NULL);
	    if (i == 0) {
		    cout<<"Resuming thread " << threads[1] << "\n";
		    uthread_resume(threads[1]);
            }
    }

    cout<<"Exiting"<<endl;
    delete[] threads;
}


