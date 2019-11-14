#include <iostream>
#include <memory>
#include "win_socket.h"
#include "unix_socket.h"
#include <cstring>
#include <thread>

#ifdef _WIN32
#include <Ws2tcpip.h>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
#endif

void msgHandler(Socket* sock)
{
	std::unique_ptr<Socket> clientSocket(sock);

	auto clientAddr = clientSocket->getAddress();
	std::cout << clientAddr.host << " : " << clientAddr.port << std::endl;

	char buf[4096];

	memset(buf, 0, 4096);
	int numBytes = clientSocket->receive(buf, 4096);

	if(numBytes == 0)
	{
		std::cout << "Client dissconnected" << std::endl;
		clientSocket->shutdown(SocketShutdownType::RDWR);
		clientSocket->close();
		return;
	}

	std::cout << "Recived: " << std::string(buf, 0, numBytes) << std::endl;

	clientSocket->send("HTTP/1.1 200 OK\r\n\r\n<h1>HENLO MINE FREN</h1>");

	clientSocket->shutdown(SocketShutdownType::RDWR);
	clientSocket->close();

	std::cout << "Close the connection" << std::endl;
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

		serverSock->bind("0.0.0.0", 54000);

		serverSock->listen();
		std::cout << "Listening..." << std::endl;

		std::unique_ptr<Socket> clientSock(serverSock->accept());

		auto clientAddress = clientSock->getAddress();
		std::cout << clientAddress.host << " : " << clientAddress.port << std::endl;

		serverSock->close();

		char buf[4096];
		while (true)
		{
			memset(buf, 0, 4096);
			int bytesRecived = clientSock->receive(buf, 4096);

			if (bytesRecived == 0)
			{
				std::cout << "Dissconnected" << std::endl;
				break;
			}

			std::cout << "Recived> " << std::string(buf, 0, bytesRecived) << std::endl;

			clientSock->send(std::string(buf, 0, bytesRecived));
		}

		clientSock->shutdown(SocketShutdownType::RDWR);
		clientSock->close();
	}
	catch (const char* errorMsg)
	{
		std::cerr << errorMsg << std::endl;
		return 1;
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
