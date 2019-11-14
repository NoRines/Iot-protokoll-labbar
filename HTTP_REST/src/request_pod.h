#ifndef REQUEST_POD_H
#define REQUEST_POD_H
#include <string>


struct RequestPod 
{
	std::string action = "";
	int content_length = 0;
	std::string body = "";
};

RequestPod populate(std::string input) {
	RequestPod res;
	std::string act = input.substr(0, input.find(" "));
	res.action = act;
	res.content_length = 0;
	//std::cout << "act " << act << std::endl;
	size_t content_len_position = input.find("Content-Length:");
	if (content_len_position != std::string::npos)
	{
		std::cout << "contentlenpos " << content_len_position << std::endl;
		input = input.substr(content_len_position);
		//std::cout << "cut imp " << input << std::endl;
		std::string cl = input.substr(input.find(":")+1, input.find("\r\n") - (input.find(":")+1));
		cl = cl.substr(1,cl.size()-1);

		res.content_length = std::stoi(cl);
	}
	size_t body_start = input.find("\r\n\r\n");
	if (body_start != std::string::npos)
	{
		std::string body = input.substr(body_start+4);
		res.body = body;
		//std::cout << "bod " << body << std::endl;
	}
	std::cout << res.action << "  len:" << res.content_length << "  body:" << res.body << std::endl;
	return res;
}


#endif