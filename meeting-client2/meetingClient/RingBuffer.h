#pragma once
#include "stdint.h"
#include <mutex>
#include <cstring>
#include <iostream>

// �ж�x�Ƿ���2�Ĵη�
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x)-1)) == 0))

/**
 * @brief ����linux kfifoд��RingBuffer���λ���
 * @author zengyf
 * @date 2023/03/17
 * @example
 *
 */
class RingBuffer {
public:
    RingBuffer(uint32_t size_ = 1024);
    ~RingBuffer();

    // ��ȡ����������
    uint32_t get_buffer_size();
    // ��ȡ�ѻ�������ݳ���
    uint32_t get_length();
    // �򻺳����������
    uint32_t push(void* data, uint32_t size);
    // �ӻ�������ȡ����
    uint32_t pop(void* data, uint32_t size);
    // ��ȡ��������ǰ���N���ֽ�
    uint32_t top(void* data, uint32_t size);

private:
    void* buf;               // ������
    uint32_t size;          // ��С
    uint32_t in;            // ���λ��
    uint32_t out;           // ����λ��
    std::mutex mux;  // ������
};