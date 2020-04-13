#ifndef MASTERMENU_H
#define MASTERMENU_H


#include "GameNetworking.h"
#include "MenuBase.h"
#include "D3DGraphics.h"
#include <process.h>
#include "ServerConnection.h"
#include <algorithm>


#define MM_EXTRAHWND_X		MENU_BTN_XCOORD + MENU_BTN_WIDTH + 50

#define LOADICO_MENU_X		MM_EXTRAHWND_X + 50
#define LOADICO_MENU_Y		200	

//main menu button data
#define MM_BUTTONS	ButtonData(	MENU_BTN_XCOORD,								\
								MENU_BTN_YCOORD,								\
								MENU_BTN_WIDTH,									\
								MENU_BTN_HEIGHT,								\
								MENU_BTN_YDIS,									\
								MENU_NBUTTONS,									\
								L"play offline",		MENU_BTN_OFFLINE,		\
								L"server list",			MENU_BTN_SVRLIST,		\
								L"Host a game",			MENU_BTN_HOST,			\
								L"connect via ip",		MENU_BTN_CONNECTIP ,	\
								L"change name",			MENU_BTN_CHANGENAME ,	\
								L"Quit",				MENU_BTN_EXIT )		

//player menu button data
#define PM_BUTTONS	ButtonData( MENU_BTN_XCOORD,								\
								MENU_BTN_YCOORD,								\
								MENU_BTN_WIDTH,									\
								MENU_BTN_HEIGHT,								\
								MENU_BTN_YDIS,									\
								MENU_SUB_NBUTTONS,								\
								L"2 players",			MENU_SUB_2PLAYERS ,		\
								L"3 players",			MENU_SUB_3PLAYERS ,		\
								L"4 players",			MENU_SUB_4PLAYERS ,		\
								L"back to main menu",	MENU_SUB_RETURN )		
								


//class for menu. owns main menu and sub menu for selecting nPlayers
class MainMenu : public BaseMenu
{
public:
	MainMenu( MenuData& menuDat, char * initName);
	 ~MainMenu();
	
	//main function for running, returns program state. game specific data is put inside dat pointer on
	//game initliasion.
	void ChangeNameDisplay(char * szName );
	//creates window displaying host list, giving the option to connect
	
private:
	HWND extraPlayerHandle;
};

//master menu struct for master to interact with
class MasterMenu
{
public:
	MasterMenu(HWND window,ButtonClient& bRef,GameNetworking& net, char * rName, D3DGraphicsClient& rGfx);
	~MasterMenu();
	
	ProgramState Go(ReturnGameVals* dat);
private:
	ProgramState MenuButtonPressed( int buttonID, ReturnGameVals* dat);

	bool MenuInitHost( ReturnGameVals * dat );

	void InitPlayerMenu();

	void ChangeName();

	ProgramState ConnectViaIP(	ULONG IPAddress,ReturnGameVals* dat );

	//takes cTeamplte::name character string, formats it with "name: " to preceed it and convert to wchar array
	void UpdateNameDisplay(); 

	

	GameNetworking& networker;
	
	HWND hwnd;
	BaseMenu* currentMenu;
	ServerConnection * w;
	
	MenuState menuState; //state of menu
	char * name;
	
	MenuData mainMenuVars;
	MenuData playerMenuVars;
	D3DGraphicsClient& gfx;
};

#endif