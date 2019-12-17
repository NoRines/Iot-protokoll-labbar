#include "sendqueue.h"
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>


static volatile bool programRunning = true;
static std::queue<outmsg> outgoingqueue;
static std::mutex queueLock;
static std::condition_variable cv;

void interruptQueue()
{
	programRunning = false;
	cv.notify_all();
}

void pushQueue(outmsg outgoing)
{
	std::lock_guard<std::mutex> lock(queueLock);
	outgoingqueue.push(outgoing);
	cv.notify_one();
}

outmsg popQueue()
{
	std::unique_lock<std::mutex> lock(queueLock);

	cv.wait(lock, [&](){ return !outgoingqueue.empty() || !programRunning; });

	if(!programRunning)
	{
		throw std::string("Program stopped");
	}

	outmsg val = outgoingqueue.front();
	outgoingqueue.pop();
	return val;
}
