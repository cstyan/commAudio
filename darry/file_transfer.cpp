#include "file_transfer.h"
#include "wrappers.h"

#define FILE_READ_SIZE 1024

typedef struct {
	bool Success;
	DWORD Error;
	DWORD FileSize;
} F_TRANSFER_HEADER;

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

typedef struct {
	OVERLAPPED overlapped;
	SOCKET socket;
	WSABUF wsaBuffer;
	std::string files_names;
} SOCK_FILE_LIST_INFO, *LPSOCK_FILE_LIST_INFO;

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
		memcpy(info->buffer, &info->header, sizeof(F_TRANSFER_HEADER));
		info->BytesInBuffer = sizeof(F_TRANSFER_HEADER);
		info->wsaBuffer.len = sizeof(F_TRANSFER_HEADER);
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
	if (info->BytesSent == info->header.FileSize + sizeof(F_TRANSFER_HEADER)) 
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
		SocketInfo->hFile = CreateFile(si->request_data.buf,
							GENERIC_READ,
							FILE_SHARE_READ,
							(LPSECURITY_ATTRIBUTES) NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							(HANDLE) NULL);
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

