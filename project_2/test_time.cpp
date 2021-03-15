#include<iostream>
#include<chrono>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

#define BUFFER_SIZE 7*1024*1024

int main(){
    const char * file_name = "spam.csv";
    auto t0 = std::chrono::high_resolution_clock::now();
    char* buffer = new char[BUFFER_SIZE];
    int bytes_read = 1, offset = 0;
    for (int i = 0; i < 1000; i++) {
	bytes_read = 1;
        int fd = open(file_name, O_RDONLY);
	int wfd = open("sync_write", O_WRONLY|O_CREAT);
        while(bytes_read){
            bytes_read = read(fd, buffer, BUFFER_SIZE);
	    //std::cout<<bytes_read;
            if (bytes_read > 0) {
                write(wfd, buffer, BUFFER_SIZE);
		offset++;
            }
        }
        close(fd);
    }
    delete buffer;
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> secs = t0-t1;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(secs);
    std::cout << ms.count() << "ms\n";
    std::cout<<"Wrote to the file "<<offset<<" number of times\n";
    return 0;
}
