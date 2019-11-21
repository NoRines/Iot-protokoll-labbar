#include "socket/socket.h"
#include <memory>
#include <thread>
#include <iostream>
#include <string>
#include <cstring>

void handleConnection(SocketInterface* sock)
{
	std::unique_ptr<SocketInterface> clientSocket(sock);

	auto address = clientSocket->getAddress();
	std::cout << address.host << " : " << address.port << std::endl;

	char buf[4096];

	memset(buf, 0, 4096);
	int bytesRecived = clientSocket->receive(buf, 4096);

	if (bytesRecived == 0)
	{
		std::cout << "Disconnected" << std::endl;
		clientSocket->shutdown(SocketShutdownType::RDWR);
		clientSocket->close();
		return;
	}

	std::cout << std::string(buf, 0, bytesRecived) << std::endl;

	clientSocket->send("Hejsansvejsan");

	clientSocket->shutdown(SocketShutdownType::RDWR);
	clientSocket->close();
}

int main(int argc, char** argv)
{
	try
	{
		std::unique_ptr<SocketInterface> serverSocket = std::make_unique<Socket>(SocketType::STREAM);

		serverSocket->bind({"0.0.0.0", 80});
		serverSocket->listen();

		for(int i = 0; i < 5; i++)
		{
			auto sock = serverSocket->accept();
			std::thread handlerThread(handleConnection, sock);
			handlerThread.detach();
		}

		serverSocket->shutdown(SocketShutdownType::RDWR);
		serverSocket->close();
	}
	catch(std::string error)
	{
		std::cout << error << std::endl;
		return 1;
	}
	return 0;
}
