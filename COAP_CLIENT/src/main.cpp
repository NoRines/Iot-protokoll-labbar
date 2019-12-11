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
	uint8_t code = msg[1];
	uint8_t codeclass = code>>5;
	std::cout <<"CODE: "<< (int)codeclass <<'.'<< (code & 0b00010000) <<(code&0b00001111)<< std::endl;
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
	if (uri.size() <= 12) //kan använda vanlig option
	{
		uint8_t options = 0b10110000;
		options += uri.size();
		msg.push_back(options);
		for (auto l : uri)
		{
			msg.push_back((uint8_t)l);
		}
	}
	else if (uri.size() <= 255) //om uri är för lång måste extended användas
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
std::vector<uint8_t> mk_post(std::string uri) //TODO: fixa för långa uri-er
{
	uint16_t msgId = getMessageId();
	std::vector<uint8_t> msg{ 0b01010000, 0b00000010, (uint8_t)(msgId >> 8), (uint8_t)msgId};
	add_uri_to_option(msg, uri);
	return msg;
}


std::vector<uint8_t> mk_put(std::string uri,std::string payload) 
{
	uint16_t msgId = getMessageId();
	std::vector<uint8_t> msg{ 0b01010000, 0b00000011, (uint8_t)(msgId >> 8), (uint8_t)msgId};
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
	uint16_t msgId = getMessageId();
	std::vector<uint8_t> msg{ 0b01010000, 0b00000100, (uint8_t)(msgId >> 8), (uint8_t)msgId};
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
		// Create the socket
		std::unique_ptr<SocketInterface> socket = std::make_unique<Socket>(SocketType::DGRAM);

		// Set up recive buffer
		char buf[UDP_MAX_SIZE];


		Address receiveAddr = {"134.102.218.18", 5683};
		char input;

		do
		{
			memset(buf, 0, UDP_MAX_SIZE);
			std::vector<uint8_t> msg;

			std::cout << "Välj en av följande: " << std::endl;
			std::cout << "\tg för GET." << std::endl;
			std::cout << "\to för POST." << std::endl;
			std::cout << "\tu för PUT." << std::endl;
			std::cout << "\td för DELETE." << std::endl;
			std::cout << "\tq för att avsluta." << std::endl;

			std::cin >> input;

			switch(input)
			{
				case 'g':
				{
					std::cout << "Skriv uri utan snestreck: ";
					std::string uri;
					std::cin >> uri;

					msg = mk_get(uri);
				} break;

				case 'o':
				{
					std::cout << "Skriv uri utan snestreck: ";
					std::string uri;
					std::cin >> uri;

					msg = mk_post(uri);
				} break;

				case 'u':
				{
					std::cout << "Skriv uri utan snestreck: ";
					std::string uri;
					std::cin >> uri;
					std::cout << "Skriv vad du vill skicka: ";
					std::string payload;
					std::getline(std::cin, payload);
					std::getline(std::cin, payload);

					msg = mk_put(uri, payload);
				} break;

				case 'd':
				{
					std::cout << "Skriv uri utan snestreck: ";
					std::string uri;
					std::cin >> uri;

					msg = mk_delete(uri);
				} break;
			}
		
			if(input != 'q')
			{
				int bytesSent = socket->sendTo((char*)msg.data(), msg.size(), { "134.102.218.18", 5683 });
				std::cout << bytesSent << " bytes skickat" << std::endl << std::endl;


				std::cout << "Väntar på svar..." << std::endl;

				int bytesReceived = socket->receiveFrom(buf, UDP_MAX_SIZE, receiveAddr);
				std::cout << bytesReceived << " bytes mottagna" << std::endl << std::endl;

				handleResponse(buf, bytesReceived);
				std::cout << std::endl;
				std::cout << "-----------------------------------------------------" << std::endl;
				std::cout << std::endl;
			}

		} while(input != 'q');
	}
	catch(std::string error)
	{
		std::cout << error << std::endl;
		return 1;
	}
	return 0;
}
