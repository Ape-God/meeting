#include "user.h"
#include "room.h"
#include "server.h"


int User::getFd() const{
    return _fd;
}

void User::setIp(uint32_t ip){
    _ip = ip;
}
uint32_t User::getIp(){
    return _ip;
}
   
// 获取对应的端口号
void User::setPort(uint16_t port){
    _port = port;
}
uint16_t User::getPort(){
    return _port;
}

// 获取对应的房间Room
std::weak_ptr<Room> User::getRoom(){
    return _room;
}

// 创建一个新的房间
std::string User::creatNewRoom(){

}

/**
 * 加入某个房间
*/
bool User::joinRoom(std::shared_ptr<Room> room){
    if(!room) return false;
    _room = room; 
}


// 获取当前的缓冲区指针
char* User::getBuf(){

}

// 获取起始缓冲区指针    
char* User::getStartBuf(){

}

int User::getAvailable(){
    
}


/**
 * 退出房间
*/
void User::exitRoom(){
    _room.reset();
}


// MSG_SEND* User::getPacket(int fd, MESGType type, void* data, int len, bool boardcast){
//     if(!data) return nullptr;
//     MSG_SEND* send_msg = new MSG_SEND(fd, boardcast);
//     send_msg->header[0] = 0X55;
//     send_msg->header[1] = 0XAA;
//     send_msg->type = type;
//     u8u16(send_msg->len) = MSG_SEND_HEAD_SIZE + len;
//     send_msg->data = data;
// }

void User::sendMesg(MSG_S_INFO* msg){
    if(!msg) return;
    // 加锁，防止多个线程同时对一个fd进行写入操作，可能会导致数据有问题
    std::lock_guard<std::mutex> lg(_sendMux);
    write(_fd, &(msg->msg), u8u16(msg->msg.len));
    // 发送计数减一
    msg->value.fetch_sub(1);
    // 判断该段数据，是否已经发送给所有用户了，是的话把该段数据返回到empty队列中
    if(msg->value.load() == 0){
        auto server = Server::getInstance();
        auto user = server->getUserByFd(msg->fd);
        if(user) {
            user->_deque_buf->pushBackTo_dequeBuffEmpty((uint8_t*)msg);
        }
    }
}


// 获取字符串类型的数据包
// MSG_S* User::getStrPacket(MESGSendType sendType, MESGType type, const char* resp){
//     MSG_S_INFO* msg_s = reinterpret_cast<MSG_S_INFO*>(_deque_buf->popFrontFrom_dequeBuffEmpty());
//     msg_s->fd = _fd;
//     msg_s->send_type = sendType;
//     msg_s->value.store(1);
//     msg_s->msg.header[0] = 0X55;
//     msg_s->msg.header[1] = 0XAA;
//     msg_s->msg.type = type;
//     u8u32(msg_s->msg.ip) = getIp();
//     u8u16(msg_s->msg.port) = getPort();
//     u8u16(msg_s->msg.len) = MSG_S_HEAD_SIZE + strlen(resp);
//     strcpy(msg_s->msg.data, resp);
// }

MSG_S_INFO* User::getBasePacket(MESGType type){
    MSG_S_INFO* msg_s = reinterpret_cast<MSG_S_INFO*>(_deque_buf->popFrontFrom_dequeBuffEmpty());
    if(!msg_s) return nullptr;
    msg_s->fd = _fd;
    // msg_s->value.store(1);
    msg_s->msg.header[0] = 0X55;
    msg_s->msg.header[1] = 0XAA;
    msg_s->msg.type = type;
    u8u32(msg_s->msg.ip) = getIp();
    u8u16(msg_s->msg.port) = getPort();
    // std::cout << u8u32(msg_s->msg.ip) << std::endl;
    // std::cout << u8u32(msg_s->msg.port) << std::endl;
    return msg_s;
}








// 处理数据
int User::process(){
    // 读取数据
    ssize_t bytesRead = read(_fd, _buf, MAX_READ_BUF_SIZE);
    if(bytesRead <= 0){
        return bytesRead;
    }
    _ring_buf->push(_buf, bytesRead);
    // 解析数据
    if(_ring_buf->get_length() >= MSG_C_HEAD_SIZE){
        std::lock_guard<std::mutex> lg(_mux);
        // 获取最前面的6个字节
        int size = _ring_buf->top(_buf, MSG_C_HEAD_SIZE);
        if(size < MSG_C_HEAD_SIZE) return bytesRead;
        // 获取整包数据
        MSG_C* msg = reinterpret_cast<MSG_C*>(_buf);
        uint16_t len = u8u16(msg->len); 
        if(_ring_buf->get_length() < len){
            // 数据没收齐，直接返回
            return bytesRead;
        }
        memset(msg, 0, len);
        size = _ring_buf->pop(_buf, len);
        // 解析处理
        if (msg->type == MESGType::CreatRoom){
            // 判断该用户是否已经在某个房间内
            auto room = _room.lock();
            if(room) {
                std::cout << "Already joined " << room->getId() << " room" << std::endl;
                return bytesRead;
            }
            // 创建一个新的Room
            auto new_room = Room::creatNewRoom(_fd);
            std::cout << "Creat new room:" << new_room->getId() << std::endl; 
            // 房间内加入该用户
            new_room->joinUserByFd(_fd);
            // 用户加入该房间
            _room = new_room;
            // 添加该房间到房间列表
            auto server = Server::getInstance();
            server->addRoom(new_room);
            // 创建好之后，将房间ID返回给客户端
            new_room->sendMesg(_fd, MESGType::CreatRoomResp, new_room->getId().c_str());
        } else if(msg->type == MESGType::JoinRoom){
            // 获取房间号
            std::string room_id = MSG_C::getJoinRoom(msg);
            // 判断该用户是否已经在某个房间内
            auto room = _room.lock();   
            if(room) {
                std::cout << "Already joined " << room->getId() << " room" << std::endl;
                room->sendMesg(_fd, MESGType::JoinRoomAlreadyJoin, nullptr, 0);
            } else{
                // 判断是否存在该房间
                auto server = Server::getInstance();
                auto join_room = server->getRoomById(room_id);
                if(join_room){
                    // 房间内加入该用户
                    join_room->joinUserByFd(_fd);
                    // 用户加入该房间
                    _room = join_room;
                    std::cout << "Join room:" << room_id << std::endl;
                    join_room->sendMesg(_fd, MESGType::JoinRoomSuccess, room_id.c_str());
                } else {
                    std::cout << "Not found room:" << room_id << std::endl;
                    // 没有房间
                    MSG_S_INFO* msg_s = getBasePacket(MESGType::JoinRoomNotFound);
                    if(!msg_s) {
                        std::cout << "getBasePacket error" << std::endl;
                        return bytesRead;
                    }
                    u8u16(msg_s->msg.len) = MSG_S_HEAD_SIZE;
                    msg_s->value.store(1);
                    sendMesg(msg_s);
                }
            }
        } else if(msg->type == MESGType::ExitRoom){
            // 判断该用户是否已经在某个房间内
            auto room = _room.lock();
            if(room){
                room->sendMesg(_fd, MESGType::ExitRoomSuccess, nullptr, 0); 
                // 房间内去除该用户
                room->exitUserByFd(_fd);
                // 用户离开房间
                exitRoom();
                std::cout << "Exit room:"<< room->getId() << std::endl;     
            } else {
                std::cout << "ExitRoomNotJoin" << std::endl;
                MSG_S_INFO* msg_s = getBasePacket(MESGType::ExitRoomNotJoin);
                if(!msg_s) {
                    std::cout << "getBasePacket error" << std::endl;
                    return bytesRead;
                }
                u8u16(msg_s->msg.len) = MSG_S_HEAD_SIZE;
                msg_s->value.store(1);
                sendMesg(msg_s);
            }
        } else if(msg->type == MESGType::textMsg){
            // 判断该用户是否已经在某个房间内
            auto room = _room.lock();
            if(room){
                room->sendMesg(_fd, MESGType::textMsgResp, msg->data, u8u16(msg->len) - MSG_C_HEAD_SIZE); 
            }







            // char* send_data = new char[std::max(data_size, 1)]{0};
            // // 有效数据长度是否大于0
            // if(data_size <= 0){
            //     std::cout << "Invaild data" << std::endl;
            //     data_size = 0;
            //     // 不转发
            //     MSG_SEND* send_msg = getPacket(_fd, MESGType::textMsgResp, send_data, data_size);
            //     sendMesg(send_msg);
            // } else {
            //     memcpy(send_data, msg->data, data_size);
            //     // 将该数据转发给房间内其他用户
            //     MSG_SEND* send_msg = getPacket(_fd, MESGType::textMsgResp, send_data, data_size, true);
            //     auto room = _room.lock();
            //     if(room) room->broadcast(send_msg);
            // }            
        }
    }
    return bytesRead;
}

