#include "coap_parser.h"
#include <bitset>
#include <iostream>

#define IT_BYTE(x) *(*x)++

namespace coap
{

Parser::Parser(const uint8_t* rawData, int numBytes) :
	header(parseHeader(&rawData, numBytes)),
	token(parseToken(&rawData, numBytes)),
	options(parseOptions(&rawData, numBytes)),
	payload(parsePayload(&rawData, numBytes))
{
}

Header Parser::parseHeader(const uint8_t** rawData, int& numBytes)
{
	Header h = {0};

	h.verTypeTokenLen = IT_BYTE(rawData);
	h.code = IT_BYTE(rawData);
	h.msgId = IT_BYTE(rawData) << 8;
	h.msgId |= IT_BYTE(rawData);

	numBytes -= 4;

	return h;
}

Token Parser::parseToken(const uint8_t** rawData, int& numBytes)
{
	Token t = {0};

	t.numBytes = getTokenLength();

	if(numBytes == 0 || t.numBytes == 0)
		return {0};

	if(t.numBytes > 8)
		return {0}; // Nåt är fel... TODO(markus): Lägg till error message.

	for(uint8_t i = 0; i < t.numBytes; i++)
	{
		t.bytes[i] = IT_BYTE(rawData);
		numBytes--;
	}

	return t;
}

OptionParser Parser::parseOptions(const uint8_t** rawData, int& numBytes)
{
	OptionParser o(*rawData, numBytes);

	int bytesInOptions = o.getBytesInOptions();
	*rawData = (*rawData) + bytesInOptions;
	numBytes -= bytesInOptions;

	return o;
}

std::vector<uint8_t> Parser::parsePayload(const uint8_t** rawData, int& numBytes)
{
	std::vector<uint8_t> res;
	res.reserve(numBytes);

	while(numBytes > 0)
	{
		uint8_t b = IT_BYTE(rawData);
		if(b != 0xFF)
			res.push_back(b);
		numBytes--;
	}

	return res;
}

uint8_t Parser::getVersion() const
{
	return header.verTypeTokenLen & 0b11000000;
}

uint8_t Parser::getMessageType() const
{
	return header.verTypeTokenLen & 0b00110000;
}

uint8_t Parser::getTokenLength() const
{
	return header.verTypeTokenLen & 0b00001111;
}

uint8_t Parser::getMessageCode() const
{
	return header.code;
}

uint16_t Parser::getMessageId() const
{
	return header.msgId;
}

Token Parser::getToken() const
{
	return token;
}

int Parser::getNumOptions() const
{
	return options.getNumOptions();
}

const Option& Parser::getOption(int n) const
{
	return options.getOption(n);
}

const std::vector<uint8_t>& Parser::getPayload() const
{
	return payload;
}

}
