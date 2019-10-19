#include "epoll_subscriber.h"

//private

//public
epoll_subscriber::epoll_subscriber(std::vector<int> fds, std::function<void(int event_fd)> callback) :
    event_callback(callback),
    event_fds(fds)
{
}

epoll_subscriber::~epoll_subscriber()
{
}

void epoll_subscriber::operator() (int event_fd)
{
    event_callback(event_fd);
}

const std::vector<int>& epoll_subscriber::get_event_fds() const
{
    return event_fds;
}
