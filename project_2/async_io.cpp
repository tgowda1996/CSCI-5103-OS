#include "async_io.h"
#include "uthread.h"
#include <aio.h>
#include <errno.h>
#include <cstdio>

// TODO

static aiocb* getAioStructure(int fd, size_t count, int offset);
static ssize_t getSsize(aiocb *s, int ret);


ssize_t async_read(int fd, void *buf, size_t count, int offset){
    aiocb* s = getAioStructure(fd, count, offset);
    s->aio_buf = buf;
    int ret = aio_read(s);
    ssize_t bytes_read = getSsize(s, ret);
    delete s;
    return bytes_read;
}

ssize_t async_write(int fd, void *buf, size_t count, int offset) {
    aiocb* s = getAioStructure(fd, count, offset);
    s->aio_buf = buf;
    int ret = aio_write(s);
    ssize_t bytes_written = getSsize(s, ret);
    delete s;
    return bytes_written;
}

static aiocb* getAioStructure(int fd, size_t count, int offset){
    aiocb* s = new aiocb;
    s->aio_fildes = fd;
    s->aio_offset = offset;
    s->aio_nbytes = count;

    return s;
}

static ssize_t getSsize(aiocb *s, int ret) {
    if (ret != 0) {
        perror("Error occurred while performing aio");
        return 0; //aio_read failed.
    }

    while(aio_error(s) == EINPROGRESS) {
        uthread_yield();
    }

    if (aio_error(s) != 0) {
        perror("Error occurred while performing aio");
        return 0;
    }

    return aio_return(s);
}
