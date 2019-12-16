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

	void connect(const Address& address) override;
	void bind(const Address& address) override;
	void listen() override;
	SocketInterface* accept() override;

	int send(const char* msg, int msgSize) override;
	int sendTo(const char* msg, int msgSize, const Address& address) override;

	int send(const std::string& msg) override;
	int sendTo(const std::string& msg, const Address& address) override;

	int receive(char* buf, int bufSize) override;
	int receiveFrom(char* buf, int bufSize, Address& address) override;

	void shutdown(SocketShutdownType type) override;
	void close() override;
	Address getAddress() override;

	void setTimeout(int seconds) override;

private:
	UnixSocket(int s);

	int sock;
};

#endif

#endif
