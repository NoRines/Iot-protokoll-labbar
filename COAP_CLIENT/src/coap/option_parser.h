#ifndef OPTION_PARSER_H
#define OPTION_PARSER_H

#include <vector>

//     0   1   2   3   4   5   6   7
//   +---------------+---------------+
//   |               |               |
//   |  Option Delta | Option Length |   1 byte
//   |               |               |
//   +---------------+---------------+
//   \                               \
//   /         Option Delta          /   0-2 bytes
//   \          (extended)           \
//   +-------------------------------+
//   \                               \
//   /         Option Length         /   0-2 bytes
//   \          (extended)           \
//   +-------------------------------+
//   \                               \
//   /                               /
//   \                               \
//   /         Option Value          /   0 or more bytes
//   \                               \
//   /                               /
//   \                               \
//   +-------------------------------+

namespace coap
{

struct Option
{
	uint16_t delta;
	uint16_t length;
	std::vector<uint8_t> values;
};

class OptionParser
{
private:
	std::vector<Option> options;
};

}

#endif
