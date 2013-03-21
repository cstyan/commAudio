#ifndef _FILE_TRANSFER_H_
#define _FILE_TRANSFER_H_

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>   
#include <string>

#define GET 1
#define LISTING 2

#define REQUEST_DATA_BUFSIZE 8192

typedef unsigned char command_t;

/*---------------------------------------------
 
 Msg struct for send and recv functions.

----------------------------------------------*/
typedef struct {
	size_t length;
	command_t command;
	WSABUF request_data; 
} Request_Msg;


typedef struct {
	OVERLAPPED overlapped;
	SOCKET Socket;
	WSABUF wsarecv_buffer;
	WSABUF request_data;
	size_t bytes_received;
	CHAR Buffer_wsarecv[REQUEST_DATA_BUFSIZE];
	CHAR Buffer_request[REQUEST_DATA_BUFSIZE];
} SOCK_RECV_INFO, *LPSOCK_RECV_INFO;

std::string list_files (void);

void CALLBACK Recv_Request_Routine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);

/*---------------------------------------------

  Request functions.

----------------------------------------------*/
void (request_func)(LPSOCK_RECV_INFO si, LPWSABUF data);

void r_get(LPSOCK_RECV_INFO si, LPWSABUF request_data);
void r_listing(LPSOCK_RECV_INFO si, LPWSABUF request_data);

DWORD WINAPI FileTransferThread(LPVOID lpParameter);

#endif