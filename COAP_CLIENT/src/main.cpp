#include "socket/socket.h"
#include <memory>
#include <thread>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <ctime>

#include "coap/coap_parser.h"
#include "coap/option_parser.h"

static uint16_t gMessageId = 0;

uint16_t getMessageId()
{
	return gMessageId++;
}

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
			if(op.length != 0)
			{ 
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
	}

	const auto& payload = coapMessage.getPayload();

	std::cout << "Bytes in payload: " << payload.size() << std::endl;
	std::cout << std::string((char*)payload.data(), payload.size()) << std::endl;
}
void add_uri_to_option(std::vector<uint8_t>&msg, std::string uri)
{
	if (uri.size() <= 12) //kan anv�nda vanlig option
	{
		uint8_t options = 0b10110000;
		options += uri.size();
		msg.push_back(options);
		for (auto l : uri)
		{
			msg.push_back((uint8_t)l);
		}
	}
	else if (uri.size() <= 255) //om uri �r f�r l�ng m�ste extended anv�ndas
	{
		uint8_t options = 0b10110000;
		options += 13;
		msg.push_back(options);
		uint8_t extended_option_length = uri.size() - 13;
		msg.push_back(extended_option_length);
		for (auto l : uri)
		{
			msg.push_back((uint8_t)l);
		}
	}
	else if (uri.size() > 255)
	{
		uint8_t options = 0b10110000;
		options += 14;
		msg.push_back(options);
		uint16_t extended_option_length = uri.size() - 269;
		uint8_t small_end = extended_option_length;
		uint8_t big_end = extended_option_length >> 8;
		msg.push_back(options);
		msg.push_back(small_end);
		msg.push_back(big_end);
		for (auto l : uri)
		{
			msg.push_back((uint8_t)l);
		}
	}
}
std::vector<uint8_t> mk_post(std::string uri) //TODO: fixa f�r l�nga uri-er
{
	std::vector<uint8_t> msg{ 0b01010100, 0b00000010, 0b10101010, 0b10101010, 
							  0b10101010, 0b10101010, 0b10101010, 0b10101010 };
	add_uri_to_option(msg, uri);
	return msg;
}


std::vector<uint8_t> mk_put(std::string uri,std::string payload) 
{
	std::vector<uint8_t> msg{ 0b01010000, 0b00000011, 0b10101010, 0b10101010};
	add_uri_to_option(msg, uri);
	uint8_t option_content_format = 0b00010000;
	msg.push_back(option_content_format);
	msg.push_back(0xff);
	for (auto p : payload)
	{
		msg.push_back((uint8_t)p);
	}
	return msg;
}

std::vector<uint8_t> mk_delete(std::string uri)
{
	std::vector<uint8_t> msg{ 0b01010000, 0b00000100, 0b10101010, 0b10101010 };
	add_uri_to_option(msg, uri);
	return msg;
}

std::vector<uint8_t> mk_get(std::string uri)
{
	uint16_t msgId = getMessageId();
	std::vector<uint8_t> msg{0b01010000, 0b00000001, (uint8_t)(msgId >> 8), (uint8_t)msgId};

	add_uri_to_option(msg, uri);

	return msg;
}

int main(int argc, char** argv)
{
	constexpr int UDP_MAX_SIZE = 65507;

	std::srand(std::time(NULL)); // Seeda rand
	gMessageId = std::rand();

	try
	{
		// Test server is coap.me the which has ip-address: 134.102.218.18


		// Create the socket
		std::unique_ptr<SocketInterface> socket = std::make_unique<Socket>(SocketType::DGRAM);

		std::string uri = "sink";
		std::vector<uint8_t> post_msg = mk_post(uri);

		//std::vector<uint8_t> put_msg = mk_put(uri,"big hello");
		std::vector<uint8_t> delete_msg = mk_delete(uri);
		// Send the simple get to coap.me
		std::cout << "Sending request..." << std::endl;
		//int bytesSent = socket->sendTo((char*)simpleGet, 12, {"134.102.218.18", 5683});
		int bytesSent = socket->sendTo((char*)delete_msg.data(), delete_msg.size(), { "134.102.218.18", 5683 });

		std::cout << bytesSent << " bytes sent" << std::endl << std::endl;

		// Set up recive buffer
		char buf[UDP_MAX_SIZE];
		memset(buf, 0, UDP_MAX_SIZE);

		// Wait for the response
		std::cout << "Waiting for response..." << std::endl;

		Address receiveAddr = {"134.102.218.18", 5683};
		int bytesReceived = socket->receiveFrom(buf, UDP_MAX_SIZE, receiveAddr);
		std::cout << bytesReceived << " bytes received" << std::endl << std::endl;

		handleResponse(buf, bytesReceived);
	}
	catch(std::string error)
	{
		std::cout << error << std::endl;
		return 1;
	}
	return 0;
}
