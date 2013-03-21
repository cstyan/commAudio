#ifndef _WRAPPERS_H_
#define _WRAPPERS_H_



#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>   
#include <string>


SOCKET _WSASocket( int af, int type, int protocol, LPWSAPROTOCOL_INFO lpProtocolInfo, GROUP g, DWORD dwFlags);

int _WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);

int _bind(SOCKET s, const struct sockaddr *name, int namelen);

int _listen(SOCKET s, int backlog);

WSAEVENT _WSACreateEvent(void);

int _WSARecv(SOCKET S, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

#endif