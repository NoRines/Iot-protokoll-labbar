#include "socket/socket.h"
#include <memory>
#include <thread>
#include <iostream>
#include <string>
#include <cstring>


int main(int argc, char** argv)
{
	try
	{
		std::unique_ptr<SocketInterface> serverSocket = std::make_unique<Socket>(SocketType::DGRAM);


		char buf[4096];
		Address address = {"0.0.0.0", 54000};

		serverSocket->bind(address);

		int bytesRecived = serverSocket->receiveFrom(buf, 4096, address);
		
		std::cout << address.host << " : " << address.port << std::endl;
		std::cout << std::string(buf, 0, bytesRecived) << std::endl;

		serverSocket->shutdown(SocketShutdownType::RDWR);
	}
	catch(std::string error)
	{
		std::cout << error << std::endl;
		return 1;
	}
	return 0;
}
