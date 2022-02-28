#pragma once

#include "Poller.h"
#include "Timestamp.h"

#include <vector>
#include <poll.h>

class PollPoller : Poller
{
public:
    PollPoller(EventLoop *loop);
    ~PollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;
private:
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

    using PollerFdList = std::vector<pollfd>;
    PollerFdList pollfds_;
};