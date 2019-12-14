#include <iostream>
#include <cstdint>
#include <cstring>
#include <array>
#include <memory>
#include <thread>
#include <vector>

#include "socket/socket.h"

enum class ParseState
{
	HEADER = 0,
	LENGTH = 1,
	OTHER = 2
};

struct MqttParseData
{
	ParseState state = ParseState::HEADER;

	uint8_t control = 0;
	uint8_t flags = 0;

	uint32_t bytesParsed = 0;

	int lengthBytes = 0;
	uint32_t messageLength = 0;

	std::vector<uint8_t> contents;
};

void printControlValue(uint8_t control)
{
	switch(control)
	{
		case 0:
			{
				std::cout << "Error: MQTT Control code 0 is forbidden." << std::endl;
			} break;
		case 1:
			{
				std::cout << "Connect request" << std::endl;
			} break;
		case 2:
			{
				std::cout << "Acknowledge connection" << std::endl;
			} break;
		case 3:
			{
				std::cout << "Publish" << std::endl;
			} break;
		case 4:
			{
				std::cout << "Acknowledge publish" << std::endl;
			} break;
		case 5:
			{
				std::cout << "Publish received" << std::endl;
			} break;
		case 6:
			{
				std::cout << "Publish released" << std::endl;
			} break;
		case 7:
			{
				std::cout << "Publish complete" << std::endl;
			} break;
		case 8:
			{
				std::cout << "Subscribe" << std::endl;
			} break;
		case 9:
			{
				std::cout << "Acknowledge subscribe" << std::endl;
			} break;
		case 10:
			{
				std::cout << "Unsubscribe" << std::endl;
			} break;
		case 11:
			{
				std::cout << "Acknowledge unsubscribe" << std::endl;
			} break;
		case 12:
			{
				std::cout << "Ping request" << std::endl;
			} break;
		case 13:
			{
				std::cout << "Ping response" << std::endl;
			} break;
		case 14:
			{
				std::cout << "Dissconnect" << std::endl;
			} break;
		case 15:
			{
				std::cout << "Error: MQTT Control code 15 is forbidden." << std::endl;
			} break;
		default:
			break;
	}
}

void parseConnectMessage(uint8_t* data, int bytes)
{
	// Check if the length of the protocol name is correct.
	uint16_t nameLen = (*data << 8) | *(data + 1);
	data += 2;
	bytes -= 2;

	if(nameLen != 4 || bytes <= 0)
	{
		std::cout << "Error: connection message malformed" << std::endl;
		return;
	}

	// Read the name and check if it is correct.
	char protName[5] = {0};
	protName[0] = *data++;
	protName[1] = *data++;
	protName[2] = *data++;
	protName[3] = *data++;
	bytes -= 4;

	if(strcmp(protName, "MQTT") != 0 || bytes <= 0)
	{
		std::cout << "Error: connection message malformed" << std::endl;
		return;
	}

	// We dont care about protocol level
	data++;
	bytes--;

	// We dont care about connect flags
	data++;
	bytes--;

	if(bytes <= 0)
	{
		std::cout << "Error: connection message malformed" << std::endl;
		return;
	}

	// Read the keep alive value
	uint16_t keepAlive = (*data << 8) | *(data + 1);
	data += 2;
	bytes -= 2;

	if(bytes <= 0)
	{
		std::cout << "Error: connection message malformed" << std::endl;
		return;
	}

	// Read the client id
	uint16_t clientIdLen = (*data << 8) | *(data + 1);
	data += 2;
	bytes -= 2;

	if(bytes <= 0)
	{
		std::cout << "Error: connection message malformed" << std::endl;
		return;
	}

	std::string clientId;
	for(int i = 0; i < clientIdLen; i++)
	{
		clientId.push_back(*data++);
		bytes--;
	}

	std::cout << "ClientId: " << clientId << std::endl;
}

bool parseByte(uint8_t currentByte, MqttParseData& parseData)
{
	switch(parseData.state)
	{
		case ParseState::HEADER:
			{
				parseData.control = currentByte >> 4;
				parseData.flags = currentByte | 0x0F;

				parseData.state = ParseState::LENGTH;
				return true;
			} break;
		case ParseState::LENGTH:
			{
				parseData.messageLength |= (currentByte & 0x7f) << (7 * parseData.lengthBytes++);

				if(currentByte == 0)
					return false;

				if((currentByte & 0x80) == 0 || parseData.lengthBytes >= 4)
					parseData.state = ParseState::OTHER;
			} break;
		case ParseState::OTHER:
			{
				parseData.bytesParsed++;
				parseData.contents.push_back(currentByte);
				if(parseData.bytesParsed >= parseData.messageLength)
					return false;
			} break;
	}

	return true;
}



void getMessage(SocketInterface* sock, char* buf, int bufSize, MqttParseData& parseData)
{
	bool continueParsing = true;
	do
	{
		memset(buf, 0, bufSize);
		int bytesParsed = 0;
		int bytesReceived = sock->receive(buf, bufSize);

		while(continueParsing)
		{
			continueParsing = parseByte(buf[bytesParsed++], parseData);
			if(bytesParsed >= bytesReceived)
				break;
		}
	} while(continueParsing);
}

void connHandler(SocketInterface* sock)
{
	std::unique_ptr<SocketInterface> clientSock(sock);

	auto clientAddress = clientSock->getAddress();

	std::cout << clientAddress.host << " : " << clientAddress.port << std::endl;

	constexpr int bufSize = 1024;
	char buf[bufSize];

	MqttParseData parseData;

	getMessage(clientSock.get(), buf, bufSize, parseData);

	printControlValue(parseData.control);
	std::cout << "Message Length: " << parseData.messageLength << std::endl;

	parseConnectMessage(parseData.contents.data(), parseData.contents.size());

	uint8_t ack[] = { 0x20, 0x02 ,0x00, 0x00 };
	clientSock->send((char*)ack, 4);

	clientSock->shutdown(SocketShutdownType::RDWR);
}

void stopThread(SocketInterface* serverSock)
{
	std::cin.get();
	serverSock->shutdown(SocketShutdownType::RD);
}

int main(int argc, char** argv)
{
	std::unique_ptr<SocketInterface> serverSock = std::make_unique<Socket>(SocketType::STREAM);

	serverSock->bind({"0.0.0.0", 1883});

	serverSock->listen();
	std::cout << "Listening..." << std::endl;

	std::thread stop(stopThread, serverSock.get());

	while(1)
	{
		try
		{
			SocketInterface* sock = serverSock->accept();
			std::thread handleConnection(connHandler, sock);
			handleConnection.detach();
		}
		catch(std::string s)
		{
			break;
		}
	}

	stop.join();

	return 0;
}

