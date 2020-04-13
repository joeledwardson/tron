#ifndef WINSYS_H
#define WINSYS_H

#include <assert.h>	 //assertions
#include <Windows.h> //win32 calls
#include <process.h> //threading
#pragma warning(disable: 4996)	//fopen and co unsafe
#pragma warning(disable: 4482)	//enum warning
#pragma warning(disable: 4355)	//using this in constructor
#define NAMESIZE		20

//structure for creating a thread. IMPORTANT: I probably massive overuse threads in this program, which is bad! I would advise you don't use them.
//"parent" refers to the caller of the thread function. "Child" refers to the calls made from inside a thread function
struct Thread_Params
{
	Thread_Params()
	{
		complete = false; //complete var. set to true by child when completed.
		
		exitFunc = false; /*exit var. set to true by parent when they wish for child to end function.
							they then wait for child to complete function and set complete to true. */
		
		inProgress = false; // set to true when function is in progress. used as only one thread should be created per thread_params struct.
	}
	void Parent_BeginFunc(void Func( void * vars ),void * vars)
	{
		assert( !inProgress ); //make sure thread is not already in progress.
		inProgress = true;		
		_beginthread( Func,0,vars);
	}
	void Parent_EndFunc()
	{
		assert( inProgress ); //make sure thread is in progress.
		assert( !exitFunc );	//make sure programmer has not already called for process to be terminated.
		exitFunc = true;		//set exitfunc to true so child knows to exit
		while( !complete ) {}	//wait for child to exit
		//reset
		complete = false;	
		exitFunc = false;
		inProgress = false;
	}
	bool Child_CheckForEnd()
	{
		return exitFunc; //checks if parent wishes for child to terminate.
	}
	void Child_EndFunc()
	{
		complete = true; //set complete to true. parent knows child has finished.
	}
private:
	bool exitFunc,complete,inProgress;
};

// **all bellow functions are declared in windows.cpp**
//message box function. exactly the same as MessageBox() except caller does not require access to the hwnd of the window. 
int DoMessageBox( wchar_t * message,  wchar_t * caption ,UINT32 type = MB_ICONINFORMATION );

//create input box. returns true if user enters value, false if cancels
bool CreateInputBox( char * returnString, wchar_t * caption, wchar_t * request, wchar_t * startingEditText );

//get edit box text. text is cleared from display upon input and stored (see windows.cpp), so this special function must be called to access text
void GetEditHWNDText( char * szString );

//register edit box for special procedures in windows.cpp.  when creating edit box, this should be called
void RegisterEditHWND( HWND hwnd );

//unregister edit box. notifying windows.cpp that box has been destroyed or is no longer in use.
void UnRegisterEditHWND();

//global function for beginning frame. set d3d to true if d3dgraphics buffer is to be cleared.
void BeginFrame( bool d3d );

//end frame. again, set d3d to true for d3dgraphics to be presented. optional pointer to buffer destination.
//(buffer source is the same as destination)
void EndFrame( bool d3d, RECT * dest = NULL);

//clear and present frame
void ClearFrame(RECT * dest = NULL);

//program states
enum ProgramState
{
	MENU,

	LOBBY ,
	
	PLAYING ,
	
	COUNTDOWN,

	
	
} ;

//menu states
enum MenuState
{
	mainmenu,
	nPlayerMenu,
	ServerList
};

#endif