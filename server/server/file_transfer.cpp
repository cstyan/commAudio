#pragma comment(lib,"Ws2_32.lib")

//#include <conio.h>
#include "file_transfer.h"
#include "wrappers.h"
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <string>

#define FILE_READ_SIZE 1024
#define PORT 5555
#define DATA_BUFSIZE 8192

using namespace std;

// This is the header that the client recieves when asking for a file download.
typedef struct {
	bool Success;
	DWORD Error;
	DWORD FileSize;
} F_TRANSFER_HEADER;

// This is used by the callback routine for file transfer.
typedef struct {
	OVERLAPPED overlapped;
	SOCKET socket;
	HANDLE hFile;
	F_TRANSFER_HEADER header;
	WSABUF wsaBuffer; 
	CHAR buffer[FILE_READ_SIZE];
	DWORD BytesSent;
	DWORD BytesInBuffer;
} SOCK_FILE_TR_INFO, *LPSOCK_FILE_TR_INFO;

// This is used by the routine for file list transfer.
typedef struct {
	OVERLAPPED overlapped;
	SOCKET socket;
	WSABUF wsaBuffer;
	std::string files_names;
} SOCK_FILE_LIST_INFO, *LPSOCK_FILE_LIST_INFO;

void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);

DWORD WINAPI WorkerThread(LPVOID lpParameter);

SOCKET AcceptSocket;

std::string list_files (void)
{
	WIN32_FIND_DATA file;
	HANDLE h;

	std::string files;
	
	h = FindFirstFile("./*.*", &file); 
	files += file.cFileName;
	files += "\n";

		while (FindNextFile(h, &file)) {

			/*
			TODO: If the function fails because no more matching files can be found, the GetLastError function returns ERROR_NO_MORE_FILES.
			*/

			if (strcmp(file.cFileName, ".") != 0 && strcmp(file.cFileName, "..") != 0) {
				files += "\n";
				printf("File: %s\n", file.cFileName);
				files += file.cFileName;
			}
			
		}
		//add socket number to string + \n

		return files;
}

void CALLBACK FileList_Routine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags) {

	DWORD NumSent;
	DWORD Flags = 0;
	LPSOCK_FILE_LIST_INFO info = (LPSOCK_FILE_LIST_INFO) Overlapped;
	
	info->wsaBuffer.buf += BytesTransferred;
	info->wsaBuffer.len -= BytesTransferred;

	if (Error != 0 || BytesTransferred == 0) {
		closesocket(info->socket);
		GlobalFree(info);
		return;
	}
	
	int result = WSASend(info->socket, &info->wsaBuffer, 1, &NumSent, Flags, &info->overlapped, FileList_Routine); 
	if (result == SOCKET_ERROR) 
	{
		if ( (result = WSAGetLastError()) != WSA_IO_PENDING ) 
		{
			//MessageBox(0, "Error with WSASend", "Error", MB_OK);
			printf("Error WSASend Error: %d\n", result);
		}
	}
}

void r_listing(LPSOCK_RECV_INFO si) {

	LPSOCK_FILE_LIST_INFO SocketInfo; 
	DWORD NumSent, Flags = 0;

	if ((SocketInfo = (LPSOCK_FILE_LIST_INFO) GlobalAlloc(GPTR,
         sizeof(SOCK_FILE_LIST_INFO))) == NULL) {
        printf("GlobalAlloc() failed with error %d\n", GetLastError());
        return;
    }

	SocketInfo->overlapped = si->overlapped;
	SocketInfo->socket = si->Socket;
	SocketInfo->files_names = list_files();
	SocketInfo->wsaBuffer.buf = (CHAR *) SocketInfo->files_names.c_str();
	SocketInfo->wsaBuffer.len = SocketInfo->files_names.size() + 1;

	int result = WSASend(SocketInfo->socket, &SocketInfo->wsaBuffer, 1, &NumSent, Flags, &SocketInfo->overlapped, FileList_Routine); 
	if (result == SOCKET_ERROR) 
	{
		if ( (result = WSAGetLastError()) != WSA_IO_PENDING ) 
		{
			//MessageBox(0, "Error with WSASend", "Error", MB_OK);
			printf("Error WSASend Error: %d\n", result);
		}
	}
}

/*---------------------------------------------

   Request Functions 

----------------------------------------------*/


/*
 
 Helper function for r_get and FileTransfer_Routine


*/

void fillBuffer(LPSOCK_FILE_TR_INFO	info, bool writeHeader = false) 
{
	DWORD amountRead;

	// resize buffer to maximum size
	info->wsaBuffer.buf = info->buffer;
	info->wsaBuffer.len = FILE_READ_SIZE;

	if (writeHeader)
	{
		//memcpy(info->buffer, &info->header, sizeof(F_TRANSFER_HEADER));

		DWORD sizeof_header = 0;
		memcpy(info->buffer, &info->header.Success, sizeof(bool));
		sizeof_header += sizeof(bool);

		memcpy(info->buffer + sizeof_header, &info->header.Error, sizeof(DWORD));
		sizeof_header += sizeof(DWORD);

		memcpy(info->buffer + sizeof_header, &info->header.FileSize, sizeof(DWORD));
		sizeof_header += sizeof(DWORD);


		info->BytesInBuffer = info->wsaBuffer.len = sizeof_header;
	}

	else if (info->header.Success)
	{
		ReadFile(info->hFile, info->wsaBuffer.buf, info->wsaBuffer.len, &(info->BytesInBuffer), NULL);
		info->wsaBuffer.len = info->BytesInBuffer;
	}
}

void CALLBACK FileTransfer_Routine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	DWORD bytesSent = 0;
	DWORD Flags = 0;
	LPSOCK_FILE_TR_INFO info = (LPSOCK_FILE_TR_INFO) Overlapped;

	info->BytesSent += BytesTransferred;
	// move buffer pointer over so effectively the buffer is shrinking
	info->wsaBuffer.buf += BytesTransferred;
	info->wsaBuffer.len -= BytesTransferred;
	info->BytesInBuffer -= BytesTransferred;

	// Check if file transfer is complete.

	DWORD sizeof_header = sizeof(bool) + 2 * sizeof(DWORD);
	//if (info->BytesSent == info->header.FileSize + sizeof(F_TRANSFER_HEADER)) 
	
	if (info->BytesSent == info->header.FileSize + sizeof_header)
	{
		closesocket(info->socket);
		
		if (info->header.Success)
			CloseHandle(info->hFile);
		
		GlobalFree(info);
		return;
	}

	if (info->BytesInBuffer == 0)
		fillBuffer(info);

	int result = WSASend(info->socket, &(info->wsaBuffer), 1, &bytesSent, Flags, Overlapped, FileTransfer_Routine);

	if (result == SOCKET_ERROR) 
	{
		if ( (result = WSAGetLastError()) != WSA_IO_PENDING ) 
		{
			//MessageBox(0, "Error with WSASend", "Error", MB_OK);
			printf("Error WSASend Error: %d\n", result);
		}
	}
}

/*

*/
bool find_ch (LPWSABUF request_data, int ch)  {

	for (size_t i=0; i < request_data->len; i++)
		if (request_data->buf[i] == ch)
			return true;

	return false;
}

void r_get(LPSOCK_RECV_INFO si)
{
	

	LPSOCK_FILE_TR_INFO SocketInfo; 

	if ((SocketInfo = (LPSOCK_FILE_TR_INFO) GlobalAlloc(GPTR,
         sizeof(SOCK_FILE_TR_INFO))) == NULL) {
        printf("GlobalAlloc() failed with error %d\n", GetLastError());
        return;
    }

	printf("Request for file %s\n", si->request_data.buf);

	SocketInfo->overlapped = si->overlapped;
	SocketInfo->socket = si->Socket;
	SocketInfo->wsaBuffer.buf = SocketInfo->buffer;
	SocketInfo->wsaBuffer.len = FILE_READ_SIZE;
	
	// For security reasons, only allow clients to access files in the current directory.
	if ( find_ch(&si->request_data, '\\') ) {

		// Simulate a CreateFile() error.
		SocketInfo->hFile = INVALID_HANDLE_VALUE;
		SetLastError(ERROR_ACCESS_DENIED);
	}
	else {
		SocketInfo->hFile = CreateFile(si->request_data.buf, GENERIC_READ, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES) NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
	}

	SocketInfo->header.Success = SocketInfo->hFile != INVALID_HANDLE_VALUE ? true : false;
	SocketInfo->header.Error =  SocketInfo->header.Success ? 0 : GetLastError();
	SocketInfo->header.FileSize = SocketInfo->header.Success ? GetFileSize(SocketInfo->hFile, NULL) : 0;

	SocketInfo->BytesInBuffer = 0;
	SocketInfo->BytesSent = 0;

	DWORD Flags = 0;
	DWORD Sent = 0;

	fillBuffer(SocketInfo, true); 
	FileTransfer_Routine(0, 0, &(SocketInfo->overlapped), Flags);

}



/*---------------------------------------------

   Completion Routines   

----------------------------------------------*/

void run_request(LPSOCK_RECV_INFO si, Request_Msg *request) {

	switch (request->command) 
	{
	case GET:
		r_get(si);
		break;

	case LISTING:
		r_listing(si);
		break;

	default:
		// error
		break;
	}

}


/* 

The protocol here is that in the request stream coming from the client:

[Header][Request Data]

[Header]:
	size_t - size of [Request Data]
	command_t - command, one of (GET or LISTING) Coming Soon: PUT

[Request Data]:
	Data to send to either GET or LISTING

*/



/*

File Transfer Protocol to client

[file size][file contents]

*/



/*

Helper function for Recv_Request to copy data between buffers:
   
   	LPSOCK_RECV_INFO->wsarecv_buffer to LPSOCK_RECV_INFO->request_data;

*/
void helper_copy_buffer(LPSOCK_RECV_INFO info, DWORD BytesTransferred)
{

	if ( !(BytesTransferred > 0) ) 
		return;
	
	if (info->request_data.len - (info->bytes_received + BytesTransferred) < 0)
	{
		MessageBox(0, "Error helper_copy_buffer(): Not enough space in buffer!", "Error", MB_OK);
		return;
	}

	memcpy((info->request_data.buf) + info->bytes_received, (info->wsarecv_buffer.buf), BytesTransferred);
	info->bytes_received += BytesTransferred;
}


/* 

Helper function for Recv_Request

*/
bool isRequestDataComplete(LPSOCK_RECV_INFO info)
{
	size_t sizeof_header = sizeof(size_t) + sizeof(command_t);
	size_t length;

	if (info->bytes_received < sizeof_header)
		return false;
	
	// Get the length from the header of the stream
	// The length is the length of the message coming from the client minus the header size
	memcpy(&length, info->request_data.buf, sizeof(size_t));

	return true; 
}

/*

Another helper function for Recv_Request

*/
Request_Msg Get_Request_Message(LPSOCK_RECV_INFO info) 
{
	
	size_t sizeof_header = sizeof(size_t) + sizeof(command_t);
	Request_Msg msg;

	if (isRequestDataComplete(info)) 
	{	
		// Get Length
		memcpy(&msg.length, info->request_data.buf, sizeof(size_t));
		
		// Get Command
		memcpy (&msg.command, info->request_data.buf + sizeof(size_t), sizeof(command_t));
		
		// Get Buffer
		msg.request_data = info->request_data;

		// Move the buffer past the header
		msg.request_data.buf += sizeof_header;
		msg.request_data.len -= msg.length;
	}

	return msg;
}


void CALLBACK Recv_Request_Routine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{

	DWORD Flags = 0;

	// TODO check Error

	// Question what if BytesTransferred == 0? Is that possible? Does it mean anything?? Do I care? Do you??

	LPSOCK_RECV_INFO info = (LPSOCK_RECV_INFO) Overlapped;

	/* Copy data transfered */
	helper_copy_buffer(info, BytesTransferred);

	if (isRequestDataComplete(info)) 
	{
		Request_Msg msg;

		msg = Get_Request_Message(info);

		info->request_data.buf = msg.request_data.buf; 
		info->request_data.len = msg.length;

		run_request(info, &msg);
	}
	else 
	{
		_WSARecv(info->Socket, &info->wsarecv_buffer, 1, &BytesTransferred, &Flags, Overlapped, Recv_Request_Routine);
	}
	
}

DWORD WINAPI FileTransferThread(LPVOID lpParameter)
{
  
   SOCKET ListenSocket;
   SOCKADDR_IN InternetAddr;
   INT Ret;
   HANDLE ThreadHandle;
   DWORD ThreadId;
   WSAEVENT AcceptEvent;

   list_files();

   if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 
      WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
   {
      printf("Failed to get a socket %d\n", WSAGetLastError());
      return 0;
   }

   InternetAddr.sin_family = AF_INET;
   InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   InternetAddr.sin_port = htons(PORT);

   if (bind(ListenSocket, (PSOCKADDR) &InternetAddr,
      sizeof(InternetAddr)) == SOCKET_ERROR)
   {
      printf("bind() failed with error %d\n", WSAGetLastError());
      return 0;
   }

   if (listen(ListenSocket, 5))
   {
      printf("listen() failed with error %d\n", WSAGetLastError());
      return 0;
   }

   if ((AcceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
   {
      printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
      return 0;
   }

   // Create a worker thread to service completed I/O requests. 

   if ((ThreadHandle = CreateThread(NULL, 0, WorkerThread, (LPVOID) AcceptEvent, 0, &ThreadId)) == NULL)
   {
      printf("CreateThread failed with error %d\n", GetLastError());
      return 0;
   }

   while(TRUE)
   {
      AcceptSocket = accept(ListenSocket, NULL, NULL);

      if (WSASetEvent(AcceptEvent) == FALSE)
      {
         printf("WSASetEvent failed with error %d\n", WSAGetLastError());
         return 0;
      }
   }
   return 0;
}

DWORD WINAPI WorkerThread(LPVOID lpParameter)
{
   DWORD Flags;
   LPSOCK_RECV_INFO SocketInfo;
   WSAEVENT EventArray[1];
   DWORD Index;
   DWORD RecvBytes = 0;

   // Save the accept event in the event array.

   EventArray[0] = (WSAEVENT) lpParameter;

   while(TRUE)
   {
      // Wait for accept() to signal an event and also process WorkerRoutine() returns.

      while(TRUE)
      {
         Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);

         if (Index == WSA_WAIT_FAILED)
         {
            printf("WSAWaitForMultipleEvents failed with error %d\n", WSAGetLastError());
            return FALSE;
         }

         if (Index != WAIT_IO_COMPLETION)
         {
            // An accept() call event is ready - break the wait loop
            break;
         } 
      }

      WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
  
	  //******** Modified from example
	
	  // Create a socket information structure to associate with the accepted socket.

      if ((SocketInfo = (LPSOCK_RECV_INFO) GlobalAlloc(GPTR,
         sizeof(SOCK_RECV_INFO))) == NULL)
      {
         printf("GlobalAlloc() failed with error %d\n", GetLastError());
         return FALSE;
      }

	  // Fill in the details of our accepted socket.
	  memset(SocketInfo, 0, sizeof(SocketInfo)); 
	  SocketInfo->Socket = AcceptSocket;

	  ZeroMemory(&(SocketInfo->overlapped), sizeof(WSAOVERLAPPED));  

	  SocketInfo->request_data.buf = SocketInfo->Buffer_request;
	  SocketInfo->request_data.len = REQUEST_DATA_BUFSIZE;
	  
	  SocketInfo->wsarecv_buffer.buf = SocketInfo->Buffer_wsarecv;
	  SocketInfo->wsarecv_buffer.len = REQUEST_DATA_BUFSIZE;

	  Flags = 0;
	  _WSARecv(SocketInfo->Socket, &(SocketInfo->wsarecv_buffer), 1, &RecvBytes, &Flags, &(SocketInfo->overlapped), Recv_Request_Routine);
	  
	   printf("Socket %d connected\n", AcceptSocket);
	  //SleepEx(INFINITE, true);  // experiment
   }

   return TRUE;
}
