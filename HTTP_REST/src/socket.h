#ifndef SOCKET_H
#define SOCKET_H

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

class Socket
{
public:
	Socket(const Socket& s) = delete;
	Socket& operator=(const Socket& s) = delete;

	Socket() {}
	virtual ~Socket() = default;

	virtual void connect(const std::string& address, int port) = 0;
	virtual void bind(const std::string& address, int port) = 0;
	virtual void listen() = 0;
	virtual Socket* accept() = 0;
	virtual int send(const std::string& msg) = 0;
	virtual int receive(char* buf, int bufSize) = 0;
	virtual void shutdown(SocketShutdownType type) = 0;
	virtual void close() = 0;
	virtual Address getAddress() = 0;
};

#endif
