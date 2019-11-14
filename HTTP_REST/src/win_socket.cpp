#ifdef _WIN32

#include "win_socket.h"


#pragma comment(lib, "Ws2_32.lib")
struct addrinfo *result = NULL, *ptr = NULL, hints;

WinSocket::WinSocket(SOCKET s) : Socket(), sock(s)
{}

WinSocket::WinSocket(SocketType type) : Socket()
{

	sock = INVALID_SOCKET;

	sock = ::socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		throw std::string("creating socket failed: %d\n", WSAGetLastError()).c_str();
	}
}

WinSocket::~WinSocket()
{
	close();
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
		throw std::string("connect failed with error %d\n", WSAGetLastError()).c_str();
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
		throw std::string("bind failed with error %d\n", WSAGetLastError()).c_str();
	}
}

void WinSocket::listen()
{
	int listenRes = ::listen(sock, SOMAXCONN);
	if (listenRes == SOCKET_ERROR)
	{
		throw std::string("listen failed with error %d\n", WSAGetLastError()).c_str();
	}
}

Socket* WinSocket::accept()
{
	SOCKET ClientSocket = ::accept(sock, (sockaddr*)NULL, NULL);
	if (ClientSocket == INVALID_SOCKET)
	{
		throw std::string("accept failed with error %d\n", WSAGetLastError()).c_str();
	}
	return new WinSocket(ClientSocket);
}

int WinSocket::send(const std::string& msg)
{
	int bytesSent = ::send(sock, msg.c_str(), msg.size() + 1, 0);

	if (bytesSent == SOCKET_ERROR)
	{
		throw std::string("send failed with error %d\n", WSAGetLastError()).c_str();
	}

	return bytesSent;
}

int WinSocket::receive(char* buf, int bufSize)
{
	int bytesRecived = recv(sock, buf, bufSize, 0);

	if (bytesRecived == SOCKET_ERROR)
	{
		throw std::string("recv failed with error %d\n", WSAGetLastError()).c_str();
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
		throw std::string("shutdown failed with error %d\n", WSAGetLastError()).c_str();
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
		throw std::string("Error getting address/name %d\n", WSAGetLastError()).c_str();
	}

	char buf[INET_ADDRSTRLEN];
	memset(buf, 0, INET_ADDRSTRLEN);

	::inet_ntop(AF_INET, &hint.sin_addr, buf, INET_ADDRSTRLEN);

	return{ std::string(buf), ntohs(hint.sin_port) };
}
#endif