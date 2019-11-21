#ifdef _WIN32

#include "win_socket.h"


#pragma comment(lib, "Ws2_32.lib")
struct addrinfo *result = NULL, *ptr = NULL, hints;


int WinSocket::numSocketsActive = 0;

static void WinSocket::initWsa()
{
	if(numSocketsActive == 0)
	{
		// Init winsock
		WSADATA wsaData;

		// Initialize Winsock
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			printf("WSAStartup failed: %d\n", iResult);
			throw std::string("Error when init WSA: %d\n", WSAGetLastError());
		}
	}
	numSocketsActive++;
}

static void WinSocket::quitWsa()
{
	if(numSocketsActive == 1)
		WSACleanup();
	numSocketsActive--;
}

WinSocket::WinSocket(SOCKET s) : SocketInterface(), sock(s)
{
	initWsa();
}

WinSocket::WinSocket(SocketType type) : SocketInterface()
{
	initWsa();

	sock = INVALID_SOCKET;

	int sockType = 0;
	switch(type)
	{
		case SocketType::STREAM:
			sockType = SOCK_STREAM;
			break;
		case SocketType::DGRAM:
			sockType = SOCK_DGRAM;
			break;
	}

	sock = ::socket(AF_INET, sockType, 0);
	if (sock == INVALID_SOCKET)
	{
		throw std::string("creating socket failed: %d\n", WSAGetLastError());
	}
}

WinSocket::~WinSocket()
{
	close();

	quitWsa();
}

void WinSocket::connect(const std::string& address, int port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

	int bindRes = ::connect(sock, (sockaddr*)&addr, sizeof(addr));
	if (bindRes == SOCKET_ERROR)
	{
		throw std::string("connect failed with error %d\n", WSAGetLastError());
	}
}

void WinSocket::bind(const std::string& address, int port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

	int bindRes = ::bind(sock, (sockaddr*)&addr, sizeof(addr));
	if (bindRes == SOCKET_ERROR)
	{
		throw std::string("bind failed with error %d\n", WSAGetLastError());
	}
}

void WinSocket::listen()
{
	int listenRes = ::listen(sock, SOMAXCONN);
	if (listenRes == SOCKET_ERROR)
	{
		throw std::string("listen failed with error %d\n", WSAGetLastError());
	}
}

SocketInterface* WinSocket::accept()
{
	SOCKET ClientSocket = ::accept(sock, (sockaddr*)NULL, NULL);
	if (ClientSocket == INVALID_SOCKET)
	{
		throw std::string("accept failed with error %d\n", WSAGetLastError());
	}
	return new WinSocket(ClientSocket);
}

int WinSocket::send(const std::string& msg)
{
	int bytesSent = ::send(sock, msg.c_str(), msg.size() + 1, 0);

	if (bytesSent == SOCKET_ERROR)
	{
		throw std::string("send failed with error %d\n", WSAGetLastError());
	}

	return bytesSent;
}

int WinSocket::receive(char* buf, int bufSize)
{
	int bytesRecived = recv(sock, buf, bufSize, 0);

	if (bytesRecived == SOCKET_ERROR)
	{
		throw std::string("recv failed with error %d\n", WSAGetLastError());
	}

	return bytesRecived;
}

void WinSocket::shutdown(SocketShutdownType type)
{
	int t = 0;
	switch (type)
	{
	case SocketShutdownType::RD:
		t = SD_RECEIVE;
		break;
	case SocketShutdownType::WR:
		t = SD_SEND;
		break;
	case SocketShutdownType::RDWR:
		t = SD_BOTH;
		break;
	}


	int shutdownRes = ::shutdown(sock, t); // SD_SEND, SD_RECEIVE
	if (shutdownRes == SOCKET_ERROR)
	{
		throw std::string("shutdown failed with error %d\n", WSAGetLastError());
	}
}

void WinSocket::close()
{
	closesocket(sock);
}

Address WinSocket::getAddress()
{
	sockaddr_in hint;
	socklen_t addrSize = sizeof(hint);
	int res = ::getsockname(sock, (sockaddr*)&hint, &addrSize);

	if (res == SOCKET_ERROR)
	{
		throw std::string("Error getting address/name %d\n", WSAGetLastError());
	}

	char buf[INET_ADDRSTRLEN];
	memset(buf, 0, INET_ADDRSTRLEN);

	::inet_ntop(AF_INET, &hint.sin_addr, buf, INET_ADDRSTRLEN);

	return{ std::string(buf), ntohs(hint.sin_port) };
}
#endif
