#pragma once

#include "FramePacket.h"
#include <mutex>
#include <deque>
#include <condition_variable>
#include <atomic>

class BoundedQueue {
public:
	explicit BoundedQueue(size_t capacity = 1);

	bool Pop(FramePacket& packet);
	bool Push(const FramePacket& packet);

private:
	size_t m_capacity;
	std::mutex q_mutex;
	
	std::condition_variable m_notEmpty;
	std::condition_variable m_notFull;

	std::deque<FramePacket> m_q;
};