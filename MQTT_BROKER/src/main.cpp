#include <iostream>
#include <cstdint>
#include <array>

#include "socket/socket.h"

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

int main(int argc, char** argv)
{
	uint8_t minMessage[] = { 0x20, 0xff, 0xff, 0xff, 0x7f };

	testMQTT(minMessage);

	return 0;
}

