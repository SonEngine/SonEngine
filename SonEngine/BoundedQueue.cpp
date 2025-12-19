#include "BoundedQueue.h"
#include "GraphicsCommon.h"

using namespace Graphics;

BoundedQueue::BoundedQueue(size_t capacity)
	:m_capacity(capacity)
{
}

bool BoundedQueue::Pop(FramePacket& packet)
{
	std::unique_lock<std::mutex> lock(q_mutex);
	m_notFull.wait(lock, [&] {return  !world->isRunning || !m_q.empty(); });

	if (!world->isRunning)
	{
		return false;
	}
	packet = m_q.front();
	m_q.pop_front();

	m_notFull.notify_one();

	return true;
}

bool BoundedQueue::Push(const FramePacket& packet)
{
	std::unique_lock<std::mutex> lock(q_mutex);
	m_notFull.wait(lock, [&] {return  !world->isRunning || m_q.size() < m_capacity; });

	if (!world->isRunning)
	{
		return false;
	}
	m_q.push_back(packet);
	m_notEmpty.notify_one();

	return true;
}
