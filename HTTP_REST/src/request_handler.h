#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H
#include <string>
#include <utility>
#include "request_pod.h"
#include <cstdio>
#include <fstream>
inline bool file_exists(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}


std::pair<std::string, std::string> requestHandler(RequestPod request) {
	std::string response_code = "";
	std::string headerinfo_n_body= "";
	
	if (request.action == "GET") //inte kollat om fungerar än
	{
		if (file_exists(request.path + ".txt"))
		{
			std::ifstream reader;
			reader.open(request.path + ".txt");
			std::string data;
			std::string line;
			while (reader.good())
			{
				std::getline(reader, line);
				data += line; //kan vara många rader, därav loop
			}
			reader.close();
			headerinfo_n_body = "Content-Length: " + data.size() + std::string("\r\n\r\n") + data;
			response_code = "HTTP/1.1 200 OK\r\n";
		}
		else
		{
			response_code = "HTTP/1.1 404 Not found\r\n\r\n";
		}
	}
	else if(request.action == "PUT")
	{
		if (file_exists(request.body.substr(0, request.body.find('\n')) + ".txt"))
		{
			std::ofstream writer;
			writer.open(request.body.substr(0, request.body.find('\n')) + ".txt");
			std::string fat = request.body.substr(request.body.find('\n')+1);
			writer << fat;
			response_code = "HTTP/1.1 200 OK \r\n\r\n";
			headerinfo_n_body = "";
		}
		else
		{
			response_code = "HTTP/1.1 404 Not found \r\n\r\n";
		}
	}
	else if (request.action == "POST")
	{
		if (file_exists(request.body + ".txt"))
		{
			response_code = "HTTP/1.1 409 Conflict \r\n\r\n";

		}
		else 
		{
			std::ofstream writer;
			writer.open(request.body + ".txt");
			writer.close();
			response_code = "HTTP/1.1 201 Success\r\n\r\n";
		}
	}
	else if (request.action == "DELETE")//inte kollat om fungerar än
	{
		if (file_exists(request.path + ".txt"))
		{
			std::remove((request.path + ".txt").c_str());
			response_code = "HTTP/1.1 200 OK\r\n\r\n";
		}
		else
		{
			response_code = "HTTP/1.1 404 Not found\r\n\r\n";
		}
	}
	else
	{
		throw "illegal action " + request.action + "  " + request.path + "  " + request.body;
	}

	return std::make_pair(response_code, headerinfo_n_body);
}




#endif
