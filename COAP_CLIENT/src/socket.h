#ifndef SOCKET_H
#define SOCKET_H

#ifdef _WIN32
#include "win_socket.h"
using Socket = WinSocket;
#endif
#ifdef __linux__
#include "unix_socket.h"
using Socket = UnixSocket;
#endif

#endif
