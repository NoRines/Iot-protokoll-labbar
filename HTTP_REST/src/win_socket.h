#ifdef _WIN32

#ifndef WIN_SOCKET_H
#define WIN_SOCKET_H

#include "socket.h"

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>


class WinSocket : public Socket
{
public:
	WinSocket(const WinSocket& s) = delete;
	WinSocket& operator=(const WinSocket& s) = delete;

	WinSocket(SocketType type);
	~WinSocket();

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
	WinSocket(SOCKET s);

	SOCKET sock;
};

#endif
#endif