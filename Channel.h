#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>

class EventLoop;

/*
"通道"，封装了sockfd和其感兴趣的event，如EPOLLIN/OUT等
绑定了poller返回的具体事件
*/
class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;//c++11

    Channel(EventLoop *loop, int fd);
    ~Channel();

    //fd得到poller通知后处理事件（调用相应的回调）
    void handleEvent(Timestamp receiveTime);

    //设置回调函数对象
    void setReadCallback(ReadEventCallback cb){readCallback_ = std::move(cb);}
    void setWriteCallback(EventCallback cb){writeCallback_ = std::move(cb);}
    void setCloseCallback(EventCallback cb){closeCallback_ = std::move(cb);}
    void setErrorCallback(EventCallback cb){errorCallback_ = std::move(cb);}

    //防止当Channel被手动remove掉时Channel还在执行回调操作
    void tie(const std::shared_ptr<void>&);

    int fd() const {return fd_;}
    int events() const {return events_;}
    int set_revents(int revt){revents_ = revt;}

    //设置fd相应的事件状态
    void enableReading()  {events_ |= kReadEvent; update();}
    void disableReading() {events_ &= ~kReadEvent; update();}
    void enableWriting()  {events_ |= kWriteEvent; update();}
    void disableWriting() {events_ &= ~kWriteEvent; update();}
    void diableAll()      {events_ = kNoneEvent; update();}

    //返回fd当前的事件状态
    bool isNoneEvent() const {return events_ == kNoneEvent;}
    bool isWriting() const   {return events_ & kWriteEvent;}
    bool isReading() const   {return events_ & kReadEvent;}

    int index() {return index_;}
    void set_index(int idx) {index_ = idx;}

    EventLoop* ownerLoop() {return loop_;}//channel属于的eventloop
    void remove();//删除channel
private:

    void update();
    void handleEventWithGuard(Timestamp receiveTime);//受保护

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_;
    const int fd_;  //poller监听的fd
    int events_;    //注册fd感兴趣的事件
    int revents_;   //poller返回的发生的事件
    int index_;     //used by poller

    std::weak_ptr<void> tie_;
    bool tied_;

    //Channel能获知fd的revents，所以由Channel负责调用具体事件的回调操作
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

};