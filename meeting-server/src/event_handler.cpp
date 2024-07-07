#include "event_handler.h"
#include "reactor.h"
#include "user.h"
#include "room.h"
#include "server.h"


void EventHandler::setReactor(std::shared_ptr<Reactor>& r){
    _reactor = r;
}

TcpAccpectHandler::TcpAccpectHandler(std::string decs, Server* server) {
    _decs = decs;
    _server = server;
}

void TcpAccpectHandler::handleEvent(int fd) {
    // 处理新的连接
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int clientfd = accept(fd, (struct sockaddr*)&client_addr, &client_len);
    if (clientfd == -1) {
        std::cerr << "Failed to accept client connection" << std::endl;
        return;
    }
    // 接收到新的连接
    std::cout << "New client connected: " << inet_ntoa(client_addr.sin_addr) << ":" << client_addr.sin_port << " fd:" << clientfd << std::endl;
    struct in_addr addr;
    
    // 将IP地址从字符串形式转换为网络字节序的二进制形式
    if (inet_pton(AF_INET, inet_ntoa(client_addr.sin_addr), &(addr.s_addr)) == 0) {
        perror("inet_pton");
        return;
    }
    // 将二进制形式的IP地址转换为uint32_t类型的整数
    uint32_t ip = htonl(addr.s_addr);
    uint16_t port = client_addr.sin_port;

    // 新建一个用户
    auto user = std::make_shared<User>(clientfd);
    user->setIp(ip);
    user->setPort(port);
    
    // Server添加一个用户
    auto server = Server::getInstance();
    server->addUser(user);
}

TcpProcessHandler::TcpProcessHandler(std::string decs, Server* server) {
    _decs = decs;
    _server = server;
}

// 可以把该函数放置到多线程中运行
void TcpProcessHandler::handleEvent(int fd) {
    auto user = _server->getUserByFd(fd);
    ssize_t bytesRead = user->process();
    if(bytesRead <= 0){
        if(bytesRead == 0){
            // 客户端断开连接
            std::cout << "Client disconnected: " << fd << std::endl;
        } else {
            std::cerr << "Read error from client: " << fd << std::endl;
        }
        auto room = user->getRoom().lock();
        if(room) room->exitUserByFd(fd);
        _server->removeUserByFd(fd);
    }
}


