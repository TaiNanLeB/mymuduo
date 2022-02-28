#include "PollPoller.h"
#include "Logger.h"
#include "Channel.h"

#include <errno.h>
#include <unistd.h>
#include <cstring>

PollPoller::PollPoller(EventLoop *loop)
    : Poller(loop)
{}
PollPoller::~PollPoller()
{}

Timestamp PollPoller::poll(int timeoutMs, ChannelList* activeChannels) 
{
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0)
    {
        LOG_INFO("%d events happened", numEvents);
        fillActiveChannels(numEvents, activeChannels);
    }
    else if (numEvents == 0)
    {
        LOG_INFO("nothing happened");
    }
    else
    {
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            LOG_FATAL("PollPoller::poll, errno = %d", errno);
        }
    }
    return now;
}
void PollPoller::updateChannel(Channel *channel) 
{
    
}
void PollPoller::removeChannel(Channel *channel) 
{

}
void PollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for (PollerFdList::const_iterator pfd = pollfds_.begin(); 
        pfd != pollfds_.end() && numEvents > 0; ++pfd)
    {
        if (pfd->revents > 0)
        {
            --numEvents;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            Channel* channel = ch->second;
            channel->set_revents(pfd->revents);
            // pfd->revents = 0;
            activeChannels->push_back(channel);
        }
    }
}