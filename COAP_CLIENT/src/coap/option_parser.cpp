#include "option_parser.h"
#include <string>
#include <iostream>

#define IT_BYTE(x) *(*x)++

namespace coap
{

OptionParser::OptionParser(const uint8_t* rawData, int numBytes)
{
	int bytesAtStart = numBytes;

	while(rawData[0] != 0xff)
	{
		int oldType = 0;
		if(!options.empty())
			oldType = options.back().type;

		options.push_back(parseOption(&rawData, numBytes));
		options.back().type = options.back().delta + oldType;
	}

	bytesInOptions = bytesAtStart - numBytes;
}

int OptionParser::getBytesInOptions() const
{
	return bytesInOptions;
}

int OptionParser::getNumOptions() const
{
	return options.size();
}

const Option& OptionParser::getOption(int n) const
{
	return options[n];
}

Option OptionParser::parseOption(const uint8_t** rawData, int& numBytes)
{
	uint8_t firstByte = IT_BYTE(rawData);
	numBytes--;

	uint16_t delta = firstByte >> 4;
	uint16_t length = firstByte & 0xf;
	std::vector<uint8_t> values;

	auto checkValue = [rawData, &numBytes](uint16_t& value)
	{
		if(value == 13)
		{
			value = IT_BYTE(rawData);
			numBytes--;
			value += 13;
		}
		else if(value == 14)
		{
			// Detta för bytesen är i network byte order, vilket är big-endian.
			value = IT_BYTE(rawData) | (IT_BYTE(rawData) << 8);
			numBytes -= 2;
			value += 269;
		}
		else if(value == 15)
		{
			throw std::string("Option contained 0xF in delta or length");
		}
	}; 

	checkValue(delta);
	checkValue(length);

	for(uint16_t i = 0; i < length; i++)
	{
		values.push_back(IT_BYTE(rawData));
		numBytes--;
	}
	return {0, delta, length, values};
}

}
