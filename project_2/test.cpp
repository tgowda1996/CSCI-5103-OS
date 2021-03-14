#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<iostream>
#include<error.h>
#include<aio.h>

int main(){
	const char* file_name = "to_read", *write_file_name = "sample_aio_write";
	int fd = open(file_name, O_RDONLY);
	//std::cout<<fd<<'\n';
	//perror("Error");
	char* buffer  = new char[1024];
	aiocb* s = new aiocb;
	s->aio_fildes = fd;
	s->aio_offset = 0;
	s->aio_buf = buffer;
	s->aio_nbytes = 500;
	int ret = aio_read(s);
	int c=0;
	if (ret !=0) {
		std::cout<<ret;
		perror("Error");
		exit(0);
	}

	while(aio_error(s) == EINPROGRESS) c++;

	//int ret = read(fd, buffer, 200);
	buffer[500] = '\0';
	close(fd);
	std::cout<<c<<"   "<<buffer<<'\n';
	//perror("Error");
	
	
	fd = open(write_file_name, O_WRONLY|O_CREAT);
	s->aio_fildes = fd;
	ret = aio_write(s);
	c=0;
	if (ret !=0) {
		std::cout<<ret<<"\n";
		perror("Error");
		exit(0);
	}

	while(aio_error(s) == EINPROGRESS) c++;
	std::cout<<"count : "<<c<<'\n';

}
