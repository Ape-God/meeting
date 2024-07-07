#include "user.h"
#include "room.h"
#include <random>
#include "server.h"

std::string charset = "0123456789";
std::unordered_set<std::string> Room::_id_set;
Room::Room(int owner_fd):_owner_fd(owner_fd) {

}

Room::~Room() {

}



std::string Room::getRandomId(int len){
    std::string randomString;
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, charset.size() - 1);

    for (int i = 0; i < len; ++i) {
        randomString += charset[distribution(generator)];
    }
    return randomString;
}

std::string Room::getNewRandomId(int len){
    std::string random_id;
    do{
        random_id = getRandomId(len);
    } while(_id_set.find(random_id) != _id_set.end());
    return random_id;
}

std::shared_ptr<Room> Room::creatNewRoom(int owner_fd){
    std::shared_ptr<Room> room = std::make_shared<Room>(owner_fd);
    room->_room_id = Room::getNewRandomId(5);
    return room;
}

int Room::destoryRoom(std::shared_ptr<Room> room){
    // 通知各个成员房间已解散
    room->_user_map.clear();
    return 0;
}

std::string Room::getId(){
    return _room_id;
}


int Room::joinUserByFd(int fd) {
    auto server = Server::getInstance();
    _user_map[fd] = server->getUserByFd(fd);
    return fd;
}

int Room::exitUserByFd(int fd){
    if(_user_map.find(fd) == _user_map.end()){
        return -1;
    }
    _user_map.erase(fd);
    if(_user_map.empty()){
        // 如果用户数为空，也直接解散房间
        auto server = Server::getInstance();    
        server->removeRoomById(getId());
    }
    return fd;
}

std::shared_ptr<User> Room::getUserByFd(int fd){
    if(_user_map.find(fd) == _user_map.end()){
        return nullptr;
    }
    return _user_map[fd];
}

void Room::sendMesg(int fd, MESGType type, const char* str){
    if(_user_map.find(fd) == _user_map.end()){
        return;
    }
    auto user = _user_map[fd];
    MSG_S_INFO* msg_s = user->getBasePacket(type);
    if(!msg_s) {
        std::cout << "Send msg error" << std::endl;
        return;
    }
    sendMesg(msg_s, str, strlen(str));
}

void Room::sendMesg(int fd, MESGType type, const void* data, uint16_t len){
    if(_user_map.find(fd) == _user_map.end()){
        return;
    }
    auto user = _user_map[fd];
    MSG_S_INFO* msg_s = user->getBasePacket(type);
    if(!msg_s) {
        std::cout << "Send msg error" << std::endl;
        return;
    }
    sendMesg(msg_s, data, len);
}



void Room::sendMesg(MSG_S_INFO* msg_s, const char* str){
    if(!msg_s || !str) return;
    sendMesg(msg_s, str, strlen(str));
}

// 
void Room::sendMesg(MSG_S_INFO* msg_s, const void* data, uint16_t len){
    if(!msg_s) return;
    if(!data) len = 0;
    u8u16(msg_s->msg.len) = MSG_S_HEAD_SIZE + len;
    if(len != 0) memcpy(msg_s->msg.data, data, len);
    msg_s->msg.crc = 0X00; // TODO 
    sendMesg(msg_s);
}

void Room::sendMesgToOne(MSG_S_INFO* msg_s){
    // 发送给一个用户
    msg_s->value.store(1);
    auto user = getUserByFd(msg_s->fd);
    // 该操作可以放置到线程池中运行
    user->sendMesg(msg_s);
}

void Room::sendMesgToOther(MSG_S_INFO* msg_s){
    // 发送给其他用户
    msg_s->value.store(_user_map.size() - 1);
    for(auto user : _user_map){
        if(user.first == msg_s->fd) continue;
        user.second->sendMesg(msg_s);
    }
}

void Room::sendMesgToAll(MSG_S_INFO* msg_s){
    // 发送给所有用户
    msg_s->value.store(_user_map.size());
    // 注意此处如果在遍历时有用户加入或退出，可能会有访问冲突
    for(auto user : _user_map){
        user.second->sendMesg(msg_s);
    }
}



void Room::sendMesg(MSG_S_INFO* msg_s){
    if(!msg_s) return;

    switch (msg_s->msg.type) {
    case MESGType::CreatRoomResp:
    case MESGType::JoinRoomAlreadyJoin:
    case MESGType::JoinRoomNotFound:
    case MESGType::ExitRoomNotJoin:
        sendMesgToOne(msg_s);
        break;
    case MESGType::textMsgResp:
    case MESGType::audioMsgResp:
    case MESGType::videoMsgResp:
        sendMesgToOther(msg_s);   
        break;
    case MESGType::JoinRoomSuccess:
    case MESGType::ExitRoomSuccess:
        sendMesgToAll(msg_s);
        break;
    default:
        sendMesgToAll(msg_s);
        break;
    }


    // if(msg_s->msg.type == MESGType::CreatRoomResp || ){
    //     // 发送给一个用户
    //     msg_s->value.store(1);
    //     auto user = getUserByFd(msg_s->fd);
    //     // 该操作可以放置到线程池中运行
    //     user->sendMesg(msg_s);
    // } else if (msg_s->msg.type == MESGType::JoinRoomSuccess){
    //     // 发送给所有用户
    //     msg_s->value.store(_user_map.size());
    //     // 注意此处如果在遍历时有用户加入或退出，可能会有访问冲突
    //     for(auto user : _user_map){
    //         user.second->sendMesg(msg_s);
    //     }
    // } else{
    //     // 发送给其他用户
    //     msg_s->value.store(_user_map.size() - 1);
    //     for(auto user : _user_map){
    //         if(user.first == msg_s->fd) continue;
    //         user.second->sendMesg(msg_s);
    //     }
    // }
}








// int Room::broadcast(MSG_SEND* msg){
//     if(!msg) return 0;
//     int cnt = 0;
//     for(auto fd : _user_set){
//         if(fd == msg->fd) continue;
//         write(fd, msg, MSG_SEND_HEAD_SIZE);
//         write(fd, msg->data, u8u16(msg->len) - MSG_SEND_HEAD_SIZE);
//     }
    
//     if(msg->data){
//         delete msg->data;
//     } 
//     if(msg){
//         delete msg;
//     }
// }