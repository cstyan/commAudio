#include <windows.h>
#include <string>

#include "client.h"
#include "resource.h"

int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                       LPSTR lpszCmdLine, int nCmdShow)
{
    HWND        hWnd ;      /* the window's "handle" */
    MSG         msg ;       /* a message structure */
    WNDCLASS    wndclass ;  /* window class structure */

        wndclass.style          = CS_HREDRAW | CS_VREDRAW ;
        wndclass.lpfnWndProc    = WndProc ;
        wndclass.cbClsExtra     = 0 ;
        wndclass.cbWndExtra     = 0 ;
        wndclass.hInstance      = hInstance ;
        wndclass.hIcon          = NULL ;
        wndclass.hCursor        = LoadCursor (NULL, IDC_ARROW) ;
        wndclass.hbrBackground  = (HBRUSH)GetStockObject (WHITE_BRUSH) ;
        wndclass.lpszMenuName   = MAKEINTRESOURCE(IDR_MENU1);
        wndclass.lpszClassName  = TEXT("MyClass");
        if (!RegisterClass (&wndclass))
            return 0 ;

    hWnd = CreateWindow (TEXT("MyClass"), TEXT("Comm Audio Client"), 
        WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400, 
        NULL, NULL, hInstance, NULL) ;
	
    ShowWindow (hWnd, nCmdShow) ;    
    UpdateWindow(hWnd) ;

    while (GetMessage (&msg, NULL, 0, 0)) 
    {
        TranslateMessage (&msg) ; 
        DispatchMessage (&msg) ;  
    }
    return (msg.wParam) ;
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT wMessage, 
                           WPARAM wParam, LPARAM lParam)
{
    static  HWND    hPlayBtn, hPauseBtn, hSongDisplay;
    HINSTANCE       hInstance ;
    BOOL            bIsChecked, bTopRadioOn ; /* button status variables */
    char            cBuf [128] ;
	std::string			sBuf;
    
    switch (wMessage)       /* process windows messages */
    {
        case WM_CREATE:     /* program is just starting */
			/* get the instance handle */
            hInstance = (HINSTANCE)GetWindowLong (hWnd, GWL_HINSTANCE) ; 

			/* create button controls */
            hPlayBtn = CreateWindow (TEXT("BUTTON"), TEXT("Play"),
                    WS_CHILD | BS_DEFPUSHBUTTON,
                    140, 250, 100, 40,
                    hWnd, (HMENU)PLAY_BUTTON, hInstance, NULL) ;

			hPauseBtn = CreateWindow(TEXT("BUTTON"), TEXT("Pause"),
					WS_CHILD | BS_DEFPUSHBUTTON,
					260, 250, 100, 40,
					hWnd, (HMENU)PAUSE_BUTTON, hInstance, NULL) ;

			hSongDisplay = CreateWindow(TEXT("LISTBOX"), TEXT("Song List"),
					WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | WS_CAPTION,
					30, 30, 425, 200,
					hWnd, (HMENU)SONG_DISPLAY, hInstance, NULL);
			SendMessage(hSongDisplay, EM_SETREADONLY, TRUE, NULL);

			/* display the buttons */
            ShowWindow (hPlayBtn, SW_SHOWNORMAL) ;
            ShowWindow (hPauseBtn, SW_SHOWNORMAL) ;
            ShowWindow (hSongDisplay, SW_SHOWNORMAL) ;
			
			SendMessage(hSongDisplay, LB_ADDSTRING, 0, (LPARAM(TEXT("hi"))));
            break ;

        case WM_COMMAND:
            switch (LOWORD(wParam)) /* check Button control ID */
            {
                case PLAY_BUTTON:
					SendMessage(hSongDisplay, LB_ADDSTRING, 0, (LPARAM(TEXT("hi"))));
                    break ; 

				case PAUSE_BUTTON:
					SendMessage(hSongDisplay, LB_ADDSTRING, 0, (LPARAM(TEXT("bye"))));
					break ;

				case ID_MIC:
					break;

				case ID_FILE_EXIT:
                    DestroyWindow (hWnd) ;  /* destroy window */
                    break ;     /* terminating application */
            }
            break ;

        case WM_DESTROY:    /* stop application */
            PostQuitMessage (0) ;
            break ;

        default:        /* default windows message processing */
            return DefWindowProc (hWnd, wMessage, wParam, lParam) ;
    }
    return (0) ;
}

