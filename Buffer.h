#pragma once

#include <vector>
#include <cstddef>
#include <string>
#include <algorithm>

// +-----------------+-----------------+-------------+
// | prependableBytes|                 |             |
// +-----------------+-----------------+-------------+
//                 readIndex        writeIndex      size

// 底层缓冲器
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;   // 记录数据包的长度
    static const size_t kInitialSize = 1024; // 缓冲区初始的大小

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize)
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend)
    {}

    size_t readableBytes() const 
    { return writerIndex_ - readerIndex_; }

    size_t writeableBytes() const
    { return buffer_.size() - writerIndex_; }

    size_t prependableBytes() const // 返回前面空间的大小
    { return readerIndex_; }

    // 返回缓冲区中可读数据的起始地址
    const char* peek() const
    { return begin() + readerIndex_; }

    // buffer=》string
    void retrieve(size_t len)
    {
        if (len < readableBytes())
        {
            readerIndex_ += len; // 应用只读取了刻度缓冲区数据的一部分就是len，还剩下readerIndex_ += len到writeindex_
        }
        else
        {
            retrieveAll();
        }
        
    }

    void retrieveAll()
    {
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    // 把onmessage函数上报的buffer数据，转成string类型的数据返回
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes()); // 应用可读取数据的长度
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len); // 上面把缓冲区中可读的事件已经读取出来了，这里对缓冲区进行复位
        return result;
    }
    
    // 判断要写的len，缓冲区是否够写
    void ensureWriteableBytes(size_t len)
    {
        if (writeableBytes() < len)
        {
            makeSpace(len); // 扩容
        }
    }

    // 把【data，data+len】内存上的数据，添加到writeable缓冲区中
    void append(const char *data, size_t len)
    {
        ensureWriteableBytes(len);
        std::copy(data, data+len, beginWrite());
        writerIndex_ += len;
    }

    char* beginWrite()
    { return begin() + writerIndex_; }
    const char* beginWrite() const
    { return begin() + writerIndex_; }

    // 从fd上读取数据
    ssize_t readFd(int fd, int *savedErrno);
    // 通过fd发送数据
    ssize_t writeFd(int fd, int *savedErrno);
private:
    char* begin()
    {
        // 取*是调用迭代器的*运算符重载函数，返回的是容器底层第零号位元素的本身
        return &*buffer_.begin(); // vector底层数组首元素的地址
    }
    const char* begin() const
    { return &*buffer_.begin(); }
    void makeSpace(size_t len)
    {
        if (writeableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            // 数据往前挪
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_,
                        begin() + writerIndex_,
                        begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
        
    }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};