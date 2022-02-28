#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

// 从fd上读取数据，poller工作在LT模式，不读完一直上报
ssize_t Buffer::readFd(int fd, int *savedErrno)
{
    char extrabuf[65536] = {0}; // 栈上的内存空间
    
    struct iovec vec[2];
    
    const size_t writable = writeableBytes(); // 这是buffer底层缓冲区剩余的可写空间的大小
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;          // 这是第一块缓冲区

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;   // 第二个

    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *savedErrno = errno;
    }
    else if (n <= writable)
    {
        writerIndex_ += n;
    }
    else // extrabuf里面也写入了数据
    {
        writerIndex_ = buffer_.size(); // 放在最后了
        append(extrabuf, n - writable);
    }
    return n;
    
    
}

ssize_t Buffer::writeFd(int fd, int *savedErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0)
    {
        *savedErrno = errno;
    }
    return n;
}