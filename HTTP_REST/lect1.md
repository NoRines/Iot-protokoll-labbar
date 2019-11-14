# LECT 1 HTTP/REST

## History

Tim Berners-Lee creates HTML, URI and HTTP.
First web-server created in 1991.
HTTP has been improved several times by RFC's.

## HTTP Protocol

Client/Server architecture.
Client request file from the web-server.
Uses TCP.

C -> Request -> S
S -> Requested file -> C

HTTP message structure
	hhhhhhhhhh		<- Each line-break needs \r\n.
	hhhhhhhhhh
	hhhhhhhhhh
					<- blank line seperates the header from the body.
	bbbbbbbbbb
	bbbbbbbbbb		<- Body (optional)
	bbbbbbbbbb
	bbbbbbbbbb

First line in header is the request line.
Then request headers follow.

Request line has the method type (GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE)
HEAD and GET are the only required methods that a server needs to implement.
After method type the file-path that we want is put and lastly the HTTP version.

Response to client has status line followed by the response header with the requested file in the body.
Example:
	HTTP/1.1 200 OK
	Content-Length: 35
	Connection: close
	Content-Type: text/html

	<h1>My home page</h1>

Status Codes:
	1XX - Informal
	2XX - Successful
	3XX - Redirection
	4XX - Client error
	5XX - Server error

## REST
An programming approach.
Follows the same grammar as HTTP.
	POST, GET, PUT, DELETE { XML, JSON }

Can be used together with HTTPS for security.

Cilient-server
Stateless
Cacheable
Layered system

## Tips for the lab
Start by making a server socket listening on port 80.
	* User your browser to simply trigger an incoming /GET connection
	* Use testing tools to trigger other commands

Upon receving an incoming connection
	* Read and parse the first line, take action depending on the contents
	* Return the correct response code and answer

Use simple text files to persistently save the data
	* One file per sensor/path

Use one thread for listening
	* Spawn new threads for handeling new connections

Remember split on \r\n

Most header lines can be ignored.
	* Content-Length is important from the headers
