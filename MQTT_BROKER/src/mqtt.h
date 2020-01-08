#ifndef MQTT_H
#define MQTT_H

#include <cstdint>
#include <vector>

struct MqttSessionData
{
	std::string type;
	std::string clientId;
	uint16_t keepAlive;

	std::string topic;
	uint16_t packetId;
	std::string message;
};

bool updateMqttSession(uint8_t control, const std::vector<uint8_t>& contents, MqttSessionData& sessionData);

#endif
