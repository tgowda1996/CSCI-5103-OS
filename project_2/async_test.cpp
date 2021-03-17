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

#define BUFFER_SIZE 1000
#define FIRST_LOOP 1000
#define SECOND_LOOP 1000
#define THIRD_LOOP 1000
#define FILE_NAME "to_read"

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


void * worker_async_calls(void* args) {
    int my_tid = uthread_self();
    //const char * file_name = "spam.csv";
    const char * file_name = ((io_args*)args)->fn;
    cout<<"Starting thread : "<<my_tid<<endl;
    char* buffer = new char[BUFFER_SIZE];
    int bytes_read = 1, offset = 0;
    int fd = open(file_name, O_RDONLY);
    int wfd = open("to_write", O_WRONLY|O_CREAT);
    bytes_read = async_read(fd, buffer, BUFFER_SIZE, offset);
    async_write(wfd, buffer, bytes_read, 0);
    cout<<"Done with read and write\n";
    delete buffer;
    return NULL;
}

void * worker_async_write(void* args) {
    int my_tid = uthread_self();
    const char * file_name = "to_write_async_call";
    cout<<"Starting thread : "<<my_tid<<endl;
    char buffer[] = "test";
    int bytes_read = 1, offset = 0;
    int fd = open(file_name, O_WRONLY|O_CREAT);
    async_write(fd, buffer, 5, offset);
    cout<<"Done with write\n";
    return NULL;
}

int main(int argc, char* argv[]){
    uthread_init(10000);
    if (argc < 2) {
        std::cerr << "Usage: ./test_async_methods <test_case_number>" << endl;
        exit(1);
    }
    int tn = atoi(argv[1]);
    if (tn > 2) {
        std::cerr << "Test case number can be either 1 or 2" << endl;
    }

    int threads[2];
    //const char* file_name = "to_read";
    loop_args* largs = getLoopArgs(FIRST_LOOP, SECOND_LOOP, THIRD_LOOP);
    io_args* ioargs = getIOArgs(FILE_NAME);
    threads[0] = uthread_create(worker_simulation_long_calculation, (void *)largs);
    if (tn == 1) threads[1] = uthread_create(worker_async_calls, (void *)ioargs);
    else threads[1] = uthread_create(worker_async_write, (void *)ioargs);

    cout<<"Threads Created\n";
    for (int i = 0; i < 2; i++) {
        cout << "Joining on - " << threads[i] << "\n";
        uthread_join(threads[i], NULL);
    }
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
