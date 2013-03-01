/*
 *	SOURCE FILE: addconsole.h
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
#ifndef _CA_ALIAO_ADDCONSOLE_H
#define _CA_ALIAO_ADDCONSOLE_H

class AddConsole
{
public:
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
	static void addConsole();
};

#endif