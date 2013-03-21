#pragma comment(lib,"Ws2_32.lib")

#include <conio.h>
#include <stdio.h>

#include "wrappers.h"
//#include "StopWatch.h"

#include "file_transfer.h"

// Module Name: callback.cpp
//
// Description:
//
//    This sample illustrates how to develop a simple echo server Winsock
//    application using the Overlapped I/O model with callback routines. 
//    This sample is implemented as a console-style application and simply prints
//    messages when connections are established and removed from the server.
//    The application listens for TCP connections on port 5150 and accepts them
//    as they arrive. When this application receives data from a client, it
//    simply echos (this is why we call it an echo server) the data back in
//    it's original form until the client closes the connection.
//
// Compile:
//
//    cl -o callback callback.cpp ws2_32.lib
//
// Command Line Options:
//
//    callback.exe 
//
//    Note: There are no command line options for this sample.

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#define PORT 5555
#define DATA_BUFSIZE 8192

#include <string>

using namespace std;


typedef struct _SOCKET_INFORMATION {
   OVERLAPPED Overlapped;
   SOCKET Socket;
   CHAR Buffer[DATA_BUFSIZE];
   WSABUF DataBuf;
   DWORD BytesSEND;
   DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred,
   LPWSAOVERLAPPED Overlapped, DWORD InFlags);

DWORD WINAPI WorkerThread(LPVOID lpParameter);

SOCKET AcceptSocket;


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
	  

#if 0
      // Create a socket information structure to associate with the accepted socket.

      if ((SocketInfo = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR,
         sizeof(SOCKET_INFORMATION))) == NULL)
      {
         printf("GlobalAlloc() failed with error %d\n", GetLastError());
         return FALSE;
      } 

      // Fill in the details of our accepted socket.

      SocketInfo->Socket = AcceptSocket;
      ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));  
      SocketInfo->BytesSEND = 0;
      SocketInfo->BytesRECV = 0;
      SocketInfo->DataBuf.len = DATA_BUFSIZE;
      SocketInfo->DataBuf.buf = SocketInfo->Buffer;

      Flags = 0;
      if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
         &(SocketInfo->Overlapped), WorkerRoutine) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSA_IO_PENDING)
         {
            printf("WSARecv() failed with error %d\n", WSAGetLastError());
            return FALSE;
         }
      }
#endif

      printf("Socket %d connected\n", AcceptSocket);
	  //SleepEx(INFINITE, true);  // experiment
   }

   return TRUE;
}


void CALLBACK Rsv_WorkerRoutine(DWORD Error, DWORD BytesTransferred,
   LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{






}

void CALLBACK Send_WorkerRoutine(DWORD Error, DWORD BytesTransferred,
   LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
   DWORD SendBytes;
   DWORD Flags;

   // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
   LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

   if (Error != 0)
   {
     printf("I/O operation failed with error %d\n", Error);
   }

   if (BytesTransferred == 0)
   {
      printf("Closing socket %d\n", SI->Socket);
   }

   if (Error != 0 || BytesTransferred == 0)
   {
      closesocket(SI->Socket);
      GlobalFree(SI);
      return;
   }



}

void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred,
   LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
   DWORD SendBytes, RecvBytes;
   DWORD Flags;

   // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
   LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

   if (Error != 0)
   {
     printf("I/O operation failed with error %d\n", Error);
   }

   if (BytesTransferred == 0)
   {
      printf("Closing socket %d\n", SI->Socket);
   }

   if (Error != 0 || BytesTransferred == 0)
   {
      closesocket(SI->Socket);
      GlobalFree(SI);
      return;
   }

   // Check to see if the BytesRECV field equals zero. If this is so, then
   // this means a WSARecv call just completed so update the BytesRECV field
   // with the BytesTransferred value from the completed WSARecv() call.

   if (SI->BytesRECV == 0)
   {
      SI->BytesRECV = BytesTransferred;
      SI->BytesSEND = 0;
   }
   else
   {
      SI->BytesSEND += BytesTransferred;
   }

   if (SI->BytesRECV > SI->BytesSEND)
   {
	   
      // Post another WSASend() request.
      // Since WSASend() is not gauranteed to send all of the bytes requested,
      // continue posting WSASend() calls until all received bytes are sent.

      ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

      SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
      SI->DataBuf.len = SI->BytesRECV - SI->BytesSEND;

	  
	  string files = list_files();
	  char databuf[1025];

	  WSABUF DataBuf;

	  DataBuf.buf = databuf;
	  DataBuf.len =  files.size() + 1;

	  memcpy(DataBuf.buf, files.c_str(), files.length());
	  DataBuf.buf[DataBuf.len-1] = '\0';

  
     if (WSASend(SI->Socket, &DataBuf, 1, &SendBytes, 0,
         &(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSA_IO_PENDING)
         {
            printf("WSASend() failed with error %d\n", WSAGetLastError());
            return;
         }
      }
   }
   else
   {
      SI->BytesRECV = 0;

      // Now that there are no more bytes to send post another WSARecv() request.

      Flags = 0;
      ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

      SI->DataBuf.len = DATA_BUFSIZE;
      SI->DataBuf.buf = SI->Buffer;

      if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
         &(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSA_IO_PENDING )
         {
            printf("WSARecv() failed with error %d\n", WSAGetLastError());
            return;
         }
      }
   }
}




#if 0

#define PORT 7777


#define PLAYBACK_BUFFER_SIZE   	327680L
#define DATABLOCK_SIZE          	32768L
#define MAX_BUFFERS                	 2  // double buffer
#define MSG_LEN                   		128

static WAVEHDR* wh[MAX_BUFFERS];

// temp buffer for this example
//.............................

static VOID*    pPlaybackBuffer;  
static DWORD    nPlaybackBufferPos = 0L;  // position in playback buffer  
static DWORD    nPlaybackBufferLen = 0L;  // total data in playback buffer

VOID AllocBuffers ()
{
   int i;

   // allocate two WAVEHDR buffer blocks
	//...................................

	for (i = 0; i < MAX_BUFFERS; i++)                                        
	{                                                               
	   wh[i] = (WAVEHDR *) HeapAlloc( GetProcessHeap(),                         
	                      HEAP_ZERO_MEMORY,                         
	                      sizeof(WAVEHDR) );                        
	   if (wh[i])                                                  
	   {                                                            
	       wh[i]->lpData = (LPSTR) HeapAlloc( GetProcessHeap(),             
	                                  HEAP_ZERO_MEMORY,             
	                                  DATABLOCK_SIZE);                       
	       wh[i]->dwBufferLength = DATABLOCK_SIZE;
	   }
	}

   // allocate playback buffer - enough space to hold
   // ten data buffer blocks of waveform sound data
   //................................................

   pPlaybackBuffer = HeapAlloc( GetProcessHeap(),
                                HEAP_ZERO_MEMORY,
                                PLAYBACK_BUFFER_SIZE);
} 

/*****************************************************************************
*  CleanUpBuffers
*
*
*
******************************************************************************/

VOID CleanUpBuffers()
{
   int i;

   // free the WAVEHDR buffer blocks
	//...............................

 	for (i = 0; i < MAX_BUFFERS; i++)                                        
	{
     if (wh[i] != NULL)
     {
	      HeapFree(GetProcessHeap(), 0, wh[i]->lpData);                 
	      HeapFree(GetProcessHeap(), 0, wh[i]);
         wh[i] = NULL;
     }
	}
	
	// free playback buffer
	//.....................
	
	HeapFree(GetProcessHeap(), 0, pPlaybackBuffer);
}

int main() {

	SOCKET socket;
	WSADATA WSAData;
	WORD wVersionRequested = MAKEWORD(2,2);
	SOCKADDR_IN InternetAddr;
	HANDLE ThreadHandle;
    DWORD ThreadId;
    WSAEVENT AcceptEvent;
	
	
	_WSAStartup( wVersionRequested, &WSAData );
	socket = _WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

   InternetAddr.sin_family = AF_INET;
   InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   InternetAddr.sin_port = htons(PORT);

   _bind(socket, (PSOCKADDR) &InternetAddr, sizeof(InternetAddr));
   _listen(socket, 5);

   AcceptEvent = _WSACreateEvent();

	StopWatch stop_watch;
	
	char ch = 0;
	
	while (1)  { 
		stop_watch.Start();
		ch = _getch();

		if (ch == 27) {
			printf("\nGoodbye\n");
			Sleep(1000);
			break;
		}

		stop_watch.Stop();
		printf("%c %d ", ch, stop_watch.GetInteval(1, 2));
	}


	WSACleanup();
}

#endif 
