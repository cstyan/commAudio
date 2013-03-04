/*
 *	SOURCE FILE: main.cpp
 *	PURPOSE: GUI and message handler for the server program
 *	APPLICATION: COMP4985 Comm Audio Server Application
 *	CREATION DATE: February-27-2013
 *	DESIGNER: Albert Liao, Callum Styan, Darry Danzig, Steve Lo
 *	PROGRAMMER: Albert Liao, Callum Styan, Darry Danzig, Steve Lo
 *
 *	REVISION HISTORY:
 *		February-27-2013 - Initial Draft.
 *	
 *	FUNCTIONS:
 *		int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow); - Entry point.
 *		LRESULT CALLBACK WndProc (HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) - WinProc that handles messages.
 *	
 *	NOTES:
 *		This code creates the GUI for the server application and handles all the messages for the program.
 */
#include <windows.h> 
#include <iostream>
#include "resource.h"
#include "CommGui.h"
#include "AddConsole.h"

#define CONSOLE_ENABLED 1 // Used to enable or disable the extra console window. 1 - Enabled, 0 - Disabled

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM); // The WinProc to handle all messages.

int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
{
	/* Variables */
	HWND hwnd;
	MSG Msg;
	WNDCLASSEX Wcl;
	
	// Window Configuration Values.
	COLORREF bgColor = RGB(100, 100, 100); // Background color.
	int windowWidth = 500; // Dimensions of the window.
	int windowHeight = 600;
	TCHAR* tstrClassName = TEXT("COMP4985_commaudio_server"); // Used as the class name for the program.
	TCHAR* tstrWindowTitle = TEXT("Comm Audio Server"); // Used as the title for the main window.

	// Define a Window class.
	Wcl.cbSize = sizeof (WNDCLASSEX);
	Wcl.style = 0; // Use default style.
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // Large icon.
	Wcl.hIconSm = NULL; // Use small version of large icon.
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // Cursor style.
	Wcl.lpfnWndProc = WndProc; // Window function.
	Wcl.hInstance = hInst; // Handle to this instance.
	Wcl.hbrBackground = CreateSolidBrush(bgColor); // Background color for window.
	Wcl.lpszClassName = tstrClassName; // Window class name.
	Wcl.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1); // No class menu.
	Wcl.cbClsExtra = 0; // No extra memory needed.
	Wcl.cbWndExtra = sizeof(CommGui); 
	
	// Register the class.
	if (!RegisterClassEx (&Wcl))
		return 0;

	// Create the main window.
	hwnd = CreateWindow(
		tstrClassName, // Type of class.
		tstrWindowTitle, // Title for the window.
		WS_OVERLAPPEDWINDOW, // Window style - normal.
		CW_USEDEFAULT,	// X coordinate.
		CW_USEDEFAULT, // Y coordinate.
		windowWidth, // Width of window.
		windowHeight, // Height of window.
		NULL, // No parent window.
		NULL, // No menu.
		hInst, // Handle to the instance.
		NULL // No additional arguments.
	);

	// Display the window.
	ShowWindow (hwnd, nCmdShow);
	UpdateWindow (hwnd);

	// Create the message loop.
	while (GetMessage (&Msg, NULL, 0, 0))
	{
   		TranslateMessage (&Msg); // Translate keyboard messages.
		DispatchMessage (&Msg); // Dispatch the message and return control to windows.
	}

	return Msg.wParam;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	/* Handles to the button controls. */
	HWND hListBox; // For the song list.
	HWND hPlayButton; // For the play button.
	HWND hPauseButton; // For the pause button.
	HWND hRefreshListButton; // For the song list refresh button.

	/* Control layouts. */
	static int songListX = 250;
	static int songListY = 25;
	static int songListWidth = 225;
	static int songListHeight = 500;

	switch (Message)
	{
	case WM_CREATE:
#if(CONSOLE_ENABLED)
		// Start console.
		AddConsole::addConsole();
#endif
		std::cout<< "YOOOOOO";

		/* Initialize all controls / child windows here. */
		// Add the song listbox.
		hListBox = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			TEXT("LISTBOX"),
			NULL,
			WS_CHILD | WS_VISIBLE | WS_VSCROLL,
			songListX,
			songListY,
			songListWidth,
			songListHeight,
			hwnd,
			NULL,
			GetModuleHandle(NULL),
			NULL
		);
		/*
		hPlayButton = CreateWindowEx(NULL, 
			"BUTTON",
			"OK",
			WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
			50,
			220,
			100,
			24,
			hWnd,
			(HMENU)IDC_MAIN_BUTTON,
			GetModuleHandle(NULL),
			NULL);
			*/

		break;

	case WM_DESTROY:
		PostQuitMessage (0);
		break;
		
	default: // Let Win32 process all other messages.
		return DefWindowProc (hwnd, Message, wParam, lParam);
	}

	return 0;	
}