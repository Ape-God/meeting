#pragma once

#include <mutex>
#include "reactor.h"

#include <memory>


class User;
class Room;

class Server {
private:
    static Server* _server;
    static std::mutex _mux;

    std::shared_ptr<Reactor> _reactor;
    std::unordered_map<int, std::shared_ptr<User>> _users;
    std::unordered_map<std::string, std::shared_ptr<Room>> _rooms;

    int _port;
    Server():_port(8888){

    }

    

public:
    static Server* getInstance(){
        if(_server == nullptr){
            std::lock_guard<std::mutex> lg(_mux);
            if(_server == nullptr){
                _server = new Server();
            }
        }
        return _server;
    }

    // 添加用户
    int addUser(std::shared_ptr<User> user);

    // 移除用户
    void removeUser(std::shared_ptr<User> user);

    void removeUserByFd(int fd);

    std::shared_ptr<User> getUserByFd(int fd);


    // 新建房间
    void addRoom(std::shared_ptr<Room>);
    // 解散房间
    void removeRoom(std::shared_ptr<Room>);    

    void removeRoomById(std::string room_id);

    std::shared_ptr<Room> getRoomById(std::string room_id);



    // 
    bool start(int port);
};



