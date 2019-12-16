#ifndef SOCKET_EXCEPTIONS_H
#define SOCKET_EXCEPTIONS_H

#include <exception>

class SocketException : public std::exception
{
	std::string message;

public:
	SocketException(const std::string& msg) : message(msg) {}

	const char* what() const throw() override
	{
		std::string ret = std::string("SocketException") + message;
		return ret.c_str();
	}
};

class SocketTimeoutException : public std::exception
{
	std::string message;

public:
	TimeoutException(const std::string& msg) : message(msg) {}

	const char* what() const throw() override
    {
		std::string ret = std::string("TimeoutException ") + message;
		return ret.c_str();
    }
};

#endif
