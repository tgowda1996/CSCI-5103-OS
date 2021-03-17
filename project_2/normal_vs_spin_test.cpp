#include "uthread.h"
#include "Lock.h"
#include "SpinLock.h"
#include "iostream"
#include <iostream>
#include <chrono>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>


using std::cout;
using std::endl;

#define FIRST_LOOP 1000
#define SECOND_LOOP 1000
#define THIRD_LOOP 1000

struct loop_args{
    int i, j, k;
};

struct io_args {
    const char* fn;
};

loop_args* getLoopArgs(int i, int j, int k);
io_args* getIOArgs(const char * str);

void testing_code(int num_threads, int type, int first_loop, int second_loop);

static Lock lock;
static SpinLock spinLock;

void * worker_simulation_long_calculation_normal(void* args) {
    int my_tid = uthread_self();
    loop_args* largs = (loop_args*)args;
    cout<<"Starting thread : "<<my_tid<<endl;
    int a = 0;
    for (int i = 0; i < largs->i; i++) {
        lock.lock()
        for (int j = 0; j < largs->j; j++);
        lock.unlock()
    }
    return NULL;
}

void * worker_simulation_long_calculation_spin(void* args) {
    int my_tid = uthread_self();
    loop_args* largs = (loop_args*)args;
    cout<<"Starting thread : "<<my_tid<<endl;
    int a = 0;
    for (int i = 0; i < largs->i; i++) {
        spinLock.lock()
        for (int j = 0; j < largs->j; j++);
        spinLock.unlock()
    }
    return NULL;
}

int main(){
    uthread_init(100);
    int num_threads = 10, type = 1, first_loop = 100, second_loop = 100;
    testing_code(num_threads, type, first_loop, second_loop); // this can be put in a loop and tuned with parameters
    cout<<"Exiting"<<endl;
}

void testing_code(int num_threads, int type, int first_loop, int second_loop) {
    int * threads = new int[num_threads];
    auto t0 = std::chrono::high_resolution_clock::now();
    loop_args* largs = getLoopArgs(first_loop, second_loop, 0);
    for (int i = 0; i < num_threads; i++) {
        if (type == 1){
            threads[i] = uthread_create(worker_simulation_long_calculation_normal, (void *)largs);
        }
        else {
            threads[i] = uthread_create(worker_simulation_long_calculation_spin, (void *)largs);
        }
    }


    cout<<"Threads Created\n";
    for (int i = 0; i < num_threads; i++) {
        cout << "Joining on - " << threads[i] << "\n";
        uthread_join(threads[i], NULL);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> secs = t0-t1;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(secs);
    std::cout << ms.count() << "ms\n";
    delete largs;
}

loop_args* getLoopArgs(int i, int j, int k){
    loop_args* args = new loop_args;
    args->i = i;
    args->j = j;
    args->k = k;
    return args;
}
