#include "uthread.h"
#include "async_io.h"
#include <iostream>
#include <chrono>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>


using std::cout;
using std::endl;

#define BUFFER_SIZE 4*1024*1024
#define FIRST_LOOP 1000
#define SECOND_LOOP 1000
#define THIRD_LOOP 500
#define FILE_NAME "spam.csv"
#define READ_WRITE_OPERATIONS 500
#define TYPE ASYNC
#define SYNC 1
#define ASYNC 2

struct loop_args{
	int i, j, k;
};

struct io_args {
	const char* fn;
};

loop_args* getLoopArgs(int i, int j, int k);
io_args* getIOArgs(const char * str);

void * worker_simulation_long_calculation(void* args) {
    int my_tid = uthread_self();
    loop_args* largs = (loop_args*)args;
    cout<<"Starting thread : "<<my_tid<<endl;
    int a = 0;
    for (int i = 0; i < largs->i; i++) {
        for (int j = 0; j < largs->j; j++) {
            for (int k = 0; k < largs->k; k++);
        }
    }
    return NULL;
}

// Will be reading the same file many times
void * worker_sync_calls(void* args) {
    int my_tid = uthread_self();
    const char * file_name = ((io_args*)args)->fn;
    cout<<"Starting thread : "<<my_tid<<endl;

    char* buffer = new char[BUFFER_SIZE];
    int bytes_read = 1, offset = 0;
    for (int i = 0; i < READ_WRITE_OPERATIONS; i++) {
	bytes_read = 1;
        int fd = open(file_name, O_RDONLY);
	int wfd = open("sync_write", O_WRONLY|O_CREAT);
        while(bytes_read){
            bytes_read = read(fd, buffer, BUFFER_SIZE);
            if (bytes_read > 0) {
                write(wfd, buffer, BUFFER_SIZE);
            }
        }
        close(fd);
    }
    delete buffer;
    return NULL;
}


void * worker_async_calls(void* args) {
    int my_tid = uthread_self();
    //const char * file_name = "spam.csv";
    const char * file_name = ((io_args*)args)->fn;
    cout<<"Starting thread : "<<my_tid<<endl;
    char* buffer = new char[BUFFER_SIZE];
    int bytes_read = 1, offset = 0;
    for (int i = 0; i < READ_WRITE_OPERATIONS; i++) {
	bytes_read = 1;
        int fd = open(file_name, O_RDONLY);
	int wfd = open("async_write", O_WRONLY|O_CREAT);
        while(bytes_read){
            bytes_read = async_read(fd, buffer, BUFFER_SIZE, offset);
            offset += bytes_read;
            if (bytes_read > 0) {
                async_write(wfd, buffer, BUFFER_SIZE, offset);
            }
        }
        close(fd);
    }
    delete buffer;
    return NULL;
}

int main(int argc, char* argv[]){
    uthread_init(100);
    int threads[4];
    if (argc < 2){
	    std::cerr<<"Illegel. Enter type as argument. 1 for sync and 2 for async\n";
	    exit(1);
    } 
    int t = atoi(argv[1]);
    if (t > 2) exit(1);
    auto t0 = std::chrono::high_resolution_clock::now();
    //const char* file_name = "to_read";
    loop_args* largs = getLoopArgs(FIRST_LOOP, SECOND_LOOP, THIRD_LOOP);
    io_args* ioargs = getIOArgs(FILE_NAME);
    threads[0] = uthread_create(worker_simulation_long_calculation, (void *)largs);
    threads[1] = uthread_create(worker_simulation_long_calculation, (void *)largs);
    threads[2] = uthread_create(worker_simulation_long_calculation, (void *)largs);
    if (t == SYNC) threads[3] = uthread_create(worker_sync_calls, (void *)ioargs);
    else if (t == ASYNC) threads[3] = uthread_create(worker_async_calls, (void *)ioargs);


    cout<<"Threads Created\n";
    for (int i = 0; i < 4; i++) {
        cout << "Joining on - " << threads[i] << "\n";
        uthread_join(threads[i], NULL);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> secs = t0-t1;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(secs);
    std::cout << ms.count() << "ms\n";
    delete largs;
    delete ioargs;
    cout<<"Exiting"<<endl;
}

loop_args* getLoopArgs(int i, int j, int k){
    loop_args* args = new loop_args;
    args->i = i;
    args->j = j;
    args->k = k;
    return args;
}

io_args* getIOArgs(const char * str){
    io_args* args = new io_args;
    args->fn = str;
    return args;
}
