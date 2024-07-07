#pragma once
#include "stdint.h"
#include "string"
#include <atomic>

#define u8float(data) (*((float *)data))
#define u8double(data) (*((double *)data))
#define u8short(data) (*((short *)data))
#define u8int(data) (*((int *)data))
#define u8long(data) (*((long *)data))  // long是8Byte
#define u8u16(data) (*((uint16_t *)data))
#define u8u16int(data) (*((int_least16_t *)data)) 
#define u816(data) (*((int16_t *)data))
#define u8u32(data) (*((uint32_t *)data))
#define u8u64(data) (*((uint64_t *)data))

enum class MESGType : uint8_t {
    Test = 0B00000000,
    TestMulit,
    CreatRoom,
    CreatRoomResp,
    JoinRoom,
    JoinRoomSuccess,
    JoinRoomAlreadyJoin,
    JoinRoomNotFound,
    ExitRoom,
    ExitRoomSuccess,
    ExitRoomNotJoin,
    textMsg,
    textMsgResp,
    audioMsg,
    audioMsgResp,
    audioOpen,
    audioClose,
    videoMsg,
    videoMsgResp,
    videoOpen,
    videoClose
};

static constexpr int MAX_DATA_SIZE = 1408; // 每次数据发送在1408字节以内

/**
 * 消息接收结构体
 * Test:55 AA 00 09 00 01 02 03 04
 * CreatRoom:55 AA 02 06 00 00
 * JoinRoom:55 AA 04 0B 00 00
 * ExitRoom:55 AA 06 06 00 00
 * textMsg:55 AA 08 0A 00 00 01 02 03 04
*/
struct MSG_C {
    uint8_t header[2];     // 数据帧头
    MESGType type;          // 数据类型
    uint8_t len[2];        // 数据包长度
    uint8_t crc;           // 校验位
    char data[MAX_DATA_SIZE];

    // 获取需要加入房间的ID
    static std::string recvJoinRoom(MSG_C* rcv_msg) {
        return std::string(rcv_msg->data);
    }
};
static constexpr int MSG_C_HEAD_SIZE = sizeof(MSG_C) - MAX_DATA_SIZE;



/**
 * 数据转发的对象枚举类
*/
enum class MESGSendType : uint8_t {
    None = 0,   // 不转发
    Client,     // 只转给客户端
    RoomOther,  // 转给房间内其他人，除去正在通信的客户端
    RoomAll     // 转给房间内所有人，包括发送者
};

struct MSG_S {
    uint8_t header[2];  // 数据帧头
    MESGType type;      // 数据类型
    uint8_t len[2];     // 数据包长度
    uint8_t crc;        // 校验位（和校验，所有字节求和，取低八位）
    uint8_t ip[4];      // 针对哪个IP：PORT做出的相应，用户可以判断是否自身IP，
    uint8_t port[2];    // 如果是，则表明是与服务器交互的返回数据，如果不是，则表示是转发的数据
    char data[MAX_DATA_SIZE];
};
struct MSG_S_INFO {
    int fd;
    std::atomic<int> value;
    // MESGSendType send_type; // 数据包发送类型
    MSG_S msg;
};

static constexpr int MSG_S_HEAD_SIZE = sizeof(MSG_S) - MAX_DATA_SIZE;
