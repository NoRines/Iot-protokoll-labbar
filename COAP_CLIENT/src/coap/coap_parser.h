#ifndef COAP_PARSER_H
#define COAP_PARSER_H

#include <cstdint>
#include <array>
#include "option_parser.h"

//    0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |Ver| T |  TKL  |      Code     |          Message ID           |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |   Token (if any, TKL bytes) ...
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |   Options (if any) ...
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |1 1 1 1 1 1 1 1|    Payload (if any) ...
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


namespace coap
{

struct Header
{
	uint8_t verTypeTokenLen;
	uint8_t code;
	uint16_t msgId;
};

struct Token
{
	std::array<uint8_t, 8> bytes;
	int numBytes;
};

class Parser
{
public:
	Parser(const uint8_t* rawData, int numBytes);

private:
	Header parseHeader(const uint8_t** rawData, int& numBytes);
	Token parseToken(const uint8_t** rawData, int& numBytes);
	OptionParser parseOptions(const uint8_t** rawData, int& numBytes);

public:
	uint8_t getVersion() const;
	uint8_t getMessageType() const;
	uint8_t getTokenLength() const;
	uint8_t getMessageCode() const;
	uint16_t getMessageId() const;

	Token getToken() const;

	int getNumOptions() const;

private:
	Header header;
	Token token;
	OptionParser options;
};

}

#endif
