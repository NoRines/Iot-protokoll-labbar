#ifndef OPTION_PARSER_H
#define OPTION_PARSER_H

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

class OptionParser
{
};

#endif
