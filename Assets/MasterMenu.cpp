#include "MasterMenu.h"



MainMenu::MainMenu( MenuData& menuDat, char * initName )
		:BaseMenu( menuDat )
{
	
	
	extraPlayerHandle =  CreateWindow(	L"static",
										L"",
										WS_CHILD | WS_VISIBLE | WS_BORDER | SS_EDITCONTROL,
										MM_EXTRAHWND_X,
										450 + MENU_BTN_HEIGHT/4,
										MENU_BTN_WIDTH+100,
										MENU_BTN_HEIGHT/2,
										hwnd,
										NULL,NULL,NULL);
		
	ChangeNameDisplay( initName );
	
};
MainMenu::~MainMenu()
{
	DestroyWindow( extraPlayerHandle );
}
void MainMenu::ChangeNameDisplay(char * szName )
{
	
	const int lenPreString = 6;

	wchar_t newNameDisplay[NAMESIZE+1 + lenPreString];
	ZeroMemory(newNameDisplay,(NAMESIZE+1 + lenPreString)*sizeof(wchar_t));

	memcpy( newNameDisplay,L"name: ",lenPreString * sizeof(wchar_t));

	wchar_t name_wchar[NAMESIZE+1];
	ZeroMemory(name_wchar,(1+NAMESIZE)*sizeof(wchar_t));
	mbstowcs( name_wchar,szName,strlen(szName));
	lstrcatW(newNameDisplay,name_wchar);
	
	SetWindowText(extraPlayerHandle,newNameDisplay);
}

MasterMenu::MasterMenu(HWND window,ButtonClient& bRef,GameNetworking& net, char * rName, D3DGraphicsClient& rGfx)
	:
	menuState( mainmenu ),
	networker(net),
	name(rName),
	hwnd(window),
	mainMenuVars(	bRef,window, MM_BUTTONS ),
	playerMenuVars( bRef,window, PM_BUTTONS ),
	gfx(rGfx),
	w(NULL)
{
	
	

	currentMenu = (BaseMenu*)new MainMenu( mainMenuVars,name);
}
MasterMenu::~MasterMenu()
{
	assert( menuState == MenuState::mainmenu || menuState == MenuState::nPlayerMenu || MenuState::ServerList );
	if( menuState == mainmenu )
	{
		delete (MainMenu*)currentMenu;

	}
	else if( menuState == MenuState::nPlayerMenu )
	{
		delete (BaseMenu*)currentMenu;
	}
	else if( menuState = MenuState::ServerList )
	{
		delete w;
	}
}
ProgramState MasterMenu::Go(ReturnGameVals* dat)
{
	int buttonPressed = 0;
	if( currentMenu )
	{
		buttonPressed = currentMenu->Go();
		if( buttonPressed > 0 ) //button pressed is -1 if no buttons pressed 0 or ave if preesed
		{
			//if state is menu
			if( menuState == mainmenu )
			{
				return MenuButtonPressed( buttonPressed,dat );
			
			}
			else if( menuState == nPlayerMenu )
			{
				if( buttonPressed == MENU_SUB_RETURN )
				{
					delete currentMenu;
					currentMenu = new MainMenu( mainMenuVars,name);
					menuState = mainmenu;
				}
				else
				{
					dat->type = GameType::OFFLINE;
					dat->nPlayers = buttonPressed+MENU_SUB_PLAYEROFF;
					return PLAYING;
				}
				
			}
		}
	}
	else
	{
		assert( w );
		ProgramState newState = w->Go( dat ,menuState);
		if( menuState == MenuState::mainmenu )
		{
			delete w;
			w = NULL;
			assert( !currentMenu );
			currentMenu = new MainMenu( mainMenuVars,name);
		}
		return newState;
	}
	
		
	return MENU;
}
ProgramState MasterMenu::MenuButtonPressed( int buttonID, ReturnGameVals* dat)
{
	char IPAddress[NAMESIZE+1];
	char * returnBuf;
	bool success;
	switch (buttonID)
	{
		case MENU_BTN_OFFLINE:
			InitPlayerMenu();
			break;
		case MENU_BTN_CHANGENAME:
			ChangeName();
			break;
		case MENU_BTN_SVRLIST:
			returnBuf = new char[1000];
			gfx.Begin_DrawLoadIco( LOADICO_MENU_X,LOADICO_MENU_Y );
			success =  networker.ConnectToServer( returnBuf );
			gfx.End_DrawLoadIco();
			if( success )
			{
				menuState = MenuState::ServerList;
				delete (MainMenu *) currentMenu;
				currentMenu = NULL;
				w = new ServerConnection( hwnd,gfx,name,mainMenuVars.buttonRef,returnBuf,networker); 
			}
			delete[] returnBuf;
			break;
		case MENU_BTN_HOST:
			if( MenuInitHost( dat ))
				return LOBBY;
			break;
		case MENU_BTN_CONNECTIP:
			if( CreateInputBox(IPAddress, L"connect via IP address",L"enter IP address\n to connect to",L"0.0.0.0"))
			{
				ULONG IP = inet_addr(IPAddress);
				int nDots = std::count( IPAddress, IPAddress + strlen(IPAddress), '.');
				if( IP == INADDR_NONE || IP == 0 || nDots != 3 )
					DoMessageBox( L"the ip address you entered is not valid",L"error");
				else
				{
					return ConnectViaIP( IP ,dat);	
				}
			}
			break;
	}
			
						
	
	return MENU;

}
void MasterMenu::InitPlayerMenu()
{
	delete (MainMenu*)currentMenu;
	currentMenu = new BaseMenu(playerMenuVars);
	menuState = MenuState::nPlayerMenu;
}
bool MasterMenu::MenuInitHost( ReturnGameVals * dat )
{
	bool success = false;
	char lobbyName[ NAMESIZE + 1];
	int tempMaxPlayers;
	char szTempMaxPlayers[NAMESIZE+1];
	if( CreateInputBox(lobbyName, L"lobby name",L"enter lobby name here",L"e.g. yoda's lobby"))
	{
		while(1)
		{
			if( CreateInputBox( szTempMaxPlayers,L"lobby settings",L"enter max players",L"4"))
			{

				tempMaxPlayers = atoi(szTempMaxPlayers ) ;
				if( tempMaxPlayers <= 1 )
				{
					MessageBox( hwnd,L"value has to be a number above 1",L"error",MB_ICONINFORMATION);
				}
				else
				{
					
					dat->type = GameType::HOST;
					dat->nPlayers = tempMaxPlayers;
					memcpy( dat->lobbyName,lobbyName,strlen(lobbyName)+1);
					bool piCon;
					dat->IPrecv = new char[100];
					gfx.Begin_DrawLoadIco( LOADICO_MENU_X,LOADICO_MENU_Y);
					success =  networker.InitHost( piCon,dat->IPrecv,1,tempMaxPlayers,lobbyName,LOBBY);
					gfx.End_DrawLoadIco();
					if( !piCon )
					{
						delete[] dat->IPrecv;
						dat->IPrecv = NULL;
					}
					break;
				}
					
			}
			else
			{
				break;
			}
		}
	}
	return success;
}
void MasterMenu::ChangeName()
{
	/*retrieve input from user to change name. if return value is not null (user did not cancel)
	set name to return value and set display text to value */
	char newName[NAMESIZE+1];
	if( CreateInputBox(newName, L"tron input",L"enter name",L"enter your name here"))
	{	
		memcpy( name,newName,strlen(newName)+1);
				
		MainMenu* s = (MainMenu*)currentMenu;
		s->ChangeNameDisplay( name );
					
	};
}
ProgramState MasterMenu::ConnectViaIP(ULONG IPAddress,ReturnGameVals* dat )
{
	
	dat->IPrecv = new char[JNP::PS_MAX];
			
	gfx.Begin_DrawLoadIco( LOADICO_MENU_X,LOADICO_MENU_Y);
	bool success = networker.ConnectToHost( IPAddress,name,dat->IPrecv );
	gfx.End_DrawLoadIco();
	if( success)
	{
		dat->type = GameType::CLIENT;
		return ProgramState::LOBBY;
	}
	else
	{
		networker.DestroyClient();
		return ProgramState::MENU;
	}
		
	
}