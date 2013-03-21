#define FILENAME "no_such_file"

#define NUM_EVENTS 4
#define SEND_REQUEST 0
#define RECEIVE_HEADER 1
#define FILE_TRANSFER 2
#define FILE_TRANFER_THREAD_DONE 3

char *lpEventNames[] = {"send_request_done", "just_received_header", "file_transfer_complete", "tread_done"};

#pragma comment(lib,"Ws2_32.lib")

#include <winsock2.h>
#include <stdio.h>
#include <errno.h>
#include <conio.h>
#include <windows.h>
#include <string>

#include "client.h"
#include "AddConsole.h"
#include "wrappers.h"
#include "resource.h"


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
	HANDLE eventDone;
} SOCK_REQUEST_INFO, *LPSOCK_REQUEST_INFO;


typedef struct {
	bool Success;
	DWORD Error;
	DWORD FileSize;
} F_TRANSFER_HEADER;


typedef struct {
	OVERLAPPED overlapped;
	SOCKET socket;
	char buffer[BUFSIZE];
	WSABUF wsaBuffer;
	DWORD TotalBytesReceived;
	DWORD TotalBytesToSend;
	F_TRANSFER_HEADER header;
	char *filename;
	HANDLE fHandle;
} SOCK_FILE_DOWNLOAD_INFO, *LPSOCK_FILE_DOWNLOAD_INFO;

#define CONSOLE_ENABLED 1 // Used to enable or disable the extra console window. 1 - Enabled, 0 - Disabled

int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                       LPSTR lpszCmdLine, int nCmdShow)
{
    HWND        hWnd ;      /* the window's "handle" */
    MSG         msg ;       /* a message structure */
    WNDCLASS    wndclass ;  /* window class structure */

        wndclass.style          = CS_HREDRAW | CS_VREDRAW ;
        wndclass.lpfnWndProc    = WndProc ;
        wndclass.cbClsExtra     = 0 ;
        wndclass.cbWndExtra     = 0 ;
        wndclass.hInstance      = hInstance ;
        wndclass.hIcon          = NULL ;
        wndclass.hCursor        = LoadCursor (NULL, IDC_ARROW) ;
        wndclass.hbrBackground  = (HBRUSH)GetStockObject (WHITE_BRUSH) ;
        wndclass.lpszMenuName   = MAKEINTRESOURCE(IDR_MENU1);
        wndclass.lpszClassName  = TEXT("MyClass");
        if (!RegisterClass (&wndclass))
            return 0 ;

    hWnd = CreateWindow (TEXT("MyClass"), TEXT("Comm Audio Client"), 
        WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400, 
        NULL, NULL, hInstance, NULL) ;
	
    ShowWindow (hWnd, nCmdShow) ;    
    UpdateWindow(hWnd) ;

    while (GetMessage (&msg, NULL, 0, 0)) 
    {
        TranslateMessage (&msg) ; 
        DispatchMessage (&msg) ;  
    }
    return (msg.wParam) ;
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT wMessage, 
                           WPARAM wParam, LPARAM lParam)
{
    static  HWND    hPlayBtn, hPauseBtn, hSongDisplay;
    HINSTANCE       hInstance ;
    BOOL            bIsChecked, bTopRadioOn ; /* button status variables */
    char            cBuf [128] ;
	std::string			sBuf;
    
    switch (wMessage)       /* process windows messages */
    {
        case WM_CREATE:     /* program is just starting */
			/* get the instance handle */
            #if(CONSOLE_ENABLED)
				// Start console.
				AddConsole::addConsole();
			#endif
			printf("hi");
			hInstance = (HINSTANCE)GetWindowLong (hWnd, GWL_HINSTANCE) ; 

			/* create button controls */
            hPlayBtn = CreateWindow (TEXT("BUTTON"), TEXT("Play"),
                    WS_CHILD | BS_DEFPUSHBUTTON,
                    140, 250, 100, 40,
                    hWnd, (HMENU)PLAY_BUTTON, hInstance, NULL) ;

			hPauseBtn = CreateWindow(TEXT("BUTTON"), TEXT("Pause"),
					WS_CHILD | BS_DEFPUSHBUTTON,
					260, 250, 100, 40,
					hWnd, (HMENU)PAUSE_BUTTON, hInstance, NULL) ;

			hSongDisplay = CreateWindow(TEXT("LISTBOX"), TEXT("Song List"),
					WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | WS_CAPTION,
					30, 30, 425, 200,
					hWnd, (HMENU)SONG_DISPLAY, hInstance, NULL);
			SendMessage(hSongDisplay, EM_SETREADONLY, TRUE, NULL);

			/* display the buttons */
            ShowWindow (hPlayBtn, SW_SHOWNORMAL) ;
            ShowWindow (hPauseBtn, SW_SHOWNORMAL) ;
            ShowWindow (hSongDisplay, SW_SHOWNORMAL) ;
			
			SendMessage(hSongDisplay, LB_ADDSTRING, 0, (LPARAM(TEXT("hi"))));
			//connect to the server
			//retrieve the playlist from the server
			//load that playlist into an arraylist of strings?
			//loadPlayList(hSongDisplay, ...);
            break ;

        case WM_COMMAND:
            switch (LOWORD(wParam)) /* check Button control ID */
            {
                case PLAY_BUTTON:
					SendMessage(hSongDisplay, LB_ADDSTRING, 0, (LPARAM(TEXT("hi"))));
                    break ; 

				case PAUSE_BUTTON:
					SendMessage(hSongDisplay, LB_ADDSTRING, 0, (LPARAM(TEXT("bye"))));
					break ;

				case ID_MIC:
					break;

				case ID_FILE_EXIT:
                    DestroyWindow (hWnd) ;  /* destroy window */
                    break ;     /* terminating application */
            }
            break ;

        case WM_DESTROY:    /* stop application */
            PostQuitMessage (0) ;
            break ;

        default:        /* default windows message processing */
            return DefWindowProc (hWnd, wMessage, wParam, lParam) ;
    }
    return (0) ;
}

// This a convience function.
// Pre-condition: events is an array of NUM_EVENT HANDLES
bool CreateFileTransferEvents(HANDLE *events) {

	memset(events, 0, sizeof(HANDLE) * NUM_EVENTS);
	for (size_t i=0; i< NUM_EVENTS; i++) 
		if ((events[i] = CreateEvent(NULL, false, false, lpEventNames[i])) == NULL)
			return false;
	
	return true;
}

void CloseEvents(HANDLE *events) {

	for (size_t i=0; i <NUM_EVENTS; i++)
		CloseHandle(events[i]);

}

void Cleanup_FileDownload(LPSOCK_FILE_DOWNLOAD_INFO info) 
{
	HANDLE events[NUM_EVENTS];
	CreateFileTransferEvents(events);

	closesocket(info->socket);
	if (info->fHandle != INVALID_HANDLE_VALUE)
		CloseHandle(info->fHandle);

	GlobalFree(info);

	SetEvent(events[FILE_TRANSFER]);

	// Experiment 
	//WSACleanup();
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

	for (int i = 0; i<BytesTransferred; i++) {
		printf("%c", info->wsaBuffer.buf[i]);
		fflush(stdin);
	}

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


// Download the header
void CALLBACK DownLoadFile_Routine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{

	DWORD NumSent, Flags = 0;

	if (Error != 0) {}
	if (BytesTransferred == 0) {}

	LPSOCK_FILE_DOWNLOAD_INFO info = (LPSOCK_FILE_DOWNLOAD_INFO) Overlapped;	

	printf("header\n");
	for (int i=0; i<BytesTransferred; i++)
		printf("%2x ", (unsigned char) info->wsaBuffer.buf[i]);
	fflush(stdin);	
	printf("\nsizeof bool is %d\n", sizeof(bool));


	info->TotalBytesReceived += BytesTransferred;
	info->wsaBuffer.buf += BytesTransferred;
	info->wsaBuffer.len -= BytesTransferred;

	if (info->TotalBytesReceived < info->TotalBytesToSend) {
		int result = WSARecv(info->socket, &info->wsaBuffer, 1, &NumSent, &Flags, Overlapped, DownLoadFile_Routine);
		// TODO: Error Checking.. Actually I have a  wrapper function..called _WSARecv()
	}
	else {
		//memcpy(&info->header, info->buffer, sizeof(F_TRANSFER_HEADER));
		//DWORD sizeof_header = sizeof(bool) + sizeof(DWORD) + sizeof(DWORD);
		//memcpy(&info->header, info->buffer, sizeof_header);

		// Get header

		DWORD sizeof_header = 0;
		memcpy(&info->header.Success, info->buffer, sizeof(bool));
		sizeof_header += sizeof(bool);

		memcpy(&info->header.Error, info->buffer + sizeof_header, sizeof(DWORD));
		sizeof_header += sizeof(DWORD);

		memcpy(&info->header.FileSize, info->buffer + sizeof_header, sizeof(DWORD));
		sizeof_header += sizeof(DWORD);

		//
		info->TotalBytesReceived = 0;
		info->wsaBuffer.buf = (CHAR *) info->buffer;
		info->wsaBuffer.len = BUFSIZE < info->header.FileSize ? BUFSIZE : info->header.FileSize;

		if (info->header.Success) {
			// Ok now download the file

			_WSARecv(info->socket, &info->wsaBuffer, 1, &NumSent, &Flags, Overlapped, helper_DownLoadFile_Routine);
			return;
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
		SetEvent(info->eventDone);
		GlobalFree(info);
		return;
	}

	info->wsaBuf.buf += BytesTransferred;
	info->wsaBuf.len -= BytesTransferred;

	WSASend(info->socket, &info->wsaBuf, 1, &NumSent, Flags, &info->overlapped, SendRequest_Routine);
	//SleepEx(INFINITE, true);
}


void SendRequest(SOCKET sd, OVERLAPPED overlapped, size_t size_data, command_t command, CHAR *data) {

	DWORD NumSend, Flags = 0;

	LPSOCK_REQUEST_INFO SocketInfo;

	HANDLE events[NUM_EVENTS];
	CreateFileTransferEvents(events);

	if ((SocketInfo = (LPSOCK_REQUEST_INFO) GlobalAlloc(GPTR,
         sizeof(SOCK_REQUEST_INFO))) == NULL) {

		char errorStr[100];
        sprintf(errorStr, "GlobalAlloc() failed with error %d\n", GetLastError());
		MessageBox(0, errorStr, "Error", MB_OK);
        return;
    }

	SocketInfo->socket = sd;
	SocketInfo->overlapped = overlapped;
	SocketInfo->eventDone = events[SEND_REQUEST];
	
	SocketInfo->msg.size = size_data;
	SocketInfo->msg.command = command; 
	memcpy(SocketInfo->BufferData, &SocketInfo->msg, sizeof(SocketInfo->msg));
	strcpy(SocketInfo->BufferData + 5, data);  // 5 is the sizeof(int) + sizeof(unsigned char)

	printf("sizeof(unsigned char) %d\n", sizeof(unsigned char));
	printf("sizeof(int) %d\n", sizeof(int));
	printf("sizeof(REQUEST_MSG) %d\n", sizeof(REQUEST_MSG));


	SocketInfo->wsaBuf.buf = SocketInfo->BufferData;
	SocketInfo->wsaBuf.len = sizeof(int) + sizeof(command_t) + size_data;
	SocketInfo->totalBytesToSend = SocketInfo->wsaBuf.len;


	printf("BufferData: \n");
	
	for (int i=0; i<SocketInfo->wsaBuf.len; i++)
		printf("%x ", SocketInfo->wsaBuf.buf[i]);
	printf("\n");
	for (int i=0; i<SocketInfo->wsaBuf.len; i++)
		printf("%c ", SocketInfo->wsaBuf.buf[i]);
	printf("\n");
	

	int result = WSASend(SocketInfo->socket, &SocketInfo->wsaBuf, 1, &NumSend, Flags, &SocketInfo->overlapped, SendRequest_Routine);

}

void DownloadFile(char *destination, char *source, char *host, int port) {

	DWORD Sent, Flags = 0;

	LPSOCK_FILE_DOWNLOAD_INFO SocketInfo; 

	HANDLE events[NUM_EVENTS];
	CreateFileTransferEvents(events);

	if ((SocketInfo = (LPSOCK_FILE_DOWNLOAD_INFO) GlobalAlloc(GPTR,
         sizeof(SOCK_FILE_DOWNLOAD_INFO))) == NULL) {

		char errorStr[100];
        sprintf(errorStr, "GlobalAlloc() failed with error %d\n", GetLastError());
		MessageBox(0, errorStr, "Error", MB_OK);
        return;
    }

	SocketInfo->filename = source;
	SocketInfo->wsaBuffer.buf = SocketInfo->buffer;
	SocketInfo->wsaBuffer.len = sizeof(bool) + sizeof(DWORD) + sizeof(DWORD);
	SocketInfo->TotalBytesReceived = 0;
	SocketInfo->TotalBytesToSend = SocketInfo->wsaBuffer.len;

	if ((SocketInfo->socket = ConnectToServer(host, port)) == -1) {
		// ConnectToServer displays MessageBox if there's an error

		Cleanup_FileDownload(SocketInfo);
		return;
	}

	printf("Socket: %d\n", SocketInfo->socket);
	
	memset(&SocketInfo->overlapped, 0, sizeof(OVERLAPPED));
	SendRequest(SocketInfo->socket, SocketInfo->overlapped, strlen(source) + 1, 1, source); 
	
	SleepEx(INFINITE, true);
	
	// TODO: look at other possible return values
	if (WAIT_OBJECT_0 == WaitForSingleObject(events[SEND_REQUEST], INFINITE)) //Timeout here might be good?
		printf("Sent the request to server.\n");
	
	int result = _WSARecv(SocketInfo->socket, &SocketInfo->wsaBuffer, 1, &Sent, &Flags, &SocketInfo->overlapped, DownLoadFile_Routine);
	SleepEx(INFINITE, true);

	WaitForSingleObject(events[FILE_TRANSFER], INFINITE); // Again timeout here might be good?
	printf("\nFile Transfer complete\n");
}

DWORD WINAPI ThreadFunc(LPVOID n)
{    
	WORD wVersionRequested;
	WSADATA WSAData;
	int err;

	HANDLE events[NUM_EVENTS];
	CreateFileTransferEvents(events);

	wVersionRequested = MAKEWORD( 2, 2 );
	err = WSAStartup( wVersionRequested, &WSAData );
	if ( err != 0 ) //No usable DLL
	{
		printf ("DLL not found!\n");
		exit(1);
	}	
	
	DownloadFile("downloaded_test.txt4", "file_transfer.cpp", "localhost", 5555);
	
	WSACleanup();
	SetEvent(events[FILE_TRANFER_THREAD_DONE]);
	//exit(0);
    return 0;
}

int main() {
	HANDLE events[NUM_EVENTS];
	CreateFileTransferEvents(events);

	HANDLE hThrd;
    DWORD threadId;
    hThrd = CreateThread(NULL, 0, ThreadFunc, NULL, 0, &threadId );

	WaitForSingleObject(events[FILE_TRANFER_THREAD_DONE], INFINITE);

	printf("Press <ESC> to exit\n");
	while (_getch() != 27)
		Sleep(100);

}

#if 0

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
#endif
