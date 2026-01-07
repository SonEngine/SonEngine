#include "BoundedQueue.h"
#include "GraphicsCommon.h"
#include "RenderCommand.h"
#include "GameCommand.h"

using namespace Graphics;

template<typename T>
BoundedQueue<T>::BoundedQueue(size_t capacity):
	m_capacity(capacity)
{
}


template<typename T>
bool BoundedQueue<T>::Push(const T& packet)
{
	std::unique_lock<std::mutex> lock(q_mutex);
	m_notFull.wait(lock, [&] {return  !world->isRunning || m_q.size() < m_capacity; });

	if (!world->isRunning)
	{
		return false;
	}
	m_q.push(packet);
	lock.unlock();

	m_notEmpty.notify_one();

	return true;
}

template<typename T>
bool BoundedQueue<T>::Push(T&& packet)
{
	std::unique_lock<std::mutex> lock(q_mutex);
	m_notFull.wait(lock, [&] {return  !world->isRunning || m_q.size() < m_capacity; });

	if (!world->isRunning)
	{
		return false;
	}
	m_q.push(std::move(packet));
	lock.unlock();
	
	m_notEmpty.notify_one();

	return true;
}

template<typename T>
bool BoundedQueue<T>::Pop(T& packet)
{
	std::unique_lock<std::mutex> lock(q_mutex);
	m_notEmpty.wait(lock, [&] {return  !world->isRunning || !m_q.empty(); });

	if (!world->isRunning)
	{
		return false;
	}
	packet = m_q.front();
	m_q.pop();

	lock.unlock();
	m_notFull.notify_one();

	return true;
}

template<typename T>
bool BoundedQueue<T>::TryPop(T& packet)
{
	std::unique_lock<std::mutex> lock(q_mutex);
	if (!world->isRunning || m_q.empty())
	{
		return false;
	}
	packet = m_q.front();
	m_q.pop();

	lock.unlock();
	m_notFull.notify_one();

	return true;
}

template class BoundedQueue<FramePacket>; 
template class BoundedQueue<RenderCmd>; 
template class BoundedQueue<GameCmd>;