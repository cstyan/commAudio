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

#pragma comment(lib,"Ws2_32.lib")

#include <conio.h>
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#define PORT 5555
#define DATA_BUFSIZE 8192

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

SOCKET GlobalSocket;

void begin(void)
{
   WSADATA wsaData;
   //SOCKET Socket;
   SOCKADDR_IN InternetAddr;
   INT Ret;
   HANDLE ThreadHandle;
   DWORD ThreadId;
   WSAEVENT Event;


   if ((Ret = WSAStartup(0x0202,&wsaData)) != 0)
   {
      printf("WSAStartup failed with error %d\n", Ret);
      WSACleanup();
      return;
   }

   if ((GlobalSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 
      WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
   {
      printf("Failed to get a socket %d\n", WSAGetLastError());
      return;
   }

   InternetAddr.sin_family = AF_INET;
   //InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   InternetAddr.sin_port = htons(PORT);

   char *host = "localhost";
   struct hostent	*hp;

   if ((hp = gethostbyname(host)) == NULL)
	{
		printf("Unknown server address");
		return;
	}

	// Copy the server address
	memcpy((char *)&InternetAddr.sin_addr, hp->h_addr, hp->h_length);

   /*
   if (bind(GlobalSocket, (PSOCKADDR) &InternetAddr,
      sizeof(InternetAddr)) == SOCKET_ERROR)
   {
      printf("bind() failed with error %d\n", WSAGetLastError());
      return;
   }
   */

   /*
   if (listen(ListenSocket, 5))
   {
      printf("listen() failed with error %d\n", WSAGetLastError());
      return;
   }
   */
   
   if ((Event = WSACreateEvent()) == WSA_INVALID_EVENT)
   {
      printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
      return;
   }

   // Create a worker thread to service completed I/O requests. 

   if ((ThreadHandle = CreateThread(NULL, 0, WorkerThread, (LPVOID) Event, 0, &ThreadId)) == NULL)
   {
      printf("CreateThread failed with error %d\n", GetLastError());
      return;
   }

  // while(TRUE)
   //{
     // AcceptSocket = accept(Socket, NULL, NULL);

	//int result = connect(GlobalSocket, 
	 
   if (connect (GlobalSocket, (struct sockaddr *)&InternetAddr, sizeof(InternetAddr)) == -1)
	{
		printf("Can't connect to server.");
		return;
	}

      if (WSASetEvent(Event) == FALSE)
      {
         printf("WSASetEvent failed with error %d\n", WSAGetLastError());
         return;
      }
   //}
}

int main() {

	begin();
	_getch();
}

DWORD WINAPI WorkerThread(LPVOID lpParameter)
{
   DWORD Flags;
   LPSOCKET_INFORMATION SocketInfo;
   WSAEVENT EventArray[1];
   DWORD Index;
   DWORD RecvBytes;

   // Save the accept event in the event array.

   EventArray[0] = (WSAEVENT) lpParameter;

   //while(TRUE)
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
   
      // Create a socket information structure to associate with the accepted socket.

      if ((SocketInfo = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR,
         sizeof(SOCKET_INFORMATION))) == NULL)
      {
         printf("GlobalAlloc() failed with error %d\n", GetLastError());
         return FALSE;
      } 

      // Fill in the details of our accepted socket.

      SocketInfo->Socket = GlobalSocket;
      ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));  
      SocketInfo->BytesSEND = 0;
      SocketInfo->BytesRECV = 0;
      

	  SocketInfo->Buffer[0] = 3;
	  SocketInfo->Buffer[1] = 0;
	  SocketInfo->Buffer[2] = 0;
	  SocketInfo->Buffer[3] = 0;
	  SocketInfo->Buffer[4] = 1;
	  SocketInfo->Buffer[5] = 'h';
	  SocketInfo->Buffer[6] = 'i';
	  SocketInfo->Buffer[7] = 0;
	  
	  SocketInfo->DataBuf.len = 8;
      SocketInfo->DataBuf.buf = SocketInfo->Buffer;
	  

      Flags = 0;
	  int result;

      if (result = WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, Flags, //RecvBytes misnamed
         &(SocketInfo->Overlapped), WorkerRoutine) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSA_IO_PENDING)
         {
            printf("WSASend() failed with error %d\n", WSAGetLastError());
            return FALSE;
         }
      }

	  SleepEx(INFINITE, true);	  

      printf("Socket %d connected\n", GlobalSocket);

	  while(TRUE)
		  ;

   }

   return TRUE;
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
   
   SI->BytesSEND += BytesTransferred;

   if (Error != 0 || BytesTransferred == 0 || SI->BytesSEND == 8)
   {
      closesocket(SI->Socket);
      GlobalFree(SI);
      return;
   }

   SI->DataBuf.buf += BytesTransferred;
   SI->DataBuf.len -= BytesTransferred;


   WSASend(SI->Socket, &SI->DataBuf, 1, &SendBytes, Flags, &SI->Overlapped, WorkerRoutine);
}