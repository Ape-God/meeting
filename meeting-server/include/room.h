#pragma once
#include <unordered_set>
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
#include "msg.h"

class User;

class Room {
    enum class RoomStatus{
        CLOSE,
        ON
    };
private:
    int _owner_fd;
    RoomStatus _status;
    std::unordered_map<int, std::shared_ptr<User>> _user_map;
    std::string _room_id;
    
    static std::unordered_set<std::string> _id_set;
    // 获取一个随机的房间ID
    static std::string getRandomId(int len);
    static std::string getNewRandomId(int len);
public:
    Room(int owner_fd);
    ~Room();
    

    // 创建一个新的房间
    static std::shared_ptr<Room> creatNewRoom(int owner_fd);
    // 解散一个房间
    static int destoryRoom(std::shared_ptr<Room>);

    // 
    std::string getId();

    // 用户加入
    int joinUserByFd(int fd);

    // 用户退出
    int exitUserByFd(int fd);

    // 获取用户
    std::shared_ptr<User> getUserByFd(int fd);


    // 发送数据-线程任务函数
    // void sendMesg(int fd, MESGType type);

    void sendMesg(int fd, MESGType type, const char* str);

    void sendMesg(int fd, MESGType type, const void* data, uint16_t len);

    void sendMesg(MSG_S_INFO* msg_s, const char* str);

    void sendMesg(MSG_S_INFO* msg_s, const void* data, uint16_t len);

    void sendMesg(MSG_S_INFO* msg_s);

    void sendMesgToOne(MSG_S_INFO* msg_s);
    void sendMesgToOther(MSG_S_INFO* msg_s);
    void sendMesgToAll(MSG_S_INFO* msg_s);


    // 向组内所有成员广播数据，拷贝数据
    int broadcast(MSG_S* msg);

};


