#include <iostream>
#include <cstdint>
#include <cstring>
#include <array>
#include <memory>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <algorithm>

#include "socket/socket.h"

#include "mqtt.h"

#include "sendqueue.h"


using SocketMap = std::unordered_map<std::string, SocketInterface*>;

using UserList = std::vector<std::string>;
using TopicMap = std::unordered_map<std::string, UserList>;

// GLOBAL VARS__________
static volatile bool GLOBAL_RUNNING = true; // Får endast ändras i stopThread

static std::mutex socketMapMutex;
static SocketMap socketMap;

static std::mutex topicMapMutex;
static TopicMap topicMap;

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



bool getMessage(SocketInterface* sock, char* buf, int bufSize, MqttParseData& parseData)
{
	bool continueParsing = true;
	do
	{
		memset(buf, 0, bufSize);
		int bytesParsed = 0;
		int bytesReceived = sock->receive(buf, bufSize);

		if(bytesReceived == 0)
			return false;

		while(continueParsing)
		{
			continueParsing = parseByte(buf[bytesParsed++], parseData);
			if(bytesParsed >= bytesReceived)
				break;
		}
	} while(continueParsing);
	return true;
}

void connHandler(SocketInterface* sock)
{
	std::unique_ptr<SocketInterface> clientSock(sock);

	auto clientAddress = clientSock->getAddress();

	std::cout << clientAddress.host << " : " << clientAddress.port << std::endl;

	bool connected = false;
	constexpr int bufSize = 1024;
	char buf[bufSize];

	MqttParseData parseData;
	MqttSessionData sessionData;

	clientSock->setTimeout(5);

	while(GLOBAL_RUNNING)
	{
		try
		{
			parseData = MqttParseData();
			if(!getMessage(clientSock.get(), buf, bufSize, parseData))
				break;
		}
		catch(...)
		{
			std::cout << "Timeout on receive, closing connection." << std::endl;
			break;
		}

		std::cout << "Message Length: " << parseData.messageLength << std::endl;
		if(!updateMqttSession(parseData.control, parseData.contents, sessionData))
			break;

		std::cout << sessionData.type << " : " << sessionData.clientId << " : " << sessionData.keepAlive << std::endl;

		// TEMP
		if(sessionData.type == std::string("CON") && connected == false)
		{
			connected = true;

			{
				// En tråd lägger till sig själv i socket listan
				std::lock_guard<std::mutex> guard(socketMapMutex);
				socketMap[sessionData.clientId] = clientSock.get();
			}

			clientSock->setTimeout(sessionData.keepAlive);
			uint8_t ack[] = { 0x20, 0x02 ,0x00, 0x00 };
			pushQueue(std::make_pair(OutgoingMessage{ (char*)ack, 4 }, clientSock.get()));
		}
		else if(sessionData.type == std::string("PING"))
		{
			uint8_t pong[] = { 0xd0, 0x00 };
			pushQueue(std::make_pair(OutgoingMessage{ (char*)pong, 2 }, clientSock.get()));
		}
		else if(sessionData.type == std::string("PUB"))
		{
			std::cout << sessionData.topic << " : " << sessionData.message << std::endl;
		}
		else if(sessionData.type == std::string("SUB"))
		{
			std::lock_guard<std::mutex> guard(topicMapMutex);

			/* Create user list if topic exists */ {
				auto it = topicMap.find(sessionData.topic);
				if(it == topicMap.end())
					topicMap[sessionData.topic] = UserList();
			}

			// Get user list
			auto& userList = topicMap[sessionData.topic];

			/* Check for id in user list */ {
				auto it = std::find(userList.begin(), userList.end(), sessionData.clientId);
				if(it == userList.end())
					userList.push_back(sessionData.clientId);
			}
		}
	}

	{
		// En tråd tar bort sin egen socket från socket listan
		std::lock_guard<std::mutex> guard(socketMapMutex);
		socketMap.erase(socketMap.find(sessionData.clientId));
	}

	std::cout << "End of connection" << std::endl;
	clientSock->shutdown(SocketShutdownType::RDWR);
}

void stopThread(SocketInterface* serverSock)
{
	std::cin.get();

	{
		// Gå igenom och stäng av alla client sockets
		std::lock_guard<std::mutex> guard(socketMapMutex);
		for(auto& it : socketMap)
			it.second->shutdown(SocketShutdownType::RD);
	}

	serverSock->shutdown(SocketShutdownType::RD);
	GLOBAL_RUNNING = false;
	interruptQueue();
}

void sendThread()
{
	while(GLOBAL_RUNNING)
	{
		try
		{
			auto msg = popQueue();
			msg.second->send(msg.first.data, msg.first.length);
		}
		catch(...)
		{
		}
	}
}

int main(int argc, char** argv)
{
	std::unique_ptr<SocketInterface> serverSock = std::make_unique<Socket>(SocketType::STREAM);

	serverSock->bind({"0.0.0.0", 1883});

	serverSock->listen();
	std::cout << "Listening..." << std::endl;

	std::thread stop(stopThread, serverSock.get());
	std::thread send(sendThread);

	while(GLOBAL_RUNNING)
	{
		try
		{
			SocketInterface* sock = serverSock->accept();
			std::thread handleConnection(connHandler, sock);
			handleConnection.detach();
		}
		catch(...)
		{
			break;
		}
	}

	stop.join();
	send.join();

	return 0;
}

