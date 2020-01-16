#include <iostream>
#include <cstring>

#include "mqtt.h"

#define ERR(b, msg) \
	if(b) { \
		std::cout << msg << std::endl; \
		return false; }

static bool parseConnectMessage(const uint8_t* data, int bytes, MqttSessionData& sessionData)
{
	ERR(bytes <= 0, "Error: connection message malformed")

	// Check if the length of the protocol name is correct.
	uint16_t nameLen = (*data << 8) | *(data + 1);
	data += 2;
	bytes -= 2;

	ERR(nameLen != 4 || bytes <= 0, "Error: connection message malformed")

	// Read the name and check if it is correct.
	char protName[5] = {0};
	protName[0] = *data++;
	protName[1] = *data++;
	protName[2] = *data++;
	protName[3] = *data++;
	bytes -= 4;

	ERR(strcmp(protName, "MQTT") != 0 || bytes <= 0, "Error: connection message malformed")

	// We dont care about protocol level
	data++;
	bytes--;

	// We dont care about connect flags
	data++;
	bytes--;

	ERR(bytes <= 0, "Error: connection message malformed")

	// Read the keep alive value
	uint16_t keepAlive = (*data << 8) | *(data + 1);
	data += 2;
	bytes -= 2;

	ERR(bytes <= 0, "Error: connection message malformed")

	sessionData.keepAlive = keepAlive;

	// Read the client id length
	uint16_t clientIdLen = (*data << 8) | *(data + 1);
	data += 2;
	bytes -= 2;

	ERR(bytes <= 0, "Error: connection message malformed")

	// Read the client id
	std::string clientId;
	for(int i = 0; i < clientIdLen; i++)
	{
		clientId.push_back(*data++);
		bytes--;
	}

	sessionData.clientId = clientId;

	return true;
}

static bool parsePublishMessage(const uint8_t* data, int bytes, MqttSessionData& sessionData)
{
	// We do not care about Quality of service

	ERR(bytes <= 0, "Error: publish message malformed")

	// Get the length of the topic name
	uint16_t topicNameLen = (*data << 8) | *(data + 1);
	data += 2;
	bytes -= 2;

	ERR(bytes <= 0, "Error: publish message malformed")

	// Read the topic name
	std::string topicName;
	for(int i = 0; i < topicNameLen; i++)
	{
		topicName.push_back(*data++);
		bytes--;
	}

	ERR(bytes <= 0, "Error: publish message malformed")

	sessionData.topic = topicName;

	// Since we do not care about QoS we will not care about the packet identifier

	// Read the message
	std::string message;
	while(bytes > 0)
	{
		message.push_back(*data++);
		bytes--;
	}

	ERR(bytes < 0, "Error: publish message malformed")

	sessionData.message = message;

	return true;
}

bool parseSubscribeMessage(const uint8_t* data, int bytes, MqttSessionData& sessionData)
{
	ERR(bytes <= 0, "Error: subscribe message malformed")

	//variable header
	uint16_t packetId = (*data << 8) | *(data + 1);
	data += 2;
	bytes -= 2;

	//payload
	while (bytes > 0)
	{
		uint16_t topiclen = (*data << 8) | *(data + 1);
		data += 2;
		bytes -= 2;
		std::string topic;
		for (uint16_t i = 0; i < topiclen; i++)
			topic.push_back(*data++);
		bytes -= topiclen;
		uint8_t qos = *data++;
		bytes--;
		if (qos == 0)
		{
			sessionData.topic = topic;
			return true;
		}
	}
	return false;
}

bool parseUnsubscribeMessage(const uint8_t* data, int bytes, MqttSessionData& sessionData)
{
	ERR(bytes <= 0, "Error: unsubscribe message malformed")

	//variable header
	uint16_t packetId = (*data << 8) | *(data + 1);
	data += 2;
	bytes -= 2;

	ERR(bytes <= 0, "Error: unsubscribe message malformed")

	uint16_t topiclen = (*data << 8) | *(data + 1);
	data += 2;
	bytes -= 2;


	ERR(bytes <= 0, "Error: unsubscribe message malformed")

	std::string topic;
	for (uint16_t i = 0; i < topiclen; i++)
		topic.push_back(*data++);
	bytes -= topiclen;

	ERR(bytes < 0, "Error: unsubscribe message malformed")

	sessionData.topic = topic;
	return true;
}

bool updateMqttSession(uint8_t control, const std::vector<uint8_t>& contents, MqttSessionData& sessionData)
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
				sessionData.type = "PUB";
				return parsePublishMessage(contents.data(), contents.size(), sessionData);
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
				sessionData.type = "SUB";
				return parseSubscribeMessage(contents.data(), contents.size(), sessionData);
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
				sessionData.type = "PING";
				return true;
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

