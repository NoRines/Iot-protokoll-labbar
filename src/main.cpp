#include <iostream>
#include <memory>
#include "unix_socket.h"
#include <string.h>

int main(int argc, char** argv)
{
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

	return 0;
}
