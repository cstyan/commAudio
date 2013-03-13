#include "wrappers.h"

SOCKET _WSASocket( int af, int type, int protocol, LPWSAPROTOCOL_INFO lpProtocolInfo, GROUP g, DWORD dwFlags) 
{
	SOCKET s;
	std::string errstr;

	s = WSASocket( af,type, protocol, lpProtocolInfo, g, dwFlags);


	if ( s == INVALID_SOCKET ) {

		switch (int error = WSAGetLastError()) {

		case WSANOTINITIALISED:
			errstr = "A successful WSAStartup call must occur before using this function.";
			break;
		case WSAENETDOWN:
			errstr = "The network subsystem has failed.";
			break;
		case WSAEAFNOSUPPORT:
			errstr = "The specified address family is not supported.";
			break;
		case WSAEFAULT:
			errstr = "The lpProtocolInfo parameter is not in a valid part of the process address space.";
			break;
		case WSAEINPROGRESS:
			errstr = "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";
			break;
		case WSAEINVAL:
			errstr = "This value is true for any of the following conditions.";
			errstr += "\nThe parameter g specified is not valid.";
			errstr += "\nThe WSAPROTOCOL_INFO structure that lpProtocolInfo points to is incomplete, the contents are invalid or the WSAPROTOCOL_INFO structure has already been used in an earlier duplicate socket operation.";
			errstr += "\nThe values specified for members of the socket triple <af, type, and protocol> are individually supported, but the given combination is not.";
			break;
		case WSAEMFILE:
			errstr = "No more socket descriptors are available.";
			break;
		case WSAENOBUFS:
			errstr = "No buffer space is available. The socket cannot be created.";
			break;
		case WSAEPROTONOSUPPORT:
			errstr = "The specified protocol is not supported.";
			break;
		case WSAEPROTOTYPE:
			errstr = "The specified protocol is the wrong type for this socket.";
			break;
		case WSAEPROVIDERFAILEDINIT:
			errstr = "The service provider failed to initialize. This error is returned if a layered service provider (LSP) or namespace provider was improperly installed or the provider fails to operate correctly.";
			break;
		case WSAESOCKTNOSUPPORT:
			errstr = "The specified socket type is not supported in this address family.";
			break;

		/*
		case WSAINVALIDPROVIDER:
			errstr = "The service provider returned a version other than 2.2.";
		case WSAINVALIDPROCTABLE:
			errstr = "The service provider returned an invalid or incomplete procedure table to the WSPStartup.";
		*/

		default:
			{
				char error_s[256];
				sprintf(error_s, "Error with WSASocket() error: %d", error);

				errstr = error_s;
			}
		}

		MessageBox(0, errstr.c_str(), TEXT("Error: _WSASocket"), MB_OK);

	}

	return s;
}

int _WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData)
{
	char *error_str;
	int result; 

	result = WSAStartup(wVersionRequested, lpWSAData);
	

	if (result != 0) {

		WSACleanup();
		switch (result) {

		case WSASYSNOTREADY:
			error_str = "The underlying network subsystem is not ready for network communication.";
			break;
		case WSAVERNOTSUPPORTED:
			error_str = "The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation.";
			break;
		case WSAEINPROGRESS:
			error_str = "A blocking Windows Sockets 1.1 operation is in progress.";
			break;
		case WSAEPROCLIM:
			error_str = "A limit on the number of tasks supported by the Windows Sockets implementation has been reached.";
			break;
		case WSAEFAULT:
			error_str = "The lpWSAData parameter is not a valid pointer.";
			break;
		}

		MessageBox(0, error_str, "Error: _WSAStartup", MB_OK);
	}
	return result;
}


int _bind(SOCKET s, const struct sockaddr *name, int namelen)
{
	int result;
	
	result = bind(s, name, namelen);

	if (result == SOCKET_ERROR) {

		char error_str[256]; 

		sprintf(error_str, "Error bind() function. Error: %d", WSAGetLastError());
		MessageBox(0, error_str, "Error: bind()", MB_OK); 
	}

	return result;
}


int _listen(SOCKET s, int backlog)
{
	int result;

	result =  listen(s, backlog);

	if (result == SOCKET_ERROR) {

		char error_str[256]; 

		sprintf(error_str, "Error listen() function. Error: %d", WSAGetLastError());
		MessageBox(0, error_str, "Error: listen()", MB_OK); 
	}


	return result;
}

WSAEVENT _WSACreateEvent(void)
{
	WSAEVENT event;


	event = WSACreateEvent();

	if (event == WSA_INVALID_EVENT)
	{
		char *error_str;
		
		switch (WSAGetLastError()) 
		{

		case WSANOTINITIALISED:
			error_str = "A successful WSAStartup call must occur before using this function.";
			break;

		case WSAENETDOWN:
			error_str = "The network subsystem has failed.";
			break;

		case WSAEINPROGRESS:
			error_str = "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";
			break;

		case WSA_NOT_ENOUGH_MEMORY:
			error_str = "Not enough free memory available to create the event object.";
			break;

		}
		MessageBox(0, error_str, "Error: WSACreateEvent()", MB_OK);
	}

	return event;
}

int _WSARecv(SOCKET S, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	int result;

	result = WSARecv(S, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine);

	if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		MessageBox(0, "error WSARecv", "error", MB_OK);
	}

	return result;
}