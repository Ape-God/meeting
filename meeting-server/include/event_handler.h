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

#include "reactor.h"

const std::string ACCPET = "accpetHander";
const std::string PROCESS = "processHander";

class Server;

// 事件处理器基类
class EventHandler {
protected:
    std::weak_ptr<Reactor> _reactor;
    Server* _server;
    std::string _decs;
public:
    void setReactor(std::shared_ptr<Reactor>& r);
    virtual void handleEvent(int fd) = 0;
};

using Handler = std::shared_ptr<EventHandler>;

/**
 * 处理新的连接
*/
class TcpAccpectHandler : public EventHandler {
public:
    TcpAccpectHandler(std::string decs, Server* server);
    void handleEvent(int fd) override;
};

/**
 * 处理具体业务逻辑
*/
class TcpProcessHandler : public EventHandler {
public:
    TcpProcessHandler(std::string decs, Server* server);

    void handleEvent(int fd) override;
};





