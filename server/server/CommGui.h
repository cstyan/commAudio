/*
 *	SOURCE FILE: commGui.h
 *	PURPOSE: Has methods to interface with the GUI.
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
 *		This code has the methods that interface with the GUI.
 */
#ifndef _COMM_AUDIO_COMM_GUI_H
#define _COMM_AUDIO_COMM_GUI_H

#include <windows.h>

class CommGui
{
public:
	static void updateSongList(TCHAR** songs, int numOfSongs);
};

#endif
