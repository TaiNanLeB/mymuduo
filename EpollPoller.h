  #pragma once 

#include "Poller.h"
#include "Timestamp.h"

#include <vector>
#include <sys/epoll.h>

class Channel;
/* epoll 的使用
 * epoll_crate
 * epoll_ctl 添加想让epoll监听的fd和感兴趣的事件
 * epoll wait
 */
class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller() override;

    // 重写基类Poller的抽象方法
    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;// epollcreate
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;// 相当于Epoll_ctl
private:
    static const int kInitEventListSize = 16;

    // 填写活跃的连接
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;//epoll_wait
    // 更新channel通道 相当于Epoll_ctrl
    void update(int operation, Channel *channel); 

    using EventList = std::vector<epoll_event>;

    int epollfd_;
    EventList events_; //vector<epoll_event>
};