#pragma once

#include "noncopyable.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "EventLoopThreadPool.h"
#include "Callbacks.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "Buffer.h"

#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>

class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    enum Options // 枚举，两个选项表示是否对端口可重用
    {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop *loop,
                const InetAddress &listenAddr,
                const std::string &nameArg,
                Options option = kNoReusePort);
    ~TcpServer();

    // 设置底层subloop的个数
    void setThreadNum(int numThreads);

    //开启服务器监听
    void start();

    void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }
    void seConnectionCallback(const ConnectionCallback &cb)  { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb)       { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop *loop_; // baseloop 用户定义的loop

    const std::string ipPort_;
    const std::string name_;

    std::unique_ptr<Acceptor> acceptor_; // 运行在mainloop，监听新连接事件
    std::shared_ptr<EventLoopThreadPool> threadPool_; // one loop per thread

    ConnectionCallback connectionCallback_; // 有新连接时的回调
    MessageCallback messageCallback_;       // 读写消息
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完成

    ThreadInitCallback threadInitCallback_; // loop线程初始化
    
    std::atomic_int started_;

    int nextConnId_;
    ConnectionMap connections_; // 保存所有的连接
};