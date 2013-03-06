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
 *		void updateSongList(HWND hListBox, vector<string> songs)
 *	
 *	NOTES:
 *		This code has the methods that interface with the GUI.
 */
#ifndef _COMM_AUDIO_COMM_GUI_H
#define _COMM_AUDIO_COMM_GUI_H

#include <windows.h>
#include <string>
#include <ShlObj.h>
#include <vector>

using namespace std;

class CommGui
{
public:
	/*
	 *	Function: void updateSongList(HWND hListBox, vector<string> songs)
	 *	Creation Date: March 4th, 2013
	 *	Designer: Albert Liao
	 *	Programmer: Albert Liao
	 *
	 *	Revision History:
	 *		March 4th 2013 - Initial Draft
	 *
	 *	Parameters:
	 *		hListBox - the handle to the listbox to update
	 *		songs - a string vector of songnames (filenames)
	 *	
	 *	Return Value:
	 *		none
	 *	
	 *	Notes:
	 *		This function adds all the strings in a vector to a listbox.
	 */
	static void CommGui::updateSongList(HWND hListBox, vector<string> songs);

	/*
	 *	Function: vector<string> CommGui::find_files()
	 *	Creation Date: March 4th, 2013
	 *	Designer: Darry Danzig
	 *	Programmer: Darry Danzig
	 *
	 *	Revision History:
	 *		March 4th 2013 - Initial Draft
	 *
	 *	Parameters:
	 *		none
	 *	
	 *	Return Value:
	 *		returns a vector of filenames
	 *	
	 *	Notes:
	 *		This function returns a list of filenames in the directory.
	 */
	static vector<string> CommGui::find_files(string directory, string fileExtension);

	/*
	 *	Function: BOOL CommGui::GetFolderSelection(HWND hwnd, LPTSTR szBuf, LPCTSTR szTitle)
	 *	Creation Date: March 4th, 2013
	 *	Designer: anonytmouse
	 *	Programmer: Albert Liao
	 *
	 *	Revision History:
	 *		March 4th 2013 - Initial Draft
	 *
	 *	Parameters:
	 *		hwnd - handle to the window
	 *		szBuf - TCHAR* to write the path of the folder chosen to
	 *		szTitle - Title of the window
	 *	
	 *	Return Value:
	 *		none
	 *	
	 *	Notes:
	 *		Taken from http://cboard.cprogramming.com/windows-programming/59282-browse-folder-dialog.html.
	 *		This function opens up a dialog box for the user to select a folder from and then the path is
	 *		saved to the string szBuf.
	 */
	static LPITEMIDLIST CommGui::GetFolderSelection(HWND hwnd, LPTSTR szBuf, LPCTSTR szTitle);
};

#endif
