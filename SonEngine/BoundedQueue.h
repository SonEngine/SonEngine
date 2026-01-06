#pragma once

#include "FramePacket.h"
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>

template<typename T>
class BoundedQueue {
public:
	BoundedQueue(size_t capacity = 1);
	bool Push(const T& packet);
	bool Push(T&& packet);

	bool Pop(T& packet);
	bool TryPop(T& packet);

private:
	size_t m_capacity;
	std::mutex q_mutex;
	
	std::condition_variable m_notEmpty;
	std::condition_variable m_notFull;

	std::queue<T> m_q;
};