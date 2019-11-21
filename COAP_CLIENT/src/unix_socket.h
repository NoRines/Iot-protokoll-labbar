#ifdef __linux__

#ifndef UNIX_SOCKET_H
#define UNIX_SOCKET_H

#include "socket_interface.h"

class UnixSocket : public SocketInterface
{
public:
	UnixSocket(const UnixSocket& s) = delete;
	UnixSocket& operator=(const UnixSocket& s) = delete;

	UnixSocket(SocketType type);
	~UnixSocket();

	void connect(const std::string& address, int port) override;
	void bind(const std::string& address, int port) override;
	void listen() override;
	SocketInterface* accept() override;
	int send(const std::string& msg) override;
	int receive(char* buf, int bufSize) override;
	void shutdown(SocketShutdownType type) override;
	void close() override;
	Address getAddress() override;

private:
	UnixSocket(int s);

	int sock;
};

#endif

#endif
