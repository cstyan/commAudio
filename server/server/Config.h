/*
 *	SOURCE FILE: config.h
 *	PURPOSE: Has all of the configuration values for the application.
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
 *		This file stores all of the configuration values for the application.
 */
#ifndef _COMM_AUDIO_CONFIG_H
#define _COMM_AUDIO_CONFIG_H

class Config
{
public:
	TCHAR* tstrClassName = TEXT("COMP4985_commaudio_server"); // Used as the class name for the program.
	TCHAR* tstrWindowTitle = TEXT("Comm Audio Server"); // Used as the title for the main window.
	COLORREF bgColor = RGB(100, 100, 100); // Used for the background color of the main window.
	int windowWidth = 500; // Used for the width of the window.
	int windowHeight = 600; // Used for the height of the window.
	int songListX = 250;
	int songListY = 25;
	int songListWidth = 225;
	int songListHeight = 500;

	Config();
};

#endif
