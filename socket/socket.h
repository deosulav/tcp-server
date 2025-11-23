#ifndef SOCKET_H
#define SOCKET_H

#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#if defined(_WIN32)
#define isValidSocket(s) ((s) != INVALID_SOCKET)
#define closeSocket(s) closesocket(s)
#define getSocketErrNo() (WSAGetLastError())
#else
#define INVALID_SOCKET -1
#define isValidSocket(s) ((s) >= 0)
#define closeSocket(s) close(s)
#define SOCKET int
#define getSocketErrNo() (errno)
#endif

int socketSetup();
void socketTeardown();



#endif
