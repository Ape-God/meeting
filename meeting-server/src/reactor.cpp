#include "reactor.h"
#include "event_handler.h"

Reactor::Reactor() : _stop(false) {
    _epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (_epoll_fd == -1) {
        std::cerr << "Failed to create epoll file descriptor" << std::endl;
        exit(EXIT_FAILURE);
    }
}
        
Reactor::~Reactor() {
    close(_epoll_fd);
}

// 注册事件处理器
void Reactor::registerEventHandler(std::string decs, Handler handler) {
    _event_handlers[decs] = handler;
}

// 移除事件处理器
void Reactor::removeEventHandler(std::string decs) {
    auto it = _event_handlers.find(decs);
    if(it != _event_handlers.end()){
        _event_handlers.erase(it);
    }
}

// 添加需要管理的fd
void Reactor::addFd(int fd, std::string decs){
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
        std::cerr << "Failed to add file descriptor to epoll" << std::endl;
        exit(EXIT_FAILURE);
    }
    _handlers[fd] = _event_handlers[decs];
}

// 移除需要管理的fd
void Reactor::delfd(int fd) {
    if (_handlers.find(fd) == _handlers.end()) {
        std::cerr << "No event handler found for file descriptor: " << fd << std::endl;
        return;
    }
    
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
        std::cerr << "Failed to remove file descriptor from epoll" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    _handlers.erase(fd);
}

// 启动事件循环
void Reactor::start() {
    const int maxEvents = 16;
    struct epoll_event events[maxEvents];
    
    while (!_stop) {
        int nfds = epoll_wait(_epoll_fd, events, maxEvents, -1);
        if (nfds == -1) {
            std::cerr << "Failed to wait for events" << std::endl;
            if (errno != EINTR)
                exit(EXIT_FAILURE);
        }
        
        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd;
            _handlers[fd]->handleEvent(fd);
        }
    }
}

// 停止事件循环
void Reactor::stopEventLoop() {
    _stop = true;
}