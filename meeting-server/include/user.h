#pragma once

#include <memory>

#include "ring_buffer.h"
#include "deque_buffer.h"
#include "msg.h"

constexpr int MAX_BUF_SIZE = 1024 * 4;
constexpr int MAX_READ_BUF_SIZE = 1024 * 2;

class Room;
class User
{
public:
    // socket_fd
    int _fd;
    // 所属房间
    std::weak_ptr<Room> _room;
    std::shared_ptr<myRingBuffer> _ring_buf;
    std::mutex _mux;
    // read缓存
    char* _buf;
    // 用户IP
    uint32_t _ip;
    // 用户PORT 
    uint16_t _port;


    // 需要发送的数据缓存
    std::shared_ptr<DequeBuffer> _deque_buf;
    std::mutex _sendMux;

public:
    User(int fd):_fd(fd), _ring_buf(std::make_shared<myRingBuffer>(MAX_BUF_SIZE)), _deque_buf(std::make_shared<DequeBuffer>(sizeof(MSG_S), 20)){
        _buf = new char[MAX_READ_BUF_SIZE];
        if(!_buf){
            // todo 如果内存申请不够了，应对策略
        }
    }
    ~User(){
        
    }

    int getFd() const;

    // 获取对应的ip地址
    void setIp(uint32_t ip);
    uint32_t getIp();
   
    // 获取对应的端口号
    void setPort(uint16_t port);
    uint16_t getPort();


    // 获取对应的房间Room
    std::weak_ptr<Room> getRoom();

    // 创建一个新的房间
    std::string creatNewRoom();

    /**
     * 加入某个房间
     * 
     * 
    */
    bool joinRoom(std::shared_ptr<Room> room);


    // 获取当前的缓冲区指针
    char* getBuf();

    // 获取起始缓冲区指针    
    char* getStartBuf();

    int getAvailable();


    /**
     * 退出房间
    */
    void exitRoom();

    // 发送数据-线程任务函数
    void sendMesg(MSG_S_INFO* msg);

    // 
    MSG_S_INFO* getBasePacket(MESGType type);

    // 处理数据
    int process();
};

