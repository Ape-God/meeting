#include "RingBuffer.h"

#include <algorithm>
#include <memory>
#include <iostream>

RingBuffer::RingBuffer(uint32_t size_) : buf(NULL), size(size_), in(0), out(0) {
    // �ж�size�Ƿ�Ϊ2��ָ���η�
    if (!is_power_of_2(size)) {
        // �������2��ָ���η�����Ҫ���䵽��һ��ָ���η�
        for (int i = 30; i >= 0; i--) {
            if (size & (1 << i)) {
                size = (1 << (i + 1));
                break;
            }
        }
        fprintf(stdout, "Resize RingBuffer size is:%d\n", size);
    }
    buf = new uint8_t[size];
    if (buf == NULL) {
        fprintf(stderr, "Failed to malloc %d bytes memory for RingBuffer.\n", size);
    }
}

RingBuffer::~RingBuffer() {
    if (buf) {
        delete[] buf;
    }
}

uint32_t RingBuffer::get_buffer_size() { return this->size; }

uint32_t RingBuffer::get_length() {
    std::lock_guard<std::mutex> lg(mux);
    return this->in - this->out;
}

uint32_t RingBuffer::push(void* data, uint32_t size) {
    // �ж�������Ч��
    if (!data || size == 0) {
        return 0;
    }
    // ��������
    std::lock_guard<std::mutex> lg(mux);
    // �жϻ������Ƿ����㹻�ռ�
    size = std::min(size, this->size - (this->in - this->out));
    /* first put the data starting from fifo->in to buffer end */
    uint32_t len = 0;
    len = std::min(size, this->size - (this->in & (this->size - 1)));
    memcpy((uint8_t*)this->buf + (this->in & (this->size - 1)), data, len);
    /* then put the rest (if any) at the beginning of the buffer */
    memcpy((uint8_t*)this->buf, (uint8_t*)data + len, size - len);
    this->in += size;
    return size;
}

uint32_t RingBuffer::pop(void* data, uint32_t size) {
    // �ж�������Ч��
    if (!data || size == 0) {
        return 0;
    }
    // ��������
    std::lock_guard<std::mutex> lg(mux);
    // �жϻ������Ƿ����㹻��������
    size = std::min(size, this->size - (this->in - this->out));
    /* first get the data from fifo->out until the end of the buffer */
    uint32_t len = 0;
    len = std::min(size, this->size - (this->out & (this->size - 1)));
    memcpy(data, (uint8_t*)this->buf + (this->out & (this->size - 1)), len);
    /* then get the rest (if any) from the beginning of the buffer */
    memcpy((uint8_t*)data + len, this->buf, size - len);
    this->out += size;

    if (this->in == this->out) {
        this->in = this->out = 0;
    }
    return size;
}

uint32_t RingBuffer::top(void* data, uint32_t size) {
    // �ж�������Ч��
    if (!data || size == 0) {
        return 0;
    }
    // ��������
    std::lock_guard<std::mutex> lg(mux);
    // �жϻ������Ƿ����㹻��������
    size = std::min(size, this->size - (this->in - this->out));
    /* first get the data from fifo->out until the end of the buffer */
    uint32_t len = 0;
    len = std::min(size, this->size - (this->out & (this->size - 1)));
    memcpy(data, (uint8_t*)this->buf + (this->out & (this->size - 1)), len);
    /* then get the rest (if any) from the beginning of the buffer */
    memcpy((uint8_t*)data + len, this->buf, size - len);
    return size;
}

