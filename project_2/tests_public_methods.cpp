#include "uthread.h"
#include "Lock.h"
#include "SpinLock.h"
#include "CondVar.h"
#include "iostream"
#include <iostream>

static Lock lock;
static SpinLock spinLock;
static CondVar condVar;

void check_conVar_and_hoare_semantics();
void check_conVar_broadcast_and_hoare_semantics();
void check_lock_unlock();
void check_spin_lock_unlock();

using std::cout;
using std::endl;

void * worker_wait_for_signal(void* args) {
    int my_tid = uthread_self();
    lock.lock();
    cout<<"Thread " << my_tid << " waiting\n";
    condVar.wait(lock);
    cout<<"got a signal. Thread "<<my_tid<<" starts working\n";
    lock.unlock();
    return NULL;
}

void * signalOtherThread(void* args) {
    int my_tid = uthread_self();
    lock.lock();
    cout<<"Starting thread " << my_tid << "\n";
    cout<<"Thread "<< my_tid <<" working\n";
    condVar.signal();
    cout<<"Thread "<<my_tid<<" work after signal\n";
    lock.unlock();
    return NULL;
}

void * broadcastSignal(void* args) {
    int my_tid = uthread_self();
    lock.lock();
    cout<<"Starting thread " << my_tid << "\n";
    cout<<"Thread "<< my_tid <<" working\n";
    condVar.broadcast();
    cout<<"Thread "<<my_tid<<" work after signal\n";
    lock.unlock();
    return NULL;
}

void * work_thread(void* args) {
    int my_tid = uthread_self();
    lock.lock();
    cout<<"Thread "<<my_tid<<" working\n";
    lock.unlock();
    return NULL;
}

void * work_thread_lock_testing(void* args) {
    int my_tid = uthread_self();
    cout<<"Starting thread "<<my_tid<<endl;
    lock.lock();
    cout<<"Thread "<<my_tid<<" starts working\n";
    for (int i = 0; i < 1000; i++)
        for (int j = 0; j < 100; j++)
            for (int k = 0; k < 100; k++);
    cout<<"Thread "<<my_tid<<" finished working\n";
    lock.unlock();
    return NULL;
}

void * work_thread_spin_lock_testing(void* args) {
    int my_tid = uthread_self();
    cout<<"Starting thread "<<my_tid<<endl;
    spinLock.lock();
    cout<<"Thread "<<my_tid<<" starts working\n";
    for (int i = 0; i < 1000; i++)
        for (int j = 0; j < 100; j++)
            for (int k = 0; k < 100; k++);
    cout<<"Thread "<<my_tid<<" finished working\n";
    spinLock.unlock();
    return NULL;
}

int main(int argc, char* argv[]){
    uthread_init(100);

    if (argc < 2) {
	    std::cerr << "Usage: ./test_public_methods <test_case_number>" << endl;
	    exit(1);
    }
    int tn = atoi(argv[1]);
    if (tn == 1) check_conVar_and_hoare_semantics();    
    else if (tn == 2) check_conVar_broadcast_and_hoare_semantics();
    else if (tn == 3) check_lock_unlock();
    else if (tn == 4) check_spin_lock_unlock();    
    else cout <<"Wrong test case number\n";
    return 0;

    //check_conVar_and_hoare_semantics();
    //check_conVar_broadcast_and_hoare_semantics();
    //check_lock_unlock();
    //check_spin_lock_unlock();

    cout<<"Exiting"<<endl;
}

void check_conVar_and_hoare_semantics() {
    int threads[3];
    threads[0] = uthread_create(worker_wait_for_signal, NULL);
    threads[1] = uthread_create(signalOtherThread, NULL);
    threads[2] = uthread_create(work_thread, NULL);


    cout<<"Threads Created\n";
    for (int i = 0; i < 3; i++) {
        cout << "Joining on - " << threads[i] << "\n";
        uthread_join(threads[i], NULL);
    }
}

void check_conVar_broadcast_and_hoare_semantics() {
    int threads[10];
    for (int i = 0; i < 8; i++)
        threads[i] = uthread_create(worker_wait_for_signal, NULL);
    threads[8] = uthread_create(broadcastSignal, NULL);
    threads[9] = uthread_create(work_thread, NULL);


    cout<<"Threads Created\n";
    for (int i = 0; i < 10; i++) {
        cout << "Joining on - " << threads[i] << "\n";
        uthread_join(threads[i], NULL);
    }
}

void check_lock_unlock(){
    int threads[5];
    for (int i = 0; i < 5; i++)
        threads[i] = uthread_create(work_thread_lock_testing, NULL);


    cout<<"Threads Created\n";
    for (int i = 0; i < 5; i++) {
        cout << "Joining on - " << threads[i] << "\n";
        uthread_join(threads[i], NULL);
    }
}

void check_spin_lock_unlock(){
    int threads[5];
    for (int i = 0; i < 5; i++)
        threads[i] = uthread_create(work_thread_spin_lock_testing, NULL);


    cout<<"Threads Created\n";
    for (int i = 0; i < 5; i++) {
        cout << "Joining on - " << threads[i] << "\n";
        uthread_join(threads[i], NULL);
    }
}
