#ifndef SENDQUEUE_H
#define SENDQUEUE_H

#include <utility>

class SocketInterface;

struct OutgoingMessage
{
	char* data;
	int length;
};

using outmsg = std::pair<OutgoingMessage, SocketInterface*>;


void interruptQueue();
void pushQueue(outmsg outgoing);
outmsg popQueue();

#endif // !SENDQUEUE_H


