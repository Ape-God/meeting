#ifndef __MYUTILS_RING_BUFFER_H__
#define __MYUTILS_RING_BUFFER_H__

#include "stdint.h"
#include <mutex>
#include <cstring>
namespace myutils {

// 判断x是否是2的次方
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x)-1)) == 0))

/**
 * @brief 仿照linux kfifo写的RingBuffer环形缓存
 * @author zengyf
 * @date 2023/03/17
 * @example
 *
 */
class RingBuffer {
public:
    RingBuffer(uint32_t size_ = 1024);
    ~RingBuffer();

    // 获取缓冲区长度
    uint32_t get_buffer_size();
    // 获取已缓存的数据长度
    uint32_t get_length();
    // 向缓冲区添加数据
    uint32_t push(void* data, uint32_t size);
    // 从缓冲区获取数据
    uint32_t pop(void* data, uint32_t size);
    // 获取缓冲区最前面的N个字节
    uint32_t top(void* data, uint32_t size);

private:
    void* buf;               // 缓冲区
    uint32_t size;          // 大小
    uint32_t in;            // 入口位置
    uint32_t out;           // 出口位置
    std::mutex mutex;  // 互斥锁
};

}  // namespace myutils

using myRingBuffer = myutils::RingBuffer;

#endif  // __MYUTILS_RING_BUFFER_H__
