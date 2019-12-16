#ifndef MQTT_H
#define MQTT_H

#include <cstdint>
#include <vector>

struct MqttSessionData
{
	std::string type;
	std::string clientId;
	uint16_t keepAlive;
};

bool handleMessage(uint8_t control, const std::vector<uint8_t>& contents, MqttSessionData& sessionData);

#endif
