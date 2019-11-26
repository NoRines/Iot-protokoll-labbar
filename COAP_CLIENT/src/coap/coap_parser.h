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

struct CoapHeader
{
	uint8_t verTypeTokenLen;
	uint8_t code;
	uint16_t msgId;
};

struct CoapToken
{
	std::array<uint8_t, 8> bytes;
	int numBytes;
};

class CoapParser
{
private:
	CoapHeader header;
	CoapToken token;
	OptionParser options;
};

#endif
