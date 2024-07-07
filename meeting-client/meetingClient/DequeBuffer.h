#pragma once

#include <deque>
#include <mutex>

#include <QObject>
#include <QDebug>
#include <QMutex>

#define MAXCHAINDEQUESIZE	256



class DequeBuffer
{
public:
	DequeBuffer(uint32_t pkg_size, uint32_t pkg_cnt = MAXCHAINDEQUESIZE);
	~DequeBuffer();
	uint32_t pkg_size_;
	uint32_t pkg_cnt_;
	std::deque<uint8_t*> dequeBuffEmpty;
	std::deque<uint8_t*> dequeBuffUse;
	std::mutex dequeBuffEmptyMutex;
	std::mutex dequeBuffUseMutex;
	uint8_t* popFrontFrom_dequeBuffEmpty();
	void pushBackTo_dequeBuffEmpty(uint8_t*);
	uint8_t* popFrontFrom_dequeBuffUse();
	void pushBackTo_dequeBuffUse(uint8_t*);
};
