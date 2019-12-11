#include <iostream>
#include <cstdint>
#include <cstring>
#include <array>
#include <memory>
#include <thread>

#include "socket/socket.h"

enum class ParseState
{
	HEADER = 0,
	LENGTH = 1,
	VAR_HEADER = 2,
	PAYLOAD = 3
};

struct MqttParseData
{
	ParseState state = ParseState::HEADER;

	uint8_t control = 0;
	uint8_t flags = 0;

	uint32_t bytesParsed = 0;

	int lengthBytes = 0;
	uint32_t messageLength = 0;
};

bool testTestMqtt(uint8_t currentByte, MqttParseData& parseData)
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
					parseData.state = ParseState::VAR_HEADER;
			} break;
		case ParseState::VAR_HEADER:
			{
				parseData.bytesParsed++;
				if(parseData.bytesParsed >= parseData.messageLength)
					return false;
			} break;
		case ParseState::PAYLOAD:
			{
				parseData.bytesParsed++;
				if(parseData.bytesParsed >= parseData.messageLength)
					return false;
			} break;
	}

	return true;
}

void testMQTT(uint8_t* msg) // TODO(markus): Refactor this maybe to a class or something.
{
	// Save byte [1]
	uint8_t control = (*msg) >> 4;
	uint8_t flags = *msg++ | 0x0F;

	int bytesInLength = 0;
	std::array<uint8_t, 4> lengthBytes;


	// Save the variable length bytes [2, 3(op), 4(op), 5(op)]
	lengthBytes[bytesInLength++] = *msg++;
	while(bytesInLength < 4 && (lengthBytes[bytesInLength-1] & 0x80) != 0)
		lengthBytes[bytesInLength++] = *msg++;

	// Convert the length bytes to a value
	uint32_t length = 0;
	for(int i = 0; i < bytesInLength; i++)
		length |= (lengthBytes[i] & 0x7f) << (7*i);

	// Switch for all possible values of the control bits
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

	std::cout << std::endl << "Bytes in variable length: ";
	for(int i = 0; i < bytesInLength; i++)
	{
		std::cout << (int)lengthBytes[i] << " ";
	}
	std::cout << std::endl;
	std::cout << bytesInLength << " bytes in length" << std::endl;

	std::cout << std::endl << "Length value: " << length << std::endl;
}

void connHandler(SocketInterface* sock)
{
	std::unique_ptr<SocketInterface> clientSock(sock);

	auto clientAddress = clientSock->getAddress();

	std::cout << clientAddress.host << " : " << clientAddress.port << std::endl;

	constexpr int bufLen = 1024;
	char buf[bufLen];

	int bytesParsed = 0;
	MqttParseData parseData;

	bool continueParsing = true;
	do
	{
		memset(buf, 0, bufLen);
		int bytesReceived = clientSock->receive(buf, bufLen);

		while(continueParsing)
		{
			continueParsing = testTestMqtt(buf[bytesParsed++], parseData);
		}
	} while(continueParsing);


	std::cout << "Message Type: " << (int)parseData.control << std::endl;
	std::cout << "Message Length: " << parseData.messageLength << std::endl;

	clientSock->shutdown(SocketShutdownType::RDWR);
}

int main(int argc, char** argv)
{
	//uint8_t test[] = { 0x20, 0x80, 0x80, 0x80, 0x01 };
	//MqttParseData parseData;

	//for(int i = 0; i < 5; i++)
	//	std::cout << testTestMqtt(test[i], parseData) << std::endl;

	//std::cout << parseData.messageLength << std::endl;

	std::unique_ptr<SocketInterface> serverSock = std::make_unique<Socket>(SocketType::STREAM);

	serverSock->bind({"0.0.0.0", 1883});

	serverSock->listen();
	std::cout << "Listening..." << std::endl;

	while(1)
	{
		SocketInterface* sock = serverSock->accept();
		std::thread handleConnection(connHandler, sock);
		handleConnection.detach();
	}

	serverSock->shutdown(SocketShutdownType::RDWR);

	return 0;
}

