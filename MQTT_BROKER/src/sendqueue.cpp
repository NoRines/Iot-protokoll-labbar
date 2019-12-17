#include "sendqueue.h"
#include <queue>
#include <mutex>

static std::queue<outmsg> outgoingqueue;
static std::mutex queueLock;
static std::condition_variable cv;

void pushQueue(outmsg outgoing)
{
	std::lock_guard<std::mutex> lock(queueLock);
	outgoingqueue.push(outgoing);
	cv.notify_one();
}

outmsg popQueue()
{
	std::unique_lock<std::mutex> lock(queueLock);
	while (outgoingqueue.empty())
	{
		cv.wait(lock);
	}
	outmsg val = outgoingqueue.front();
	outgoingqueue.pop();
	return val;
}
