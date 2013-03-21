/*
 *	SOURCE FILE: addconsole.c
 *	PURPOSE: Adds a console to a Win32 project through addConsole(). stdout, stdin and stderr are all redirected to this console.
 *	APPLICATION: none
 *	CREATION DATE: February-27-2013
 *	DESIGNER: Albert Liao
 *	PROGRAMMER: Albert Liao
 *
 *	REVISION HISTORY:
 *		February-27-2013 - Initial Draft.
 *	
 *	FUNCTIONS:
 *		void addConsole(); - Creates a console window and redirects stdout, stdin and stderr to it.
 *	
 *	NOTES:
 *		This code is an adaptation of Andrew Tucker's example from http://www.halcyon.com/~ast/dload/guicon.htm.
 */
#include <windows.h>
#include <fcntl.h>
#include <fstream>
#include <io.h>
#include <iostream>
#include <stdio.h>
//#include "AddConsole.h"
#include "AddConsole.h"

using namespace std;

static const WORD MAX_CONSOLE_LINES = 500; // The maximum number of lines to buffer for the console.

/*
 *	Function: void addConsole()
 *	Creation Date: February 27th, 2013
 *	Designer: Albert Liao
 *	Programmer: Albert Liao
 *
 *	Revision History:
 *		February 27th 2013 - Initial Draft
 *
 *	Parameters:
 *		none
 *	
 *	Return Value:
 *		none
 *	
 *	Notes:
 *		This function creates a console window and redirects stdin, stdout and stderr to it.
 */
void AddConsole::addConsole()
{
	long lStdWinHandle; // Used to temporarily store the windows handles for the std devices.
	int hConsoleCHandle; // Used to temporarily store C file descriptors of their windows equivalents.
	FILE *fp; // Used to temporarily store the stream to redirect to stdout, stdin and stderr.

	CONSOLE_SCREEN_BUFFER_INFO consoleInfo; // Structure used to change console buffer size.

	// Create the console window.
	AllocConsole();

	// Change the screen buffer to allow more lines.
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleInfo); // Get the current console values.
	consoleInfo.dwSize.Y = MAX_CONSOLE_LINES; // Change the number of lines in Y.
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), consoleInfo.dwSize); // Apply the new settings.

	// Redirect unbuffered STDOUT to the console.
	lStdWinHandle = (long) GetStdHandle(STD_OUTPUT_HANDLE); // Retrieve handle to the STDOUT device.
	hConsoleCHandle = _open_osfhandle(lStdWinHandle, _O_TEXT); // Get the C file descriptor version of the Windows handle.
	fp = _fdopen(hConsoleCHandle, "w"); // Associate the stream fp to the file descriptor in write mode.
	*stdout = *fp; // Set stdout to point to the stream.
	setvbuf(stdout, NULL, _IONBF, 0); // Turn off input / output buffering.

	// Redirect unbuffered STDINPUT to the console.
	lStdWinHandle = (long) GetStdHandle(STD_INPUT_HANDLE); // Retrieve handle to the STDIN device.
	hConsoleCHandle = _open_osfhandle(lStdWinHandle, _O_TEXT); // Get the C file descriptor version of the Windows handle.
	fp = _fdopen(hConsoleCHandle, "r"); // Associate the stream fp to the file descriptor in write mode.
	*stdin = *fp; // Set stdin to point to the stream.
	setvbuf(stdin, NULL, _IONBF, 0); // Turn off input / output buffering.

	// Redirect unbuffered STDERROR to the console.
	lStdWinHandle = (long) GetStdHandle(STD_ERROR_HANDLE); // Retrieve handle to the STDOUT device.
	hConsoleCHandle = _open_osfhandle(lStdWinHandle, _O_TEXT); // Get the C file descriptor version of the Windows handle.
	fp = _fdopen(hConsoleCHandle, "w"); // Associate the stream fp to the file descriptor in write mode.
	*stderr = *fp; // Set stderr to point to the stream.
	setvbuf(stderr, NULL, _IONBF, 0); // Turn off input / output buffering.

	// Make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well.
	ios::sync_with_stdio();
}
