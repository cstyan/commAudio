#include "CommGui.h"
#include "resource.h"
#include <iostream>

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
void CommGui::updateSongList(HWND hListBox, vector<string> songs)
{
	// Go through all the elements in the vector.
	for(vector<string>::iterator it = songs.begin(); it != songs.end(); ++it)
	{
		// Add them to the list box.
		SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)((string) *it).c_str());
	}
}

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
vector<string> CommGui::find_files(string directory, string fileExtension)
{
	WIN32_FIND_DATA file;
	HANDLE h;
	vector<string> filenames;
	
	// Build the directory search string to look for files.
	directory += "\\*.";
	directory += fileExtension;

	h = FindFirstFile(directory.c_str(), &file);
	
	if(h == INVALID_HANDLE_VALUE)
	{
		// If we can't find any files in the directory print an error to console.
		if(GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			cerr << "No matching files found in directory: " << directory << endl;
		}
	}
	else
	{
		// Ignore the . and .. "files" otherwise, add the filename to the vector.
		if(strcmp(file.cFileName, ".") != 0 && strcmp(file.cFileName, "..") != 0)
		{
			filenames.push_back(file.cFileName);
		}

		// Loop through to go through all the files.
		while (FindNextFile(h, &file))
		{
			// Ignore the . and .. "files" otherwise, add the filename to the vector.
			if(strcmp(file.cFileName, ".") != 0 && strcmp(file.cFileName, "..") != 0)
			{
				filenames.push_back(file.cFileName);
			}
		}
	}

	return filenames;
}

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
void CommGui::GetFolderSelection(HWND hwnd, LPTSTR szBuf, LPCTSTR szTitle)
{
	LPITEMIDLIST pidl     = NULL;
	BROWSEINFO   bi       = {0};

	bi.hwndOwner      = hwnd;
	bi.pszDisplayName = szBuf;
	bi.pidlRoot       = NULL;
	bi.lpszTitle      = szTitle;
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;

	if ((pidl = SHBrowseForFolder(&bi)) != NULL)
	{
		SHGetPathFromIDList(pidl, szBuf);
		CoTaskMemFree(pidl);
	}
}
