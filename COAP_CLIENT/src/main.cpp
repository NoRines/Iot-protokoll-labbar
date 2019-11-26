#include "socket/socket.h"
#include <memory>
#include <thread>
#include <iostream>
#include <string>
#include <cstring>

#include <cstdint>

#include "coap/coap_parser.h"

int main(int argc, char** argv)
{
	try
	{
		// Test server is coap.me the which has ip-address: 134.102.218.18


		// Create the socket
		std::unique_ptr<SocketInterface> socket = std::make_unique<Socket>(SocketType::DGRAM);

		// Very simple GET message
		uint8_t simpleGet[] = { 0b01010011, 0b00000001, 0b10101010, 0b10101010,
			200,
			201,
			202};

		// Send the simple get to coap.me
		std::cout << "Sending GET..." << std::endl;
		int bytesSent = socket->sendTo((char*)simpleGet, 7, {"134.102.218.18", 5683});
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

		coap::Parser test((uint8_t*)buf, bytesReceived);

		std::cout << (int)test.getTokenLength() << std::endl;

		auto t = test.getToken();
		for(int i = 0; i < t.numBytes; i++)
			std::cout << (int)t.bytes[i] << std::endl;
	}
	catch(std::string error)
	{
		std::cout << error << std::endl;
		return 1;
	}
	return 0;
}
