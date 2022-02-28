#pragma once
#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

#include <functional>

class EventLoop;
class InetAddress;

// 相当于accept函数的封装
class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb)
    { newConnectionCallback_ = cb; }

    bool listenning() const { return listenning_; }
    void listen();
private:
    void handleRead();

    EventLoop* loop_; // Acceptor用的就是用户定义的baseloop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_; // 将接受的fd打包成channdl通过轮询找到subloop
    bool listenning_;
};