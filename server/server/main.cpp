/*
 *	SOURCE FILE: main.cpp
 *	PURPOSE: GUI and message handler for the server program
 *	APPLICATION: COMP4985 Comm Audio Server Application
 *	CREATION DATE: February-27-2013
 *	DESIGNER: Albert Liao
 *	PROGRAMMER: Albert Liao
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
#include "Config.h"
#include "AddConsole.h"

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM); // The WinProc to handle all messages.

int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
{
	HWND hwnd;
	HWND hListBox;
	MSG Msg;
	WNDCLASSEX Wcl;
	Config config = new Config();
	
	// Define a Window class.
	Wcl.cbSize = sizeof (WNDCLASSEX);
	Wcl.style = 0; // Use default style.
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // Large icon.
	Wcl.hIconSm = NULL; // Use small version of large icon.
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // Cursor style.
	Wcl.lpfnWndProc = WndProc; // Window function.
	Wcl.hInstance = hInst; // Handle to this instance.
	Wcl.hbrBackground = CreateSolidBrush(bgColor); // 
	Wcl.lpszClassName = tstrClassName; // Window class name.
	Wcl.lpszMenuName = NULL; // No class menu.
	Wcl.cbClsExtra = 0; // No extra memory needed.
	Wcl.cbWndExtra = 0; 
	
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

	// Add a ListBox.
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
		hInst,
		NULL
	);

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
	switch (Message)
	{
		case WM_DESTROY:
			PostQuitMessage (0);
			break;
		
		default: // Let Win32 process all other messages.
			return DefWindowProc (hwnd, Message, wParam, lParam);
	}

	return 0;	
}