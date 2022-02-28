#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"

#include <memory>

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop, const std::string &nameArg)
    : baseLoop_(baseloop)
    , name_(nameArg)
    , started_(false)
    , numThreads_(0)
    , next_(0) //做轮询的下标的
{//析构不用做,是在线程栈上的
}

EventLoopThreadPool::~EventLoopThreadPool(){}



void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    started_ = true;

    for (int i = 0; i < numThreads_; ++i)
    {
        char buf[name_.size() + 32]; // 命名规则
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread *t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t)); // 智能指针自动delete
        loops_.push_back(t->startLoop()); // 底层创建线程，绑定一个新的EventLoop，并返回该loop的地址
    }

    if (numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}

// 如果工作在多线程中，baseloop默认以轮询的方式分配channel给subloop
EventLoop* EventLoopThreadPool::getNextLoop()
{
    EventLoop *loop = baseLoop_;

    if (!loops_.empty())
    {
        loop = loops_[next_];
        ++next_;
        if (next_ >= loops_.size())
        {
            next_ = 0;
        }
    }

    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    if (loops_.empty())
    {
        return std::vector<EventLoop*>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}