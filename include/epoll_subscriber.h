#ifndef JOYCOND_EPOLL_SUBSCRIBER_H
#define JOYCOND_EPOLL_SUBSCRIBER_H

#include <functional>
#include <vector>

class epoll_subscriber
{
    private:
        std::function<void(int)> event_callback;
        std::vector<int> event_fds;

    public:
        epoll_subscriber(std::vector<int> fds, std::function<void(int event_fd)> callback);

        ~epoll_subscriber();

        void operator() (int event_fd);
        const std::vector<int>& get_event_fds() const;
};

#endif

