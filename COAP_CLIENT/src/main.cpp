#include "socket/socket.h"
#include <memory>
#include <thread>
#include <iostream>
#include <string>
#include <cstring>

#include <cstdint>

#include "coap/coap_parser.h"
#include "coap/option_parser.h"

void handleResponse(const char* msg, int size)
{
	coap::Parser coapMessage((uint8_t*)msg, size);

	int numOptions = coapMessage.getNumOptions();

	for(int i = 0; i < numOptions; i++)
	{
		const auto& op = coapMessage.getOption(i);

		if(op.type == 12)
		{
			std::cout << "Content-Format option received" << std::endl;
			std::cout << "Bytes in option: " << op.length << std::endl;

			uint8_t coapValue = op.values[0];
			switch(coapValue)
			{
				case 0:
					std::cout << "Data is text/plain" << std::endl;
					break;
				case 40:
					std::cout << "application/link-format" << std::endl;
					break;
				case 41:
					std::cout << "application/xml" << std::endl;
					break;
				case 42:
					std::cout << "application/octet-stream" << std::endl;
					break;
				case 47:
					std::cout << "application/exi" << std::endl;
					break;
				case 50:
					std::cout << "application/json" << std::endl;
					break;
				default:
					break;
			}
		}
	}

	const auto& payload = coapMessage.getPayload();

	std::cout << "Bytes in payload: " << payload.size() << std::endl;
	std::cout << std::string((char*)payload.data(), payload.size()) << std::endl;
}

int main(int argc, char** argv)
{
	try
	{
		// Test server is coap.me the which has ip-address: 134.102.218.18


		// Create the socket
		std::unique_ptr<SocketInterface> socket = std::make_unique<Socket>(SocketType::DGRAM);

		// Very simple GET message
		uint8_t simpleGet[] = { 0b01011000, 0b00000001, 0b10101010, 0b10101010,
			200,
			201,
			202,
			203,
			204,
			205,
			206,
			207};

		// Send the simple get to coap.me
		std::cout << "Sending GET..." << std::endl;
		int bytesSent = socket->sendTo((char*)simpleGet, 12, {"134.102.218.18", 5683});
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

		handleResponse(buf, bytesReceived);
	}
	catch(std::string error)
	{
		std::cout << error << std::endl;
		return 1;
	}
	return 0;
}
