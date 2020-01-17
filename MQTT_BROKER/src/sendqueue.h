#ifndef SENDQUEUE_H
#define SENDQUEUE_H

#include <utility>
#include <vector>
#include <cstdint>

class SocketInterface;

//struct OutgoingMessage
//{
//	char* data;
//	int length;
//};

using OutgoingMessage = std::vector<uint8_t>;

using outmsg = std::pair<OutgoingMessage, SocketInterface*>;


void interruptQueue();
void pushQueue(outmsg outgoing);
outmsg popQueue();

#endif // !SENDQUEUE_H


