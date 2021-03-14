#include "uthread.h"
#include "async_io.h"
#include <iostream>
#include <chrono>

#define BUFFER_SIZE 1000

void * worker_simulation_long_calculation() {
    int my_tid = uthread_self();
    cout<<"Starting thread : "<<my_tid<<endl;
    int a = 0;
    for (int i = 0; i < 10000; i++) {
        for (int j = 0; j < 10000; j++) {
            for (int k = 0; k < 10000; k++);
        }
    }
    return NULL;
}

// Will be reading the same file many times
void * worker_sync_calls(void* args) {
    int my_tid = uthread_self();
    char * file_name = (char *) args;
    cout<<"Starting thread : "<<my_tid<<endl;
    int a = 0;

    char* buffer = new char[BUFFER_SIZE];
    int bytes_read = 1, offset = 0;
    for (int i = 0; i < 10000; i++) {
        int fd = open(file_name, O_RDONLY);
        while(bytes_read){
            bytes_read = read(fd, buffer, BUFFER_SIZE);
            if (bytes_read > 0) {
                write(fd, buffer, BUFFER_SIZE);
            }
        }
        close(fd);
    }
    delete buffer;
    return NULL;
}


void * worker_async_calls(void* args) {
    int my_tid = uthread_self();
    char * file_name = (char *) args;
    cout<<"Starting thread : "<<my_tid<<endl;
    char* buffer = new char[BUFFER_SIZE];
    int bytes_read = 1, offset = 0;
    for (int i = 0; i < 10000; i++) {
        int fd = open(file_name, O_RDONLY);
        while(bytes_read){
            bytes_read = async_read(fd, buffer, BUFFER_SIZE, offset);
            offset += bytes_read;
            if (bytes_read > 0) {
                async_write(fd, buffer, BUFFER_SIZE, offset);
            }
        }
        close(fd);
    }
    delete buffer;
    return NULL;
}

int main(){
    uthread_init(100);
    int threads[4];
    auto t0 = std::chrono::high_resolution_clock::now();
    threads[0] = uthread_create(worker_simulation_long_calculation, NULL);
    threads[1] = uthread_create(worker_simulation_long_calculation, NULL);
    threads[2] = uthread_create(worker_simulation_long_calculation, NULL);
    threads[3] = uthread_create(worker_sync_calls, NULL);

    cout<<"Threads Created\n";
    for (int i = 0; i < 3; i++) {
        cout << "Joining on - " << threads[i] << "\n";
        uthread_join(threads[i], NULL);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> secs = t0-t1;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(secs);
    std::cout << ms.count() << "ms\n";
    cout<<"Exiting"<<endl;
}