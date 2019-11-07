#ifndef UNIX_SOCKET_H
#define UNIX_SOCKET_H

#include "socket.h"

class UnixSocket : public Socket
{
public:
	UnixSocket(const Socket& s) = delete;
	UnixSocket& operator=(const UnixSocket& s) = delete;

	UnixSocket(SocketType type);
	~UnixSocket();

	void connect(const std::string& address, int port) override;
	void bind(const std::string& address, int port) override;
	void listen() override;
	Socket* accept() override;
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
