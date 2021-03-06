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
	
	if (request.action == "GET") //inte kollat om fungerar �n
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
				data += line; //kan vara m�nga rader, d�rav loop
			}
			reader.close();
			headerinfo_n_body = "Content-Length: " + std::to_string(data.size()) + std::string("\r\n\r\n") + data;
			response_code = "HTTP/1.1 200 OK\r\n";
		}
		else
		{
			response_code = "HTTP/1.1 404 Not found\r\n\r\n";
		}
	}
	else if(request.action == "PUT")
	{
		std::string fileName = request.path+".txt";
		std::cout << "FILE: " << fileName << std::endl;
		if (file_exists(fileName))
		{
			std::ofstream writer;
			writer.open(request.path + ".txt");
			std::string content = request.body;
			if (!writer.is_open()) {
				std::cout << "file exists but cant open " <<content<< std::endl;
				response_code = "HTTP/1.1 500 Internal Server Error \r\n\r\n";
			}
			else{
			writer << content;
			response_code = "HTTP/1.1 200 OK \r\n\r\n";
			writer.close();
			headerinfo_n_body = "";}
		}
		else
		{
			response_code = "HTTP/1.1 404 Not found \r\n\r\n";
		}
	}
	else if (request.action == "POST")
	{
		if (file_exists(request.path+ ".txt"))
		{
			response_code = "HTTP/1.1 409 Conflict \r\n\r\n";

		}
		else 
		{
			std::ofstream writer;
			//std::cout << request.body << std::endl << request.body.length() << std::endl;
			writer.open(request.path + ".txt");
			writer.close();
			response_code = "HTTP/1.1 201 Success\r\n\r\n";
		}
	}
	else if (request.action == "DELETE")//inte kollat om fungerar �n
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
