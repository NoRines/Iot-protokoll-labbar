#include <iostream>
#include <cstring>

#include "mqtt.h"

static bool parseConnectMessage(const uint8_t* data, int bytes, MqttSessionData& sessionData)
{
	// Check if the length of the protocol name is correct.
	uint16_t nameLen = (*data << 8) | *(data + 1);
	data += 2;
	bytes -= 2;

	if(nameLen != 4 || bytes <= 0)
	{
		std::cout << "Error: connection message malformed" << std::endl;
		return false;
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
		return false;
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
		return false;
	}

	// Read the keep alive value
	uint16_t keepAlive = (*data << 8) | *(data + 1);
	data += 2;
	bytes -= 2;

	if(bytes <= 0)
	{
		std::cout << "Error: connection message malformed" << std::endl;
		return false;
	}
	sessionData.keepAlive = keepAlive;

	// Read the client id length
	uint16_t clientIdLen = (*data << 8) | *(data + 1);
	data += 2;
	bytes -= 2;

	if(bytes <= 0)
	{
		std::cout << "Error: connection message malformed" << std::endl;
		return false;
	}

	// Read the client id
	std::string clientId;
	for(int i = 0; i < clientIdLen; i++)
	{
		clientId.push_back(*data++);
		bytes--;
	}

	if(bytes < 0)
	{
		std::cout << "Error: connection message malformed" << std::endl;
		return false;
	}

	sessionData.clientId = clientId;

	return true;
}


bool handleMessage(uint8_t control, const std::vector<uint8_t>& contents, MqttSessionData& sessionData)
{
	switch(control)
	{
		case 0:
			{
				std::cout << "Error: MQTT Control code 0 is forbidden." << std::endl;
				return false;
			} break;
		case 1:
			{
				sessionData.type = "CON";
				return parseConnectMessage(contents.data(), contents.size(), sessionData);
			} break;
		case 2:
			{
				sessionData.type = "CONACK";
				return false;
			} break;
		case 3:
			{
				std::cout << "Publish" << std::endl;
				return false;
			} break;
		case 4:
			{
				std::cout << "Acknowledge publish" << std::endl;
				return false;
			} break;
		case 5:
			{
				std::cout << "Publish received" << std::endl;
				return false;
			} break;
		case 6:
			{
				std::cout << "Publish released" << std::endl;
				return false;
			} break;
		case 7:
			{
				std::cout << "Publish complete" << std::endl;
				return false;
			} break;
		case 8:
			{
				std::cout << "Subscribe" << std::endl;
				return false;
			} break;
		case 9:
			{
				std::cout << "Acknowledge subscribe" << std::endl;
				return false;
			} break;
		case 10:
			{
				std::cout << "Unsubscribe" << std::endl;
				return false;
			} break;
		case 11:
			{
				std::cout << "Acknowledge unsubscribe" << std::endl;
				return false;
			} break;
		case 12:
			{
				std::cout << "Ping request" << std::endl;
				return false;
			} break;
		case 13:
			{
				std::cout << "Ping response" << std::endl;
				return false;
			} break;
		case 14:
			{
				std::cout << "Dissconnect" << std::endl;
				return false;
			} break;
		case 15:
			{
				std::cout << "Error: MQTT Control code 15 is forbidden." << std::endl;
				return false;
			} break;
		default:
			break;
	}
	return false;
}

