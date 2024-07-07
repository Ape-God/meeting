#include "server.h"
#include "user.h"
#include "room.h"

#include "event_handler.h"


Server* Server::_server = nullptr;
std::mutex Server::_mux;

int Server::addUser(std::shared_ptr<User> user){
    // 向列表中添加一个用户
    _users[user->getFd()] = user;
    // 向_reactor中添加fd
    _reactor->addFd(user->getFd(), PROCESS);
}

void Server::removeUser(std::shared_ptr<User> user) {
    // 1、如果用户加入了某个房间，则先退出该房间
    user->exitRoom();
    // 2、在_reactor中移除fd
    _reactor->delfd(user->getFd());
    // 3、在用户列表中移除
    auto it = _users.find(user->getFd());
    if(it != _users.end()){
        _users.erase(it);
    }
    // 4、关闭fd
    close(user->getFd());
}

void Server::removeUserByFd(int fd){
    auto user = getUserByFd(fd);
    if(user){
        removeUser(user);
    }
}

std::shared_ptr<User> Server::getUserByFd(int fd){
    if(_users.find(fd) == _users.end()){
        return nullptr;
    }
    return _users[fd];
}

void Server::addRoom(std::shared_ptr<Room> room){
    // 向列表中添加一个房间
    _rooms[room->getId()] = room;
}

void Server::removeRoom(std::shared_ptr<Room> room){
    // 先解散房间
    Room::destoryRoom(room);
    // 
    auto it = _rooms.find(room->getId());
    if(it != _rooms.end()){
        _rooms.erase(it);
    }
} 

void Server::removeRoomById(std::string room_id){
    auto room = getRoomById(room_id);
    if(room){
        removeRoom(room);
    }
}

std::shared_ptr<Room> Server::getRoomById(std::string room_id){
    if(_rooms.find(room_id) == _rooms.end()){
        return nullptr;
    }
    return _rooms[room_id];
} 



// 启动服务器
bool Server::start(int port){
    _port = port;
    // 创建TCP Socket并绑定到本地IP和端口
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listenfd == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(_port);

    int option = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &option, sizeof(int));

    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, SOMAXCONN) == -1) {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // reactor开始处理
    // 1、注册handler
    auto accpect_handler =  std::make_shared<TcpAccpectHandler>(ACCPET, this);    
    auto process_handler =  std::make_shared<TcpProcessHandler>(PROCESS, this);
    _reactor = std::make_shared<Reactor>();
    accpect_handler->setReactor(_reactor);
    process_handler->setReactor(_reactor);
    _reactor->registerEventHandler(ACCPET, accpect_handler);
    _reactor->registerEventHandler(PROCESS, process_handler);

    // 2、将listenfd加入到epoll中
    _reactor->addFd(listenfd, ACCPET);

    // 3、开始loop循环
    _reactor->start();
}


