/*
 *	SOURCE FILE: commGui.h
 *	PURPOSE: Has all of the configuration values for the application as well as the interfaces for the GUI.
 *	APPLICATION: COMP4985 Comm Audio Server Application
 *	CREATION DATE: February-27-2013
 *	DESIGNER: Albert Liao
 *	PROGRAMMER: Albert Liao
 *
 *	REVISION HISTORY:
 *		February-27-2013 - Initial Draft.
 *	
 *	FUNCTIONS:
 *		none
 *	
 *	NOTES:
 *		This file stores all of the configuration values for the application and has the interfaces for the GUI.
 */
#ifndef _COMM_AUDIO_COMM_GUI_H
#define _COMM_AUDIO_COMM_GUI_H

#include <windows.h>

// Structure used to keep track of the layout dimensions for the GUI.
struct CommGuiLayout
{
	COLORREF bgColor;
	int windowWidth;
	int windowHeight;
	int songListX;
	int songListY;
	int songListWidth;
	int songListHeight;
};

class CommGui
{
public:
	CommGuiLayout layout; // Used to store the dimensions of the layout.

	/* Handles to the button controls. */
	HWND hListBox; // For the song list.
	HWND hPlayButton; // For the play button.
	HWND hPauseButton; // For the pause button.
	HWND hRefreshListButton; // For the song list refresh button.

	CommGui(); // Constructor.
	void initLayout(CommGuiLayout &layout); // Initializes the layout.
	void updateSongList(TCHAR** songs, int numOfSongs);
};

#endif
