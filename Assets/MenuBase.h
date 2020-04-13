#ifndef MENUBASE_H
#define MENUBASE_H



#include <assert.h>
#include <Windows.h>
#include "WinSys.h"
/*	definitions for menu buttons. used for identifying buttons
	note that these definitions also correspond to place in 
	menuHandles array
*/

/*identifiers for main menu buttons */
#define MENU_BTN_OFFLINE	1
#define MENU_BTN_SVRLIST	2
#define MENU_BTN_HOST		3
#define MENU_BTN_CONNECTIP	4
#define MENU_BTN_CHANGENAME	5
#define MENU_BTN_EXIT		6
/****/
#define MENU_NBUTTONS		6


/* identifiers for submenu (nplayers select) */

#define MENU_SUB_2PLAYERS	7
#define MENU_SUB_3PLAYERS	8
#define MENU_SUB_4PLAYERS	9
#define MENU_SUB_RETURN		10

#define MENU_SUB_PLAYEROFF	2 - MENU_SUB_2PLAYERS /*number to add to identifier for nPlayers
when a number of players is selected in menu. E.G 2 players is selected (MENU_SUB_2PLAYERS)
no of players = 6 - 4 = 2; */
/***/
#define MENU_SUB_NBUTTONS	4

#define LOBBY_LB_NAMES		11
#define LOBBY_QUIT			12
#define LOBBY_TITLE			13
#define LOBBY_DATA			14
#define LOBBY_CHAT_DISP		17
#define LOBBY_CHAT_EDIT		18

#define LOBBY_HOST_BOOT		15
#define LOBBY_HOST_START	16

#define LOBBY_HOST_NBUTTONS 3
#define LOBBY_CLIENT_NBUTTONS 1

/*** server window connection buttons */
#define SVRWND_CONNECT		19
#define SVRWND_CANCEL		20
#define SVRWND_HOSTLIST		21
#define SVRWND_HOSTDATA		22

/* button sizes and coordinates */
#define MENU_BTN_WIDTH		200
#define MENU_BTN_HEIGHT		50
#define MENU_BTN_XCOORD		100
#define MENU_BTN_YCOORD		50
#define MENU_BTN_YDIS		100
/****/

struct ButtonClient;

//sctruct for windows.cpp to interact with when user presses buttons
struct ButtonServer
{
	friend ButtonClient;
private:
	int buttonPressed;
public:
	void NoButtonsPressed();
	ButtonServer();
	void SetButtonPressed( int bPressed );
};

struct ButtonClient
{
	
public:
	
	//returns 0 if no buttons pressed. otherwise returns identifier
	int GetButtonPressed();
		

	ButtonClient( ButtonServer& b );

private:
	ButtonServer& bServ;
};


enum GameType
{
	HOST,CLIENT,OFFLINE
};


struct ReturnGameVals
{
	GameType type;

	ReturnGameVals();
	
	/* offline */
	int nPlayers;
	/*********/
	/** host and client **/
	char lobbyName[NAMESIZE + 1];
	/*** client only ***/
	sockaddr ipDat;
	char * IPrecv; //recvbuffer from host

		
	~ReturnGameVals();

};

struct ButtonData
{

	ButtonData(	int menuXCoord,int menuYCoord, int menuButtonWidth,
				int menuButtonHeight, int menuYButtonDis, int nOfButtons, ...);
	ButtonData( const ButtonData& );
	
	struct Button
	{
		Button( wchar_t * display, int buttonIdentifier );
		wchar_t * wszDisplay;
		int identifier;
	};

	int xCoord, yStart, buttonWidth, buttonHeight, yButtonDis,nButtons;
	Button * buttonList; //buttons in order with top one starting first

	~ButtonData();
};
struct MenuData
{
	
	MenuData( ButtonClient& bRef , HWND hwnd, ButtonData& buttonData );
	~MenuData();
	ButtonClient& buttonRef;
	HWND window;
	ButtonData buttonDat;
};

//template class for menu
class BaseMenu
{
public:
	BaseMenu(MenuData& menuDat );

	~BaseMenu();
	
	int Go();
protected:
	HWND CreateMenuButton( ButtonData& b , int buttonNumber);

	ButtonClient& buttonRef;
	int nButtons;
	HWND hwnd;		//handle for main window. initiliased in consctructor
	HWND * buttonHandles; //pointer to a list of handles for buttons of sub windows for main and sub menu
	
};

#endif