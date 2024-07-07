#pragma once
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sys/epoll.h>
#include <memory>

class EventHandler;

// Reactor类，负责事件循环和事件分发
class Reactor {
public:
    Reactor();
    ~Reactor();
    void registerEventHandler(std::string decs, std::shared_ptr<EventHandler> handler);
    void removeEventHandler(std::string decs);
    void addFd(int fd, std::string decs);
    void delfd(int fd);
    void start();
    void stopEventLoop();

private:
    int _epoll_fd;
    std::unordered_map<int, std::shared_ptr<EventHandler>> _handlers;
    bool _stop;

    // 已注册的事件驱动器
    std::unordered_map<std::string, std::shared_ptr<EventHandler>> _event_handlers;
};
