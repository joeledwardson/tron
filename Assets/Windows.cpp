/****************************************************************************************** 
 *	Chili DirectX Framework Version 12.04.24											  *	
 *	Windows.cpp																			  *
 *	Copyright 2012 PlanetChili.net														  *
 *																						  *
 *	This file is part of The Chili DirectX Framework.									  *
 *																						  *
 *	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
 *	it under the terms of the GNU General Public License as published by				  *
 *	the Free Software Foundation, either version 3 of the License, or					  *
 *	(at your option) any later version.													  *
 *																						  *
 *	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
 *	GNU General Public License for more details.										  *
 *																						  *
 *	You should have received a copy of the GNU General Public License					  *
 *	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
 ******************************************************************************************/
#include <wchar.h>
#include "Master.h"
#include "resource.h"
#include "WinSys.h"

/* visual leak detector. This checks for memory leaks.
To enable, install visual leak detector. Link: https://vld.codeplex.com/releases
Find the files for visual leak detector, usually this will be something like c:\program files\visual leak detector\
Go into project->properties->vc++ directories.
Go into include directories and add a new line with the path set to the include directory within visual leak detector folder.
Go into library directories and add a new line with the path set to the library directory within visual leak detector folder.
run regsvr32 on the dll in the VLD folder.
un-comment the #define VLD line. */
//#define VLD

#ifdef VLD
#include <vld.h>
#pragma comment( lib, "vld.lib" )
#endif

/*manifest linking. not entirely sure what this does, but it is required to get common controls to work,
so that the win32 buttons and other stuff can look fancy and not plain old windows style. */
#pragma comment(linker,	"\"/manifestdependency:type='win32' \
						name='Microsoft.Windows.Common-Controls' \
						version='6.0.0.0' \
						processorArchitecture='*' \
						publicKeyToken='6595b64144ccf1df' \
						language='*'\"")							
#include <CommCtrl.h>


KeyboardServer kServ;
ButtonServer bServ;



D3DGraphics * g; 		/*pointer to d3dgraphics class, so it can be accessed in beginframe and endframe functions. object cannot be 
						initialised here as it requires the hwnd of the window which is created in main().*/

MSG msg;				//msg object. again, declared globally so it can be accessed in functions.
bool exitProg;
HWND hWnd;
HWND hWndEdit = NULL;
HINSTANCE hInst;
LPWSTR gCaption,gRequest,gStartingEditText = NULL;
wchar_t gReturnString[NAMESIZE+1];

char szEditInput[LOB_MAX_INPUT+1];


INT_PTR CALLBACK InputBoxProc(  HWND hwndDlg, UINT32 uMsg, WPARAM wParam,LPARAM lParam )
{
	switch (uMsg) 
	{
		
		case WM_INITDIALOG:
			SetWindowText( hwndDlg,gCaption);							//set title
			SetDlgItemText( hwndDlg,ID_IB_EDITTEXT,gStartingEditText);				//set starting edit text
			SetDlgItemText( hwndDlg,ID_IB_DISPLAYTEXT,gRequest);					//set request string
			SendMessage( GetDlgItem( hwndDlg,ID_IB_EDITTEXT),EM_LIMITTEXT,(WPARAM)NAMESIZE,NULL);	//limit edit text
			return true;
		case WM_COMMAND: 
			switch (LOWORD(wParam)) 
			{ 
				
				case IDOK:
					GetDlgItemText(hwndDlg,ID_IB_EDITTEXT,gReturnString,21);			//get edit text
					//dont want blank input
					if( gReturnString[0] == NULL )
					{
						MessageBox( hwndDlg,L"you need to enter something!",L"error",NULL);
					}
					else
					{
						EndDialog( hwndDlg,1);							//end dialog
					}
					return TRUE; 
					
				case IDCANCEL:
					ZeroMemory( gReturnString,21);							//string is zero-memoried so the programmer knows the user cancelled.
					EndDialog(hwndDlg,0);
					return true;
			}
    } 
    return FALSE; 
}

LRESULT WINAPI MsgProc( HWND hWnd, UINT32 msg, WPARAM wParam, LPARAM lParam )
{
	
	
    switch( msg )
    {

		
		

		case WM_COMMAND:
			{
				//when user presses button, button server (bserv) is updated.

				int identifier = LOWORD( wParam ); //get button ID.
				
				

				if( wParam == MENU_BTN_EXIT )
					PostQuitMessage( 0 );
				if( identifier != LOBBY_CHAT_EDIT )
					bServ.SetButtonPressed( identifier );
				break;

			}

				

        case WM_DESTROY:
			
            PostQuitMessage( 0 );
            break;

		// ************ KEYBOARD MESSAGES ************ //
		
		case WM_KEYDOWN:
			//if key is in range, set key pressed			
			if( wParam >= 0 && wParam <= NKEYS )
			{
				kServ.OnKeyPressed( wParam );
				
			}
			
			
			break;
		case WM_KEYUP:
   			
			if( wParam >= 0 && wParam <= NKEYS )
			{
				kServ.OnKeyReleased( wParam );
			}
			
			break;
		// ************ END KEYBOARD MESSAGES ************ //

		
    }

	

    return DefWindowProc( hWnd, msg, wParam, lParam );
}

void BeginFrame(bool d3d)
{
	
		while( PeekMessage( &msg,NULL,0,0,PM_REMOVE ) )
		{
			/* special case. if user presses enter while in edit box, if the edit box is on its last line win32 will
			bleep as there is no room to create a new line in the box. Therefore this must be processed before it
			enters the message procedure. */		
	
			if( msg.message == WM_CHAR &&		//key pressed
				msg.hwnd == hWndEdit &&			//hwnd is hwnd of editbox
				msg.wParam == VK_RETURN )		//key pressed is return key
			{
				WCHAR tempBuf[LOB_MAX_INPUT+1];										//temp wide char array
				SendMessage( hWndEdit,WM_GETTEXT,LOB_MAX_INPUT+1,(LPARAM)tempBuf );	//get message
				bServ.SetButtonPressed( LOBBY_CHAT_EDIT);							//message is not processed, so bserv is not updated in msgproc. therefore button pressed must be set.
				wcstombs( szEditInput,tempBuf,LOB_MAX_INPUT+1);						//move wide char string to lobby input string.
				SendMessage( hWndEdit,WM_SETTEXT,NULL,(LPARAM)L"");					//clear edit box display.
			}
			else
			{
				if( msg.message == WM_QUIT )
				{
					exitProg = true; //exit variable set to true. main() will terminate when ready.
					return;
				}
				else
				{
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}
			}
		}
		if( d3d )
		{
			g->ClearGraphicsFrame(); //clears frame if d3d enabled.
		}
		
	
}

void EndFrame(bool d3d, RECT * dest)
{
	/* d3dgraphics is not required for some parts of the program. if so, d3d is set to false and dest ignored.
	In which case, the d3d graphics buffer will not be presented to the screen. */

	//when endframe is called, master.go() will have been called. button server is set so no button is pressed.
	//Otherwise button pressed would remain the same until another button was pressed.
	kServ.Update();			//update keyboard server
	bServ.NoButtonsPressed(); 	//initilase menuserv so no buttons are pressed
	if( d3d )
	{
		g->EndGraphicsFrame( dest ); //update d3dgraphics if required.
	}
}

void ClearFrame(RECT * dest)
{
	//wipe graphics buffer and present it
	g->ClearGraphicsFrame();
	g->EndGraphicsFrame(dest);
}

int WINAPI wWinMain( HINSTANCE hInstance,HINSTANCE prevInstace,LPWSTR,INT )
{
	
	
	hInst = hInstance;

	InitCommonControls(); //initialise common controls so that you see fancy buttons ;)

	WNDCLASSEX wc = { sizeof( WNDCLASSEX ),CS_CLASSDC,MsgProc,0,0,
                      GetModuleHandle( NULL ),NULL,NULL,(HBRUSH)(COLOR_WINDOW+1),NULL,
                      L"Chili DirectX Framework Window",NULL };

    
	wc.hIcon   = (HICON)LoadImage( hInst,MAKEINTRESOURCE( MYICO_BIG ),IMAGE_ICON,256,256,0 );
	wc.hIconSm = (HICON)LoadImage( hInst,MAKEINTRESOURCE( MYICO_SMALL ),IMAGE_ICON,32,32,0);
	
	wc.hCursor = LoadCursor( NULL,IDC_ARROW );
	assert( RegisterClassEx( &wc ));
	
	RECT wr;
	wr.left = 650;
	wr.right = SCREEN_WIDTH + wr.left;
	wr.top = 150;
	wr.bottom = SCREEN_HEIGHT + wr.top;
	AdjustWindowRect( &wr,WS_OVERLAPPEDWINDOW,FALSE );
	hWnd = CreateWindowW( L"Chili DirectX Framework Window",L"TRON",
                              WS_OVERLAPPEDWINDOW,wr.left,wr.top,wr.right-wr.left,wr.bottom-wr.top,
                              NULL,NULL,wc.hInstance,NULL );
	
	assert(hWnd);
    ShowWindow( hWnd,SW_SHOWDEFAULT );
    UpdateWindow( hWnd );
	
	D3DGraphics rGfx( hWnd );
	g = &rGfx; //set d3dgraphics pointer to actual d3dgraphics object

	Master theMaster( hWnd,kServ,bServ,rGfx );
	
	exitProg = false; //dont exit the program yet!
     
	ZeroMemory( &msg,sizeof(msg));
   
	while(!exitProg)
	{
		
		theMaster.Go();

		
	}

	DestroyWindow( hWnd );
    
    assert( UnregisterClass( L"Chili DirectX Framework Window",wc.hInstance ));
	
    return 0;
}

int DoMessageBox( wchar_t * message , wchar_t * caption, UINT32 type )
{
	return MessageBox( hWnd,message,caption,type );
}

void RegisterEditHWND( HWND hwnd )
{
	assert( !hWndEdit );
	hWndEdit = hwnd;
	
}

void UnRegisterEditHWND()
{
	assert( hWndEdit );
	hWndEdit = NULL;
}

void GetEditHWNDText( char * szString )
{
	memcpy( szString,szEditInput,strlen(szEditInput)+1);
}

//creates an input box to enter text into. returns input value. If cancelled by user, returns NULL
bool CreateInputBox( char * returnString,  wchar_t * caption, wchar_t * request, wchar_t * startingEditText )
{
	/* inputboxproc can only access global vars, so global variables are set the the parameters below, so that
	when the input box is created and WM_INITDIALOG is called it will refer to the correct variables. */
	gCaption = caption;
	gRequest = request;
	gStartingEditText = startingEditText;

	//returns true if user enters something, false if cancels. See InputBoxProc
	int rVal = DialogBox( hInst,MAKEINTRESOURCE( ID_IB_WINDOW ),hWnd,(DLGPROC)InputBoxProc ); //create the dialog
	
	//returns true if user pressed ok and entered something, false if they cancelled.
	if( rVal )
	{
		wcstombs( returnString,gReturnString,21);
	}
	return rVal!=0;
	
}