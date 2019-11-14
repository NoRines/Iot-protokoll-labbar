#include <iostream>
#include <memory>
#include <thread>
#include "win_socket.h"
#include "unix_socket.h"
#include <cstring>
#include <thread>

#ifdef _WIN32
#include <Ws2tcpip.h>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
#endif

void msgHandler(Socket* sock) {

	std::unique_ptr<Socket> clientSock(sock);

	auto clientAddress = clientSock->getAddress();
	std::cout << clientAddress.host << " : " << clientAddress.port << std::endl;

	char buf[4096];

	memset(buf, 0, 4096);
	int bytesRecived = clientSock->receive(buf, 4096);

	if (bytesRecived == 0)
	{
		std::cout << "Dissconnected" << std::endl;
		clientSock->shutdown(SocketShutdownType::RDWR);
		clientSock->close();
		return;
	}

	std::cout << "Recived> " << std::string(buf, 0, bytesRecived) << std::endl;
	std::string resp = std::string(buf, 0, bytesRecived);
	int body_pos = resp.find("\r\n\r\n");
	resp = "<h1>resp:"+resp.substr(resp.find("\r\n\r\n"), bytesRecived - body_pos)+"</h1>";
	//clientSock->send(std::string(buf, 0, bytesRecived));
	clientSock->send("HTTP/1.1 200 OK \r\n\r\n"+resp);
	clientSock->shutdown(SocketShutdownType::RDWR);
	clientSock->close();
}

int main(int argc, char** argv)
{
#ifdef _WIN32
	// Init winsock
	WSADATA wsaData;

	// Initialize Winsock
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	try
	{
		std::unique_ptr<Socket> serverSock(new WinSocket(SocketType::STREAM));

		serverSock->bind("127.0.0.1", 80);

		serverSock->listen();
		std::cout << "Listening..." << std::endl;

		while (1)
		{
			Socket* sock = serverSock->accept();
			std::thread handlerThread(msgHandler, sock);
			handlerThread.detach();
		}
		serverSock->shutdown(SocketShutdownType::RDWR);
		serverSock->close();
	}
	catch (const char* errorMsg)
	{
		std::cerr << errorMsg << std::endl;
		return 1;
	}
	catch (std::string err)
	{
		std::cerr << err << std::endl;
	}

	// Shutdown winsock
	WSACleanup();

#endif
#ifdef __linux__
	try
	{
		std::unique_ptr<Socket> serverSocket(new UnixSocket(SocketType::STREAM));

		serverSocket->bind("127.0.0.1", 80);

		serverSocket->listen();

		while(true)
		{
			Socket* sock = serverSocket->accept();

			std::thread newThread(msgHandler, sock);
			newThread.detach();
		}

		serverSocket->shutdown(SocketShutdownType::RDWR);
		serverSocket->close();

	} catch(const char* error)
	{
		std::cout << error << std::endl;
		return 1;
	} catch(std::string error)
	{
		std::cout << error << std::endl;
		return 1;
	}

#endif
	return 0;
}
