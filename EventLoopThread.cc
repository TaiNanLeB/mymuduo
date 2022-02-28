#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, 
    const std::string &name)
    : loop_(nullptr)
    , exiting_(false)
    , thread_(std::bind(&EventLoopThread::threadFunc, this), name)
    , mutex_()
    , cond_()
    , callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join(); // 等待底层子线程结束
    }
}

EventLoop* EventLoopThread::startLoop()
{
    thread_.start(); // 启动新线程，也就是上面的threadFunc

    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while ( loop_ == nullptr )
        {
            cond_.wait(lock); // 等待在这把互斥锁上
        }
        loop = loop_;
    }
    return loop;
}

// 这个方法是在单独的新线程里运行的
void EventLoopThread::threadFunc()
{
    EventLoop loop; // 创建一个独立的eventloop，和上面的线程是一一对应的

    if (callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}