#include <iostream>
#include <memory>
#include "win_socket.h"


#include <Ws2tcpip.h>


// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

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

		serverSocket->bind("0.0.0.0", 54000);

		serverSocket->listen();

		std::unique_ptr<Socket> clientSocket(serverSocket->accept());

		auto clientAddr = clientSocket->getAddress();
		std::cout << clientAddr.host << " : " << clientAddr.port << std::endl;

		serverSocket->shutdown(SocketShutdownType::RDWR);

		char buf[4096];

		while(true)
		{
			memset(buf, 0, 4096);
			int numBytes = clientSocket->receive(buf, 4096);

			if(numBytes == 0)
			{
				std::cout << "Client dissconnected" << std::endl;
				break;
			}

			std::cout << "Recived: " << std::string(buf, 0, numBytes) << std::endl;

			clientSocket->send(std::string(buf, 0, numBytes));
		}

		clientSocket->shutdown(SocketShutdownType::RDWR);
	} catch(const char* error)
	{
		std::cout << error << std::endl;
		return 1;
	}
#endif
	return 0;
}
