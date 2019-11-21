#ifdef _WIN32
#ifndef WIN_SOCKET_H
#define WIN_SOCKET_H

#include "socket_interface.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

class WinSocket : public SocketInterface
{
public:
	WinSocket(const WinSocket& s) = delete;
	WinSocket& operator=(const WinSocket& s) = delete;

	WinSocket(SocketType type);
	~WinSocket();

	void connect(const Address& address) override;
	void bind(const Address& address) override;
	void listen() override;
	SocketInterface* accept() override;

	int send(const std::string& msg) override;
	int sendTo(const std::string& msg, const Address& address) override;

	int receive(char* buf, int bufSize) override;
	int receiveFrom(char* buf, int bufSize, const Address& address) override;

	void shutdown(SocketShutdownType type) override;
	void close() override;
	Address getAddress() override;

private:

	static void initWsa();
	static void quitWsa();
	static int numSocketsActive;
	
	SOCKET sock;
	WinSocket(SOCKET s);
	
};

#endif
#endif // _WIN32
