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
#include "file_transfer.h"
#include "FileList.h"
#include <windows.h> 
#include <iostream>
#include "resource.h"
#include "CommGui.h"
#include "AddConsole.h"

#define CONSOLE_ENABLED 1 // Used to enable or disable the extra console window. 1 - Enabled, 0 - Disabled
#define MAX_PATH_LENGTH 128 // Used as the maximum path length for a directory.

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM); // The WinProc to handle all messages.

int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
{
	/* Variables */
	HWND hwnd;
	MSG Msg;
	WNDCLASSEX Wcl;
	
	int returnValue;
	WSADATA wsaData;
   
	if ((returnValue = WSAStartup(0x0202,&wsaData)) != 0)
	{
		cout << "WSAStartup failed with error " << returnValue << endl;
		WSACleanup();
		return 1;
	}

	// Window Configuration Values.
	COLORREF bgColor = RGB(100, 100, 100); // Background color.
	int windowWidth = 500; // Dimensions of the window.
	int windowHeight = 710;
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
	HDC hdc;

	/* Handles to the button controls. */
	static HWND hListBox; // For the song list.
	static HWND hFolderText; // To display the current folder.
	static HWND hChooseFolderButton; // To choose the folder to get songs from.
	static HWND hRefreshListButton; // For the song list refresh button.
	static HWND hPlayButton; // For the play button.
	static HWND hPauseButton; // For the pause button.
	static HWND hMicButton; // For the microphone.
	static HWND hStatusText; // To display the application status.

	/* Control layout values. */
	static int folderTextX = 15;
	static int folderTextY = 10;
	static int folderTextWidth = 200;
	static int folderTextHeight = 20;
	static int songListX = 10;
	static int songListY = 40;
	static int songListWidth = 465;
	static int songListHeight = 550;
	static int chooseFolderButtonX = 245;
	static int chooseFolderButtonY = 8;
	static int chooseFolderWidth = 110;
	static int chooseFolderHeight = 25;
	static int refreshListButtonX = 365;
	static int refreshListButtonY = 8;
	static int refreshListWidth = 110;
	static int refreshListHeight = 25;
	static int playButtonX = 15;
	static int playButtonY = 595;
	static int playButtonWidth = 110;
	static int playButtonHeight = 25;
	static int pauseButtonX = 135;
	static int pauseButtonY = 595;
	static int pauseButtonWidth = 110;
	static int pauseButtonHeight = 25;
	static int micButtonX = 365;
	static int micButtonY = 595;
	static int micButtonWidth = 110;
	static int micButtonHeight = 25;
	static int statusTextX = 10;
	static int statusTextY = 625;
	static int statusTextWidth = 465;
	static int statusTextHeight = 20;

	/* Variables. */
	static TCHAR folderPath[MAX_PATH_LENGTH]; // String that stores the path of the folder the user selects for music.

	static HANDLE hFileTransferThread;
	static DWORD dwFileTransferThreadId;

	switch (Message)
	{
	case WM_CREATE:
#if(CONSOLE_ENABLED)
		// Start console.
		AddConsole::addConsole();
#endif

		hFileTransferThread = CreateThread(NULL, 0, FileTransferThread, NULL, 0, &dwFileTransferThreadId);

		/* Initialize all controls / child windows here. */
		// The text for the currently selected folder.
		hFolderText = CreateWindowEx(
			0,
			"STATIC",
			"No folder chosen.",
			WS_CHILD | WS_VISIBLE,
			folderTextX,
			folderTextY,
			folderTextWidth,
			folderTextHeight,
			hwnd,
			(HMENU) IDC_FOLDERTEXT,
			GetModuleHandle(NULL),
			NULL
		);

		// The choose folder button.
		hChooseFolderButton = CreateWindowEx(
			NULL, 
			"BUTTON",
			"Choose Folder",
			WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
			chooseFolderButtonX,
			chooseFolderButtonY,
			chooseFolderWidth,
			chooseFolderHeight,
			hwnd,
			(HMENU) IDC_CHOOSEFOLDERBUTTON,
			GetModuleHandle(NULL),
			NULL
		);

		// The refresh list button.
		hRefreshListButton = CreateWindowEx(
			NULL, 
			"BUTTON",
			"Refresh List",
			WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
			refreshListButtonX,
			refreshListButtonY,
			refreshListWidth,
			refreshListHeight,
			hwnd,
			(HMENU) IDC_REFRESHLISTBUTTON,
			GetModuleHandle(NULL),
			NULL
		);

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
			(HMENU) IDC_SONGLISTBOX,
			GetModuleHandle(NULL),
			NULL
		);

		// The play button.
		hRefreshListButton = CreateWindowEx(
			NULL, 
			"BUTTON",
			"PLAY",
			WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
			playButtonX,
			playButtonY,
			playButtonWidth,
			playButtonHeight,
			hwnd,
			(HMENU) IDC_PLAYBUTTON,
			GetModuleHandle(NULL),
			NULL
		);

		// The pause button.
		hRefreshListButton = CreateWindowEx(
			NULL, 
			"BUTTON",
			"PAUSE",
			WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
			pauseButtonX,
			pauseButtonY,
			pauseButtonWidth,
			pauseButtonHeight,
			hwnd,
			(HMENU) IDC_PAUSEBUTTON,
			GetModuleHandle(NULL),
			NULL
		);

		// The mic button.
		hRefreshListButton = CreateWindowEx(
			NULL, 
			"BUTTON",
			"MIC",
			WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
			micButtonX,
			micButtonY,
			micButtonWidth,
			micButtonHeight,
			hwnd,
			(HMENU) IDC_MICBUTTON,
			GetModuleHandle(NULL),
			NULL
		);

		// The status text of what the application is doing.
		hStatusText = CreateWindowEx(
			0,
			"STATIC",
			"Not currently playing any song.",
			WS_CHILD | WS_VISIBLE,
			statusTextX,
			statusTextY,
			statusTextWidth,
			statusTextHeight,
			hwnd,
			(HMENU) IDC_STATUSTEXT,
			GetModuleHandle(NULL),
			NULL
		);

		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		// The folder button was pressed. Let the user choose a folder and display it.
		case IDC_CHOOSEFOLDERBUTTON:
			// Open up a dialog for the user to choose a folder.
			cout << "User is selecting a new folder." << endl;
			if(CommGui::GetFolderSelection(hwnd, folderPath, TEXT("Please select a folder.")) != NULL) // If the user selects a folder.
			{
				// Clear the listbox.
				SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
				// Update the text displaying the currently selected folder.
				SendMessage(hFolderText, WM_SETTEXT, 0, (LPARAM)folderPath);
				cout << "Folder \"" << folderPath << "\" selected." << endl;
				// Update the listbox with all of the files available in that folder.
				CommGui::updateSongList(hListBox, CommGui::find_files(folderPath, "*"));
			}
			else
			{
				cout << "No folder selected." << endl;
			}
			break;

		// The refresh button was pressed, reload the file list for the selected folder.
		case IDC_REFRESHLISTBUTTON:
			// Clear the listbox.
			SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
			// Update the listbox with all the files in the folder originally selected.
			CommGui::updateSongList(hListBox, CommGui::find_files(folderPath, "*"));
			cout << "Refreshing file list from folder \"" << folderPath << "\"." << endl;
			break;

		// The play button was pressed.
		case IDC_PLAYBUTTON:
			cout << "Now playing." << endl;
			break;

		// The pause button was pressed.
		case IDC_PAUSEBUTTON:
			cout << "Pausing." << endl;
			break;

		// The mic button was pressed.
		case IDC_MICBUTTON:
			break;

		/* Menu Stuff */
		case ID_TOOLS_SETTINGS:
			break;

		case ID_HELP_ABOUT:
			MessageBox(hwnd, TEXT("Made by Albert, Callum, Darry, Steve. (C)2013"), TEXT("About"), NULL);
			break;

		case ID_FILE_EXIT:
			PostQuitMessage (0);
			break;
		}

		break;

	case WM_DESTROY:
		WSACleanup();
		PostQuitMessage (0);
		break;
		
	default: // Let Win32 process all other messages.
		return DefWindowProc (hwnd, Message, wParam, lParam);
	}

	return 0;	
}