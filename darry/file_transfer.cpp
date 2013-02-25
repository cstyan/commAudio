#include "file_transfer.h"
#include "wrappers.h"

#define FILE_READ_SIZE 1024

typedef struct {
	OVERLAPPED overlapped;
	SOCKET socket;
	HANDLE hFile;
	DWORD dwFileSize;
	WSABUF wsaBuffer; 
	CHAR buffer[FILE_READ_SIZE];
	DWORD BytesSent;
	DWORD BytesInBuffer;
} SOCK_FILE_TR_INFO, *LPSOCK_FILE_TR_INFO;


/*---------------------------------------------

   Request Functions 

----------------------------------------------*/


/*
 
 Helper function for r_get and FileTransfer_Routine

*/

void fillBuffer(LPSOCK_FILE_TR_INFO	info, bool putFileSize = false) 
{
	DWORD amountRead;

	// resize buffer to maximum size
	info->wsaBuffer.buf = info->buffer;
	info->wsaBuffer.len = FILE_READ_SIZE;

	// This part is a bit convoluted, I know.. I can clean it up
	if (putFileSize)
	{
		size_t file_size = (size_t) info->dwFileSize;
		memcpy(info->buffer, &file_size, sizeof(size_t));
		info->wsaBuffer.buf += sizeof(size_t);
		info->wsaBuffer.len -= sizeof(size_t);
	}

	ReadFile(info->hFile, info->wsaBuffer.buf, info->wsaBuffer.len, &(info->BytesInBuffer), NULL);
	info->wsaBuffer.len = info->BytesInBuffer;

	if (putFileSize)
	{
		// Move buffer back so it includes the filesize
		info->wsaBuffer.buf -= sizeof(size_t);
		info->wsaBuffer.len += sizeof(size_t);
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

	if (info->BytesSent == info->dwFileSize) 
	{
		closesocket(info->socket);
		CloseHandle(info->hFile);
		GlobalFree(info);
		return;
	}

	if (info->BytesInBuffer == 0)
		fillBuffer(info);

	//ZeroMemory(&(info->overlapped), sizeof(WSAOVERLAPPED));
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


void r_get(LPSOCK_RECV_INFO si, LPWSABUF request_data)
{
	

	LPSOCK_FILE_TR_INFO SocketInfo; 

	if ((SocketInfo = (LPSOCK_FILE_TR_INFO) GlobalAlloc(GPTR,
         sizeof(SOCK_FILE_TR_INFO))) == NULL)
    {
        printf("GlobalAlloc() failed with error %d\n", GetLastError());
        return;
    }

	SocketInfo->overlapped = si->overlapped;
	SocketInfo->socket = si->Socket;
	SocketInfo->wsaBuffer.buf = SocketInfo->buffer;
	SocketInfo->wsaBuffer.len = FILE_READ_SIZE;
	
	SocketInfo->hFile = CreateFile(request_data->buf,
						GENERIC_READ,
                        0,
                        (LPSECURITY_ATTRIBUTES) NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        (HANDLE) NULL);
	
	// TODO: Error check... hFile

	SocketInfo->dwFileSize = GetFileSize(SocketInfo->hFile, NULL);
	SocketInfo->BytesInBuffer = 0;
	SocketInfo->BytesSent = 0;

	DWORD Flags = 0;
	DWORD Sent = 0;

	fillBuffer(SocketInfo, false); //change to true later
	FileTransfer_Routine(0, 0, &(SocketInfo->overlapped), Flags);

}

void r_listing(LPSOCK_RECV_INFO si, LPWSABUF request_data)
{
	MessageBox(0, "Listing Request", "listing Request", MB_OK);
}

/*---------------------------------------------

   Completion Routines   

----------------------------------------------*/

void run_request(LPSOCK_RECV_INFO si, Request_Msg *request) {

	switch (request->command) 
	{
	case GET:
		r_get(si, &request->request_data);
		break;

	case LISTING:
		r_listing(si, &request->request_data);
		break;

	default:
		// error
		break;
	}


	// For now close socket here..
	// Free si

	//closesocket(si->Socket);
	//GlobalFree(si);

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

	// Now I realize we don't need two buffers..
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

	if (length < (info->bytes_received - sizeof_header))
		return false;

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
		msg.request_data.len -= sizeof_header;
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
		run_request(info, &msg);
	}
	else 
	{
		_WSARecv(info->Socket, &info->wsarecv_buffer, 1, &BytesTransferred, &Flags, Overlapped, Recv_Request_Routine);
	}
	
}

