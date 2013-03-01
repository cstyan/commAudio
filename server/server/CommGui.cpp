#include "CommGui.h"

CommGui::CommGui()
{
	initLayout(layout);
}

void CommGui::initLayout(CommGuiLayout &layout)
{
	// Initialize layout variables for the GUI.
	layout.bgColor = RGB(100, 100, 100); // Used for the background color of the main window.
	layout.windowWidth = 500; // Used for the width of the window.
	layout.windowHeight = 600; // Used for the height of the window.
	layout.songListX = 250;
	layout.songListY = 25;
	layout.songListWidth = 225;
	layout.songListHeight = 500;
}