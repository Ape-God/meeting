#pragma once
#include "stdint.h"
#include "string"
#include <atomic>

#define u8float(data) (*((float *)data))
#define u8double(data) (*((double *)data))
#define u8short(data) (*((short *)data))
#define u8int(data) (*((int *)data))
#define u8long(data) (*((long *)data))  // long��8Byte
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

static constexpr int MAX_DATA_SIZE = 1408; // ÿ�����ݷ�����1408�ֽ�����

/**
 * ��Ϣ���սṹ��
 * Test:55 AA 00 09 00 01 02 03 04
 * CreatRoom:55 AA 02 06 00 00
 * JoinRoom:55 AA 04 0B 00 00
 * ExitRoom:55 AA 06 06 00 00
 * textMsg:55 AA 08 0A 00 00 01 02 03 04
*/
struct MSG_C {
    uint8_t header[2];     // ����֡ͷ
    MESGType type;          // ��������
    uint8_t len[2];        // ���ݰ�����
    uint8_t crc;           // У��λ
    char data[MAX_DATA_SIZE];

    // ��ȡ��Ҫ���뷿���ID
    static std::string recvJoinRoom(MSG_C* rcv_msg) {
        return std::string(rcv_msg->data);
    }
};
static constexpr int MSG_C_HEAD_SIZE = sizeof(MSG_C) - MAX_DATA_SIZE;



/**
 * ����ת���Ķ���ö����
*/
enum class MESGSendType : uint8_t {
    None = 0,   // ��ת��
    Client,     // ֻת���ͻ���
    RoomOther,  // ת�������������ˣ���ȥ����ͨ�ŵĿͻ���
    RoomAll     // ת�������������ˣ�����������
};

struct MSG_S {
    uint8_t header[2];  // ����֡ͷ
    MESGType type;      // ��������
    uint8_t len[2];     // ���ݰ�����
    uint8_t crc;        // У��λ����У�飬�����ֽ���ͣ�ȡ�Ͱ�λ��
    uint8_t ip[4];      // ����ĸ�IP��PORT��������Ӧ���û������ж��Ƿ�����IP��
    uint8_t port[2];    // ����ǣ��������������������ķ������ݣ�������ǣ����ʾ��ת��������
    char data[MAX_DATA_SIZE];
};
struct MSG_S_INFO {
    int fd;
    std::atomic<int> value;
    // MESGSendType send_type; // ���ݰ���������
    MSG_S msg;
};

static constexpr int MSG_S_HEAD_SIZE = sizeof(MSG_S) - MAX_DATA_SIZE;
