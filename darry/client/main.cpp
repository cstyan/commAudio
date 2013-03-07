#define FILENAME "no_such_file"


/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		tcp_clnt.c - A simple TCP client program.
--
--	PROGRAM:			tclnt.exe
--
--	FUNCTIONS:			Winsock 2 API
--
--	DATE:				January 11, 2006
--
--	REVISIONS:			(Date and Description)
--
--						Oct. 1, 2007 (A. Abdulla):
--						
--						Changed the read loop to better handle the 
--						blocking recv call. 
--
--	DESIGNERS:			Aman Abdulla
--
--	PROGRAMMERS:		Aman Abdulla
--
--	NOTES:
--	The program will establish a TCP connection to a user specifed server.
--  The server can be specified using a fully qualified domain name or and
--	IP address. After the connection has been established the user will be
--  prompted for date. The date string is then sent to the server and the
--  response (echo) back from the server is displayed.
---------------------------------------------------------------------------------------*/

#pragma comment(lib,"Ws2_32.lib")

#include <stdio.h>
#include <winsock2.h>
#include <errno.h>
#include <conio.h>

#include "wrappers.h"

//#include <string.h>
//#include <memory.h>

#define SERVER_TCP_PORT			7000	// Default port
#define BUFSIZE					99999	// Buffer length

typedef unsigned char command_t;

typedef struct {
	size_t size;
	command_t command;
} REQUEST_MSG;

typedef struct {
	OVERLAPPED overlapped;
	SOCKET socket;
	WSABUF wsaBuf;
	REQUEST_MSG msg;
	CHAR BufferData[BUFSIZE];
	DWORD totalBytesSent;
	DWORD totalBytesToSend;
} SOCK_REQUEST_INFO, *LPSOCK_REQUEST_INFO;


typedef struct {
	bool Success;
	DWORD Error;
	DWORD FileSize;
} F_TRANSFER_HEADER;


typedef struct {
	OVERLAPPED overlapped;
	SOCKET socket;
	char *buffer[BUFSIZE];
	WSABUF wsaBuffer;
	DWORD TotalBytesReceived;
	F_TRANSFER_HEADER header;
	char *filename;
	HANDLE fHandle;
} SOCK_FILE_DOWNLOAD_INFO, *LPSOCK_FILE_DOWNLOAD_INFO;


void Cleanup_FileDownload(LPSOCK_FILE_DOWNLOAD_INFO info) 
{
	closesocket(info->socket);
	if (info->fHandle != INVALID_HANDLE_VALUE)
		CloseHandle(info->fHandle);

	GlobalFree(info);
}







// This function downloads and writes that file
void CALLBACK helper_DownLoadFile_Routine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	
	DWORD Flags = 0;
	DWORD BytesSent;

	WSABUF tempBuff;

	if (Error != 0) { }

	if (BytesTransferred == 0) { }

	LPSOCK_FILE_DOWNLOAD_INFO info = (LPSOCK_FILE_DOWNLOAD_INFO) Overlapped;

	info->TotalBytesReceived += BytesTransferred;
	
	// Write buffer to file 
	tempBuff.buf = info->wsaBuffer.buf;
	tempBuff.len = BytesTransferred;
	/*
	do {
		_WriteFile(info->fHandle, tempBuff.buf, tempBuff.len, &BytesWritten, NULL);
		tempBuff.buf += BytesWritten;
		tempBuff.len -= BytesWritten;
	} while (BytesWritten != 0);
	*/


	if (info->TotalBytesReceived == info->header.FileSize) {
		Cleanup_FileDownload(info);
		return;
	} else {

		_WSARecv(info->socket, &info->wsaBuffer, 1, &BytesSent, &Flags, Overlapped, helper_DownLoadFile_Routine);
	}
}

void CALLBACK DownLoadFile_Routine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{

	MessageBox(0, "inside", "inside", 0);

	DWORD NumSent, Flags = 0;

	if (Error != 0) {}
	if (BytesTransferred == 0) {}

	LPSOCK_FILE_DOWNLOAD_INFO info = (LPSOCK_FILE_DOWNLOAD_INFO) Overlapped;

	info->TotalBytesReceived += BytesTransferred;
	info->wsaBuffer.buf += BytesTransferred;
	info->wsaBuffer.len -= BytesTransferred;

	if (info->TotalBytesReceived < sizeof(F_TRANSFER_HEADER)) {
		int result = WSARecv(info->socket, &info->wsaBuffer, 1, &NumSent, &Flags, Overlapped, DownLoadFile_Routine);
		// TODO: Error Checking.. Actually I have a  wrapper function..called _WSARecv()
	}
	else {
		memcpy(&info->header, info->buffer, sizeof(F_TRANSFER_HEADER));
		info->TotalBytesReceived = 0;
		info->wsaBuffer.buf = (CHAR *) info->buffer;
		info->wsaBuffer.len = BUFSIZE;

		if (info->header.Success) {
			// Ok now download the file
			_WSARecv(info->socket, &info->wsaBuffer, 1, &NumSent, &Flags, Overlapped, helper_DownLoadFile_Routine);
			
		}
		else {
			char errorStr[256];
			sprintf(errorStr, "Error downloading file %s error: %d", info->filename, info->header.Error);
			MessageBox(0, errorStr, "Error", MB_OK);

			Cleanup_FileDownload(info);
			return;
		}
	}
}

SOCKET ConnectToServer(char *host, int port)
{

	SOCKET sd;
	int err;
	struct hostent	*hp;
	struct sockaddr_in server;
	char  *bp, rbuf[BUFSIZE], sbuf[BUFSIZE], **pptr;

	if ((sd = _WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 
      WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
   {
      return -1;
   }

	// Initialize and set up the address structure
	memset((char *)&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if ((hp = gethostbyname(host)) == NULL)
	{
		MessageBox(0, "Unknown server address", "Error", MB_OK);
		return -1;
	}

	// Copy the server address
	memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

	// Connecting to the server
	if (connect (sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		MessageBox(0, "Can't connect to server.", "Error", MB_OK);
		return -1;
	}

	return sd;
}




void CALLBACK SendRequest_Routine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{

	printf("SendRequest_Routine()\n");

	DWORD NumSent, Flags = 0;

	if (Error != 0) {}
	if (BytesTransferred == 0) {}

	LPSOCK_REQUEST_INFO info = (LPSOCK_REQUEST_INFO) Overlapped;

	info->totalBytesSent += BytesTransferred;

	if (info->totalBytesSent == info->totalBytesToSend) {
		GlobalFree(info);
		return;
	}

	info->wsaBuf.buf += BytesTransferred;
	info->wsaBuf.len -= BytesTransferred;

	WSASend(info->socket, &info->wsaBuf, 1, &NumSent, Flags, &info->overlapped, SendRequest_Routine);
	SleepEx(INFINITE, true);
}


void SendRequest(SOCKET sd, OVERLAPPED overlapped, size_t size_data, command_t command, CHAR *data) {

	DWORD NumSend, Flags = 0;

	LPSOCK_REQUEST_INFO SocketInfo;

	if ((SocketInfo = (LPSOCK_REQUEST_INFO) GlobalAlloc(GPTR,
         sizeof(SOCK_REQUEST_INFO))) == NULL) {

		char errorStr[100];
        sprintf(errorStr, "GlobalAlloc() failed with error %d\n", GetLastError());
		MessageBox(0, errorStr, "Error", MB_OK);
        return;
    }

	SocketInfo->socket = sd;
	SocketInfo->overlapped = overlapped;
	
	SocketInfo->msg.size = size_data;
	SocketInfo->msg.command = command; 
	memcpy(SocketInfo->BufferData, &SocketInfo->msg, sizeof(SocketInfo->msg));
	strcpy(SocketInfo->BufferData + 5, data);  // 5 is the sizeof(int) + sizeof(unsigned char)

	printf("sizeof(unsigned char) %d\n", sizeof(unsigned char));
	printf("sizeof(int) %d\n", sizeof(int));
	printf("sizeof(REQUEST_MSG) %d\n", sizeof(REQUEST_MSG));


	SocketInfo->wsaBuf.buf = SocketInfo->BufferData;
	SocketInfo->wsaBuf.len = sizeof(int) + sizeof(command_t) + size_data;

	printf("BufferData: \n");
	
	for (int i=0; i<SocketInfo->wsaBuf.len; i++)
		printf("%x ", SocketInfo->wsaBuf.buf[i]);
	printf("\n");
	for (int i=0; i<SocketInfo->wsaBuf.len; i++)
		printf("%c ", SocketInfo->wsaBuf.buf[i]);
	printf("\n");
	

	// DEBUGGING

	/*
	SocketInfo->wsaBuf.buf[0] = 2;
	SocketInfo->wsaBuf.buf[1] = 0;
	SocketInfo->wsaBuf.buf[2] = 0;
	SocketInfo->wsaBuf.buf[3] = 0;
	SocketInfo->wsaBuf.buf[4] = 1; 
	SocketInfo->wsaBuf.buf[5] = 'Z';
	SocketInfo->wsaBuf.buf[6] = 0;
	
	SocketInfo->wsaBuf.len = 7;
	

	//


	SocketInfo->totalBytesSent = 0;
	SocketInfo->totalBytesToSend = SocketInfo->wsaBuf.len;
	*/

	//memset(&SocketInfo->overlapped, 0, sizeof(OVERLAPPED));
	SleepEx(0, true);
	int result = WSASend(SocketInfo->socket, &SocketInfo->wsaBuf, 1, &NumSend, Flags, &SocketInfo->overlapped, SendRequest_Routine);
	//SleepEx(INFINITE, true);

	if (result == 0 && NumSend == SocketInfo->wsaBuf.len) {
		//

	}
	else if (NumSend != SocketInfo->wsaBuf.len) {
		// 	Bad!

	}
	
	printf("NumSend: %d\n", NumSend);


	printf("Result %d\n", result);

}

void DownloadFile(char *destination, char *source, char *host, int port) {

	DWORD Sent, Flags = 0;

	LPSOCK_FILE_DOWNLOAD_INFO SocketInfo; 


	if ((SocketInfo = (LPSOCK_FILE_DOWNLOAD_INFO) GlobalAlloc(GPTR,
         sizeof(SOCK_FILE_DOWNLOAD_INFO))) == NULL) {

		char errorStr[100];
        sprintf(errorStr, "GlobalAlloc() failed with error %d\n", GetLastError());
		MessageBox(0, errorStr, "Error", MB_OK);
        return;
    }

	SocketInfo->filename = source;

	if ((SocketInfo->socket = ConnectToServer(host, port)) == -1) {
		// ConnectToServer displays MessageBox if there's an error

		Cleanup_FileDownload(SocketInfo);
		return;
	}
	

	/*
	SocketInfo->fHandle = CreateFile(destination, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL,(HANDLE) NULL);

	if (SocketInfo->fHandle == INVALID_HANDLE_VALUE) {

		char errorStr[100];
		sprintf(errorStr, "Error creating file. Error: %d", GetLastError());
		MessageBox(0, errorStr, "Error", MB_OK);
	
		Cleanup_FileDownload(SocketInfo);
		return;
	}
	*/

	
#if 0
		//-----------Test-----------------------
	char sbuf[1024];
	unsigned char command = 1;
	size_t length = strlen(FILENAME);
	
		memcpy(sbuf, &length, sizeof(size_t));
		memcpy(sbuf + sizeof(size_t), &command, 1);

		memcpy (&sbuf[5], FILENAME, strlen(FILENAME) + 1);


		printf("Sending %x %x %x %x %x %s \n", sbuf[0], sbuf[1], sbuf[2], sbuf[3], sbuf[4], &sbuf[5]);

		//send (SocketInfo->socket, sbuf, 5 + 1 + strlen(FILENAME), 0);
		
	//--------------------------------------
#endif

	memset(&SocketInfo->overlapped, 0, sizeof(OVERLAPPED));
	SendRequest(SocketInfo->socket, SocketInfo->overlapped, strlen("bacon") + 1, 1, "bacon"); 
	
	
	int result = _WSARecv(SocketInfo->socket, &SocketInfo->wsaBuffer, 1, &Sent, &Flags, &SocketInfo->overlapped, DownLoadFile_Routine);

	printf("result of WSARecv() %d\n", result);
	
}

DWORD WINAPI ThreadFunc(LPVOID n)
{
    
	WORD wVersionRequested;
	WSADATA WSAData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );
	err = WSAStartup( wVersionRequested, &WSAData );
	if ( err != 0 ) //No usable DLL
	{
		printf ("DLL not found!\n");
		exit(1);
	}	
	
	DownloadFile("downloaded_test.txt4", "test.txt", "localhost", 5555);
	
	printf("Press <ESC> to exit\n");
	while (_getch() != 27)
		Sleep(100);
	
	WSACleanup();

	//exit(0);
    return 0;
}


int main() {

	HANDLE hThrd;
    DWORD threadId;
    hThrd = CreateThread(NULL, 0, ThreadFunc, NULL, 0, &threadId );


	while (1)
		;
}



int nosooldmain() 
{
	WORD wVersionRequested;
	WSADATA WSAData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );
	err = WSAStartup( wVersionRequested, &WSAData );
	if ( err != 0 ) //No usable DLL
	{
		printf ("DLL not found!\n");
		exit(1);
	}	
	
	DownloadFile("downloaded_test.txt4", "test.txt", "localhost", 5555);
	
	printf("Press <ESC> to exit\n");
	while (_getch() != 27)
		Sleep(100);
	
	WSACleanup();


	//exit(0);
}

int oldmain (int argc, char **argv)
{
	int n, ns, bytes_to_read;
	int port, err;
	SOCKET sd;
	struct hostent	*hp;
	struct sockaddr_in server;
	char  *host, *bp, rbuf[BUFSIZE], sbuf[BUFSIZE], **pptr;
	WSADATA WSAData;
	WORD wVersionRequested;

	switch(argc)
	{
		case 2:
			host =	argv[1];	// Host name
			port =	SERVER_TCP_PORT;
		break;
		case 3:
			host =	argv[1];
			port =	atoi(argv[2]);	// User specified port
		break;
		default:
			host = "localhost";
			port = 5555;
			break;

	}

	wVersionRequested = MAKEWORD( 2, 2 );
	err = WSAStartup( wVersionRequested, &WSAData );
	if ( err != 0 ) //No usable DLL
	{
		printf ("DLL not found!\n");
		exit(1);
	}

	// Create the socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Cannot create socket");
		exit(1);
	}

	// Initialize and set up the address structure
	memset((char *)&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if ((hp = gethostbyname(host)) == NULL)
	{
		fprintf(stderr, "Unknown server address\n");
		exit(1);
	}

	// Copy the server address
	memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

	// Connecting to the server
	if (connect (sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		fprintf(stderr, "Can't connect to server\n");
		perror("connect");
		exit(1);
	}

	sockaddr saddr;

	//WSAConnect(sd, 


	printf("Connected:    Server Name: %s\n", hp->h_name);
	pptr = hp->h_addr_list;
	printf("\t\tIP Address: %s\n", inet_ntoa(server.sin_addr));

	//for (int i=0; i<3; i++) {

	unsigned char command = 1;
	size_t length = strlen(FILENAME) + 1;

		printf("Transmiting:\n");
		//memset((char *)sbuf, 0, sizeof(sbuf));
		//gets(sbuf); // get user's text

		// Transmit data through the socket

		memcpy(sbuf, &length, sizeof(size_t));
		memcpy(sbuf + sizeof(size_t), &command, 1);

		memcpy (&sbuf[5], FILENAME, strlen(FILENAME) + 1);


		printf("Sending %x %x %x %x %x %s \n", sbuf[0], sbuf[1], sbuf[2], sbuf[3], sbuf[4], &sbuf[5]);

		ns = send (sd, sbuf, 5 + 1 + strlen(FILENAME), 0);

		printf("ns == %d\n", ns);

		printf("Receive:\n");
		
		F_TRANSFER_HEADER header;


		bytes_to_read = sizeof(header);
		
		
		bp = (char *) &(header);

		// Get Header
		while ((n = recv (sd, bp, bytes_to_read, 0)) < BUFSIZE)
		{
			bp += n;
			bytes_to_read -= n;
			if (n == 0)
				break;
		}
		
		printf("\n\n");
		printf("Success: %s\n", header.Success ? "Yes" : "No");
		if (!header.Success)
			printf("Error code: %d\n", header.Error);
		printf("File size %d\n", header.FileSize );
		
		//-------------Read client-----------------------//

		bp = rbuf;
		memset(bp, 0, BUFSIZE);

		// client makes repeated calls to recv until no more data is expected to arrive.

		bytes_to_read = header.FileSize;

		size_t bytes_read = 0;

		while ((n = recv (sd, bp, bytes_to_read, 0)) < BUFSIZE)
		{

			bp += n;
			bytes_to_read -= n;
			if (n == 0 || bytes_to_read ==0)
				break;

			
		}
		printf ("%s\n", rbuf);
		printf ("Transfer complete.\n");
	//}

	closesocket (sd);
	WSACleanup();

	_getch();
	exit(0);
}
