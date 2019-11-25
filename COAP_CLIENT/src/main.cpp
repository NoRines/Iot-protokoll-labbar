#include "socket/socket.h"
#include <memory>
#include <thread>
#include <iostream>
#include <string>
#include <cstring>

#include <cstdint>

int main(int argc, char** argv)
{
	try
	{
		// Test server is coap.me the which has ip-address: 134.102.218.18


		// Create the socket
		std::unique_ptr<SocketInterface> socket = std::make_unique<Socket>(SocketType::DGRAM);

		// Very simple GET message
		uint8_t simpleGet[] = { 0b01010000, 0b00000001, 0b10101010, 0b10101010 };

		// Send the simple get to coap.me
		std::cout << "Sending GET..." << std::endl;
		int bytesSent = socket->sendTo((char*)simpleGet, 4, {"134.102.218.18", 5683});
		std::cout << bytesSent << " bytes sent" << std::endl << std::endl;

		// Wait for the response
		std::cout << "Waiting for response..." << std::endl;
		char buf[4096];
		memset(buf, 0, 4096);
		Address receiveAddr = {"134.102.218.18", 5683};
		int bytesReceived = socket->receiveFrom(buf, 4096, receiveAddr);
		std::cout << bytesReceived << " bytes received" << std::endl << std::endl;

		std::cout << "Raw message: " << std::endl;
		std::cout << std::string(buf, 0, bytesReceived) << std::endl;
	}
	catch(std::string error)
	{
		std::cout << error << std::endl;
		return 1;
	}
	return 0;
}
