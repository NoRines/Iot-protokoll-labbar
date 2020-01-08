#include <iostream>
#include <cstdint>
#include <cstring>
#include <array>
#include <memory>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>

#include "socket/socket.h"

#include "mqtt.h"

#include "sendqueue.h"

using SocketMap = std::unordered_map<std::string, SocketInterface*>;
using TopicMap = std::unordered_map<std::string, std::vector<std::string>>;
// GLOBAL VARS__________
static volatile bool GLOBAL_RUNNING = true; // Får endast ändras i stopThread
//sockets
static std::mutex clientSocketsMutex;
static SocketMap clientSockets;

//topics
static TopicMap topicMap;
static std::mutex topicMapMutex;

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

	std::array<uint8_t, 4> rawRemaningLen;

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
				parseData.rawRemaningLen[parseData.lengthBytes] = currentByte;

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
				std::lock_guard<std::mutex> guard(clientSocketsMutex);
				clientSockets[sessionData.clientId] = clientSock.get();
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

			{
				std::lock_guard<std::mutex> guard1(topicMapMutex);
				std::lock_guard<std::mutex> guard2(clientSocketsMutex);

				auto it = topicMap.find(sessionData.topic);
				if (it != topicMap.end())
				{
					const auto& userList = topicMap[sessionData.topic];

					for (const auto& user : userList)
					{
						auto it = clientSockets.find(user);
						if (it != clientSockets.end())
						{
							std::cout << "content size: " << parseData.contents.size() << std::endl;
							std::vector<uint8_t> send_msg = { (uint8_t)((parseData.control << 4) | parseData.flags)};
							for (int i = 0; i < parseData.lengthBytes; i++)
								send_msg.push_back(parseData.rawRemaningLen[i]);
							for (int i = 0; i < parseData.contents.size(); i++)
								send_msg.push_back(parseData.contents[i]);
							pushQueue(std::make_pair(OutgoingMessage{ (char*)send_msg.data(), (int)send_msg.size() }, clientSockets[user]));
						}
							
					}
				}
			}
			

		}
		else if (sessionData.type == std::string("SUB"))
		{
			std::cout << sessionData.topic << std::endl;
			{
				// Lägger till userid till subscribat topic
				std::lock_guard<std::mutex> guard(topicMapMutex);

				auto it = topicMap.find(sessionData.topic);
				if (it == topicMap.end())
				{
					topicMap[sessionData.topic] = std::vector<std::string>();
					
				}
				auto client_it = std::find(topicMap[sessionData.topic].begin(), topicMap[sessionData.topic].end(), sessionData.clientId);
				if (client_it == std::end(topicMap[sessionData.topic]))
				{
					topicMap[sessionData.topic].push_back(sessionData.clientId);

					uint8_t lsb = sessionData.packetId;
					uint8_t subAck[] = {0x90, 0x03,(uint8_t)(sessionData.packetId >> 8), lsb, 0x00 };

					pushQueue(std::make_pair(OutgoingMessage{(char*)subAck, 5 },clientSockets[sessionData.clientId]));
				}

			}
		}
	}

	{
		// En tråd tar bort sin egen socket från socket listan
		std::lock_guard<std::mutex> guard(clientSocketsMutex);
		clientSockets.erase(clientSockets.find(sessionData.clientId));
	}

	std::cout << "End of connection" << std::endl;
	clientSock->shutdown(SocketShutdownType::RDWR);
}

void stopThread(SocketInterface* serverSock)
{
	std::cin.get();

	{
		// Gå igenom och stäng av alla client sockets
		std::lock_guard<std::mutex> guard(clientSocketsMutex);
		for(auto& it : clientSockets)
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

