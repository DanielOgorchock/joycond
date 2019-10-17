#ifndef JOYCOND_EPOLL_MGR_H
#define JOYCOND_EPOLL_MGR_H

#include <map>
#include <memory>

#include "epoll_subscriber.h"

class epoll_mgr
{
    private:
        int epoll_fd;
        std::map<int, std::shared_ptr<epoll_subscriber>> subscribers;

    public:
        epoll_mgr();
        ~epoll_mgr();

        void add_subscriber(std::shared_ptr<epoll_subscriber> sub);
        void remove_subscriber(std::shared_ptr<epoll_subscriber> sub);
        void loop();
};

#endif

