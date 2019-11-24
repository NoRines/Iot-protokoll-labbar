#ifndef SOCKET_INTERFACE_H
#define SOCKET_INTERFACE_H

#include <string>
#include <utility>

enum class SocketType
{
	DGRAM, // Datagram type (UDP)
	STREAM // Stream type (TCP)
};

enum class SocketShutdownType
{
	RD, // Disables further receive ops
	WR, // Disables further send ops
	RDWR // Disables further send and receive ops
};

struct Address
{
	std::string host;
	int port;
};

class SocketInterface
{
public:
	SocketInterface(const SocketInterface& s) = delete;
	SocketInterface& operator=(const SocketInterface& s) = delete;

	SocketInterface() {}
	virtual ~SocketInterface() = default;

	virtual void connect(const Address& address) = 0;
	virtual void bind(const Address& address) = 0;
	virtual void listen() = 0;
	virtual SocketInterface* accept() = 0;

	virtual int send(const std::string& msg) = 0;
	virtual int sendTo(const std::string& msg, const Address& address) = 0;

	virtual int receive(char* buf, int bufSize) = 0;
	virtual int receiveFrom(char* buf, int bufSize, Address& address) = 0;
	
	virtual void shutdown(SocketShutdownType type) = 0;
	virtual void close() = 0;
	virtual Address getAddress() = 0;
};

#endif
