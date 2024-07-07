#include "DequeBuffer.h"

DequeBuffer::DequeBuffer(uint32_t pkg_size, uint32_t pkg_cnt) :pkg_size_(pkg_size), pkg_cnt_(pkg_cnt)
{
	for (int i = 0; i < pkg_cnt; i++)
	{
		// 申请单帧字节长度的数组
		uint8_t* pData = new uint8_t[pkg_size];
		this->pushBackTo_dequeBuffEmpty(pData);
	}
}

DequeBuffer::~DequeBuffer()
{
	uint8_t* data = NULL;
	while (false == dequeBuffEmpty.empty())
	{
		data = dequeBuffEmpty.front();
		free(data);
		dequeBuffEmpty.pop_front();
	}
	while (false == dequeBuffUse.empty())
	{
		data = dequeBuffUse.front();
		free(data);
		dequeBuffUse.pop_front();
	}
}

uint8_t* DequeBuffer::popFrontFrom_dequeBuffEmpty()
{
	std::lock_guard<std::mutex> lg(dequeBuffEmptyMutex);
	if (dequeBuffEmpty.empty())
	{
		return NULL;
	}
	uint8_t* data = dequeBuffEmpty.front();
	dequeBuffEmpty.pop_front();
	return data;
}

void DequeBuffer::pushBackTo_dequeBuffEmpty(uint8_t* a)
{
	std::lock_guard<std::mutex> lg(dequeBuffEmptyMutex);
	dequeBuffEmpty.push_back(a);
}

uint8_t* DequeBuffer::popFrontFrom_dequeBuffUse()
{
	std::lock_guard<std::mutex> lg(dequeBuffUseMutex);
	if (dequeBuffUse.empty())
	{
		return NULL;
	}
	uint8_t* data = dequeBuffUse.front();
	dequeBuffUse.pop_front();
	return data;
}

void DequeBuffer::pushBackTo_dequeBuffUse(uint8_t* a)
{
	std::lock_guard<std::mutex> lg(dequeBuffUseMutex);
	dequeBuffUse.push_back(a);
}