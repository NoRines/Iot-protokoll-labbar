#ifdef __linux__

#include "unix_socket.h"

#include <iostream>
#include <exception>

#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

UnixSocket::UnixSocket(int s) : SocketInterface(), sock(s)
{
}

UnixSocket::UnixSocket(SocketType type) : SocketInterface()
{
	//	int socket(int domain, int type, int protocol)
	//
	//	domain:
	//		AF_UNIX : UNIX domain sockets
	//		AF_UNSPEC : Unspecified
	//		AF_INET : Internet domain sockets for use with IPv4 addresses
	//		AF_INET6 : Internet domain sockets for use with IPv6 addresses 
	//
	//	type:
	//		SOCK_STREAM : Provides sequenced, reliable, bidirectional,
	//			connection-mode byte streams, and may provide a transmission
	//			mechanism for out-of-band data.
	//		SOCK_DGRAM : Provides datagrams, which are connectionless-mode,
	//			unreliable messages of fixed maximum length.
	//		SOCK_SEQPACKET : Provides sequenced, reliable, bidirectional,
	//			connection-mode transmission path for records. A record can be
	//			sent using one or more output operations and received using
	//			one or more input operations, but a single operation never
	//			transfers part of more than one record. Record boundaries are
	//			visible to the receiver via the MSG_EOR flag. 
	//
	//	protocol:
	//		Specifies a particular protocol to be used with the socket.
	//		Specifying a protocol of 0 causes socket() to use an unspecified
	//		default protocol appropriate for the requested socket type. 
	//
	//	return:
	//		Upon successful completion, socket() returns a nonnegative
	//		integer, the socket file descriptor. Otherwise a value of -1 is
	//		returned and errno is set to indicate the error. 
	//
	
	int t = SOCK_DGRAM;
	if(type == SocketType::STREAM)
		t = SOCK_STREAM;

	sock = socket(AF_INET, t, 0);
	if(sock < 0)
	{
		throw std::string("Could not create Socket.");
	}
}

UnixSocket::~UnixSocket()
{
	close();
}

void UnixSocket::connect(const Address& address)
{
	// 	int connect(int socket, const struct sockaddr* address,
	// 		socklen_t address_len)
	//
	//	socket:
	//		Specifies the file descriptor associated with the socket.
	//
	//	address:
	//		Points to a sockaddr structure containing the peer address. The
	//		length and format of the address depend on the address family of
	//		the socket.
	//
	//	address_len:
	//		Specifies the length of the sockaddr structure pointed to by the
	//		address argument. 
	//
	//	return:
	//		Upon successful completion, connect() returns 0. Otherwise, -1 is
	//		returned and errno is set to indicate the error. 
	//

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(address.port);
	inet_pton(AF_INET, address.host.c_str(), &hint.sin_addr);

	int connRes = ::connect(sock, (sockaddr*)(&hint), sizeof(hint));

	if(connRes < 0)
	{
		throw std::string("Connection failed.");
	}
}

void UnixSocket::bind(const Address& address)
{
	// 	int bind(int socket, const struct sockaddr *address,
	// 		socklen_t address_len);
	//
	//	socket: 
	//		Specifies the file descriptor of the socket to be bound.  
	//
	//	address: 
	//		Points to a sockaddr structure containing the address to be bound 
	//		to the socket. The length and format of the address depend on the 
	//		address family of the socket.  
	//
	//	address_len: 
	//		Specifies the length of the sockaddr structure pointed to by the 
	//		address argument. 
	//
	//	return:
	//		Upon successful completion, bind() returns 0. Otherwise, -1 is 
	//		returned and errno is set to indicate the error. 
	//
	
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(address.port);
	inet_pton(AF_INET, address.host.c_str(), &hint.sin_addr);

	int bindRes = ::bind(sock, (sockaddr*)(&hint), sizeof(hint));

	if(bindRes < 0)
	{
		std::string error = (std::string("Could not bind the socket. ") + std::to_string(bindRes));
		throw error;
	}


}

void UnixSocket::listen()
{
	//	int listen(int socket, int backlog);
	//
	//	socket:
	//		socket to use
	//
	//	backlog:
	//		num incoming connections
	//
	//	return:
	//		Upon successful completions, listen() returns 0. Otherwise, -1 is
	//		returned and errno is set to indicate the error. 
	//	

	int listenRes = ::listen(sock, SOMAXCONN);

	if(listenRes < 0)
	{
		throw std::string("Could not start listen.");
	}
}

SocketInterface* UnixSocket::accept()
{
	//	int accept (int socket, struct sockaddr *address,
	//		socklen_t *address_len);
	//
	//	socket: 
	//		Specifies a socket that was created with socket(), has been bound
	//		to an address with bind(), and has issued a successful call to 
	//		listen().  
	//
	//	address: 
	//		Either a null pointer, or a pointer to a sockaddr structure where 
	//		the address of the connecting socket will be returned.  
	//
	//	address_len: 
	//		Points to a socklen_t which on input specifies the length of the 
	//		supplied sockaddr structure, and on output specifies the length of 
	//		the stored address. 
	//
	//	return:	
	//		Upon successful completion, accept() returns the non-negative file 
	//		descriptor of the accepted socket. Otherwise, -1 is returned and 
	//		errno is set to indicate the error. 	

	sockaddr_in clientAddr;
	socklen_t clientSize = sizeof(clientAddr);
	int clientSocket = ::accept(sock, (sockaddr*)(&clientAddr), &clientSize);

	if(clientSocket < 0)
	{
		throw std::string("Could not accept the client.");
	}

	return new UnixSocket(clientSocket);
}

int UnixSocket::send(const std::string& msg)
{
	//	ssize_t send(int socket, const void* buffer, size_t length, int flags)
	//
	//	socket:
	//		Specifies the socket file descriptor.
	//
	//	buffer:
	//		Points to the buffer containing the message to send.
	//
	//	length:
	//		Specifies the length of the message in bytes.
	//
	//	flags:
	//		Specifies the type of message transmission. Values of this 
	//		argument are formed by logically OR'ing zero or more of the 
	//		following flags:
	//		MSG_EOR:
	//			Terminates a record (if supported by the protocol)
	//		MSG_OOB:
	//			Sends out-of-band data on sockets that support out-of-band
	//			communications. The significance and semantics of out-of-band
	//			data are protocol-specific. 
	//
	//	return:
	//		Upon successful completion, send() returns the number of bytes
	//		sent. Otherwise, -1 is returned and errno is set to indicate the
	//		error. 
	//

	int sendRes = ::send(sock, msg.c_str(), msg.size() + 1, 0);

	if(sendRes < 0)
	{
		throw std::string("Message could not be sent.");
	}

	return sendRes;
}

int UnixSocket::sendTo(const std::string& msg, const Address& address)
{
	sockaddr_in destAddr;
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(address.port);
	inet_pton(AF_INET, address.host.c_str(), &destAddr.sin_addr);

	int bytesSent = ::sendto(sock, msg.c_str(), msg.size() + 1, 0, (sockaddr*)&destAddr, sizeof(destAddr));

	if(bytesSent < 0)
	{
		throw std::string("Message could not be sent to destination.");
	}

	return bytesSent;
}

int UnixSocket::receive(char* buf, int bufSize)
{
	//	ssize_t send(int socket, const void* buffer, size_t length, int flags)
	//
	//	socket:
	//		Specifies the socket file descriptor.
	//
	//	buffer:
	//		Points to the buffer containing the message to send.
	//
	//	length:
	//		Specifies the length of the message in bytes.
	//
	//	flags:
	//		Specifies the type of message transmission. Values of this 
	//		argument are formed by logically OR'ing zero or more of the 
	//		following flags:
	//		MSG_EOR:
	//			Terminates a record (if supported by the protocol)
	//		MSG_OOB:
	//			Sends out-of-band data on sockets that support out-of-band
	//			communications. The significance and semantics of out-of-band
	//			data are protocol-specific. 
	//
	//	return:
	//		Upon successful completion, send() returns the number of bytes
	//		sent. Otherwise, -1 is returned and errno is set to indicate the
	//		error. 
	//

	int recvRes = ::recv(sock, buf, bufSize, 0);

	if(recvRes < 0)
	{
		throw std::string("Error when receiving.");
	}

	return recvRes;
}

int UnixSocket::receiveFrom(char* buf, int bufSize, Address& address)
{
	sockaddr_in srcAddr;
	srcAddr.sin_family = AF_INET;
	srcAddr.sin_port = htons(address.port);
	inet_pton(AF_INET, address.host.c_str(), &srcAddr.sin_addr);
	socklen_t addrSize = sizeof(srcAddr);

	int bytesRecived = ::recvfrom(sock, buf, bufSize, 0, (sockaddr*)&srcAddr, &addrSize);

	if(bytesRecived < 0)
	{
		throw std::string("Failed when receiving from an address");
	}

	return bytesRecived;
}

void UnixSocket::shutdown(SocketShutdownType type)
{
	//	int shutdown(int socket, int how);
	//
	//	socket:
	//		Specifies the file descriptor of the socket.  
	//
	//	how: 
	//		Specifies the type of shutdown. The values are as follows:
	//		SHUT_RD: 
	//			Disables further receive operations.  
	//		SHUT_WR: 
	//			Disables further send operations.  
	//		SHUT_RDWR: 
	//			Disables further send and receive operations. 
	//
	//	return:
	//		Upon successful completion, shutdown() returns 0. Otherwise, -1 is
	//		returned and errno is set to indicate the error. 
	
	int t = 0;
	switch(type)
	{
		case SocketShutdownType::RD:
			t = SHUT_RD;
			break;
		case SocketShutdownType::WR:
			t = SHUT_WR;
			break;
		case SocketShutdownType::RDWR:
			t = SHUT_RDWR;
			break;
	}
	int shutRes = ::shutdown(sock, t);

	if(shutRes < 0)
	{
		throw std::string("Fail on shutdown of socket.");
	}
}

void UnixSocket::close()
{
	::close(sock);
}

Address UnixSocket::getAddress()
{
	sockaddr_in hint;
	socklen_t addrSize = sizeof(hint);

	int res = ::getsockname(sock, (sockaddr*)(&hint), &addrSize);

	if(res < 0)
	{
		throw std::string("Error getting socket address/name.");
	}

	char buf[INET_ADDRSTRLEN];
	memset(buf, 0, INET_ADDRSTRLEN);

//const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
	::inet_ntop(AF_INET, &hint.sin_addr, buf, INET_ADDRSTRLEN);

	return {std::string(buf), ntohs(hint.sin_port)};
}

#endif
