#include "uthread.h"
#include "Lock.h"
#include <iostream>


using std::cout;
using std::endl;

struct loop_args{
    int i, j, k;
};

loop_args* getLoopArgs(int i, int j, int k);

static Lock lock;

void * worker_long_work(void* args) {
    int my_tid = uthread_self();
    loop_args* largs = (loop_args*)args;
    cout<<"Starting thread : "<<my_tid<<endl;
    int a = 0;
    for (int i = 0; i < largs->i; i++) {
        for (int j = 0; j < largs->j; j++) {
            for (int k = 0; k < largs->k; k++);
        }
    }
    cout<<"Exiting thread " << my_tid << endl;
    return NULL;
}

void * worker_lock_acquire(void* args) {
    int my_tid = uthread_self();
    cout<<"Acquiring lock for " << my_tid << endl;
    lock.lock();
    uthread_yield();
    lock.unlock();
    cout<<"Exiting thread " << my_tid << endl;
}

int main(){
    uthread_init(100);
    int threads[6];
    loop_args* largs = getLoopArgs(100, 100, 100);
    threads[0] = uthread_create(worker_lock_acquire, NULL);
    uthread_decrease_priority(threads[0]);
    uthread_yield();

    threads[1] = uthread_create(worker_lock_acquire, NULL);
    uthread_increase_priority(threads[0]);
    for (int i = 0; i < 2; i++) {
        threads[2+i] = uthread_create(worker_long_work, loop_args);
        uthread_increase_priority(threads[2+i]);
    }

    for (int i = 0; i < 2; i++) {
        threads[4+i] = uthread_create(worker_long_work, loop_args);
    }

    cout<<"Threads Created\n";
    for (int i = 0; i < 6; i++) {
        cout << "Joining on - " << threads[i] << "\n";
        uthread_join(threads[i], NULL);
    }

    delete largs;
    cout<<"Exiting"<<endl;
}