#include "uthread.h"
#include <iostream>
#include <unistd.h>

using namespace std;

typedef struct arguments_for_worker {
    int start;
    int end;
    int yeild_mod;
} arguments_for_worker;

void *worker(void *arg) {
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

int main(int argc, char *argv[]) {
    // Default to 1 ms time quantum
    int quantum_usecs = 10000;

    int *threads = new int[3];
    // Init user thread library
    int ret = uthread_init(quantum_usecs);
    unsigned long *result;
    if (ret != 0) {
        cerr << "uthread_init FAIL!\n" << endl;
        exit(1);
    }
    
    arguments_for_worker args = {1,50,5};
    threads[0] = uthread_create(worker, &args);
    arguments_for_worker args1 = {51,100,10};
    threads[1] = uthread_create(worker, &args1);
    arguments_for_worker args2 = {100,250,15};
    threads[2] = uthread_create(worker, &args2);

    cout<<"Threads Created\n";
    for (int i = 0; i < 3; i++){
	cout << "Joining on - " << threads[i] << "\n";
        uthread_join(threads[i], (void **)&result);
	if (i == 1) {
		cout<<"Calling uthread resume\n";
		uthread_resume(102);
	}
    }
    //while(uthread_yield()){
        //cout<<"In main thread"<<endl;
    //}
    cout<<"Exiting"<<endl;
    delete[] threads;

    return 0;
}
