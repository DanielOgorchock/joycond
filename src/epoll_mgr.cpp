#include "epoll_mgr.h"

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>

//private

//public
epoll_mgr::epoll_mgr()
{
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        std::cerr << "Failed to create epoll; " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

epoll_mgr::~epoll_mgr()
{
}

void epoll_mgr::add_subscriber(std::shared_ptr<epoll_subscriber> sub)
{
    for (int fd : sub->get_event_fds()) {
        if (subscribers.count(fd)) {
            std::cerr << "epoll_mgr already contains event_fd; cannot add twice\n";
            exit(EXIT_FAILURE);
        }

        struct epoll_event event = {0};
        event.events = EPOLLIN;
        event.data.fd = fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event)) {
            std::cerr << "Failed to add fd to epoll; errno=" << errno << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "adding epoll_subscriber: fd=" << fd << std::endl;
        subscribers[fd] = sub;
    }
}

void epoll_mgr::remove_subscriber(std::shared_ptr<epoll_subscriber> sub)
{
    for (int fd : sub->get_event_fds()) {
        if (!subscribers.count(fd)) {
            std::cerr << "epoll_mgr doesn't contain event_fd; cannot remove: " << fd << std::endl;
            exit(EXIT_FAILURE);
        }
        if (subscribers[fd] != sub) {
            std::cerr << "subscriber to be removed matches fd of other subscriber\n";
            exit(EXIT_FAILURE);
        }

        struct epoll_event event = {0};
        event.events = EPOLLIN;
        event.data.fd = fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event)) {
            std::cerr << "Failed to remove fd from epoll; errno=" << errno << std::endl;
            exit(EXIT_FAILURE);
        }
        subscribers.erase(fd);
    }
}

static const int MAX_EVENTS = 10;
static const int TIMEOUT = 500;
void epoll_mgr::loop()
{
    struct epoll_event events[MAX_EVENTS];
    int nfds;

    nfds = epoll_pwait(epoll_fd, events, MAX_EVENTS, TIMEOUT, nullptr);
    if (nfds == -1) {
        std::cerr << "epoll_pwait failure\n";
        return;
    }

    for (int i = 0; i < nfds; i++) {
        int e_fd = events[i].data.fd;
        if (subscribers.count(e_fd))
            (*subscribers[e_fd])(e_fd);
        else
            std::cerr << "fd not found in subscribers map\n";
    }
}

