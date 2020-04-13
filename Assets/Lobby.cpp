#include "Lobby.h"

LobbyInt::LobbyInt(MenuData& lobbyDat )
	:
	menu( lobbyDat )

{
	
	windowHandles[playerList] = CreateWindow(	L"listbox",
												L"player list",
												WS_VISIBLE | WS_CHILD | WS_BORDER,
												LOBBY_XCOORD_LEFT,
												LOBBY_YCOORD_TOP,
												LOBBY_STD_WIDTH,
												LOBBY_PL_HEIGHT,
												lobbyDat.window,
												(HMENU)LOBBY_LB_NAMES,
												NULL,NULL);	

	windowHandles[lobbyTitle] = CreateWindow(	L"static",
												NULL,
												WS_CHILD | WS_VISIBLE | WS_BORDER | SS_EDITCONTROL,
												LOBBY_XCOORD_LEFT,
												LOBBY_TITLE_YCOORD,
												LOBBY_TITLE_WIDTH,
												LOBBY_STD_HEIGHT,
												lobbyDat.window,
												(HMENU)LOBBY_TITLE,
												NULL,NULL);

	windowHandles[lobbyData] = CreateWindow(	L"static",
												NULL,
												WS_CHILD |WS_VISIBLE | WS_BORDER | SS_EDITCONTROL,
												LOBBY_XCOORD_RIGHT,
												LOBBY_LD_YCOORD,
												LOBBY_STD_WIDTH,
												LOBBY_STD_HEIGHT,
												lobbyDat.window,
												(HMENU)LOBBY_DATA,
												NULL,NULL);

	windowHandles[chatDisplay] = CreateWindow(	L"edit",
												NULL,
												WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL | ES_READONLY,
												LOBBY_XCOORD_CHAT,
												LOBBY_YCOORD_TOP,
												LOBBY_CHAT_WIDTH,
												LOBBY_CHATOT_HEIGHT,
												lobbyDat.window,
												(HMENU)LOBBY_CHAT_DISP,
												NULL,NULL);

	windowHandles[chatEdit] = 	CreateWindow(	L"edit",
												NULL,
												WS_BORDER | WS_CHILD | WS_VISIBLE | ES_MULTILINE,
												LOBBY_XCOORD_CHAT,
												LOBBY_CHATIN_YCOORD,
												LOBBY_CHAT_WIDTH,
												LOBBY_STD_HEIGHT,
												lobbyDat.window,
												(HMENU)LOBBY_CHAT_EDIT,
												NULL,NULL);

	

	SendMessage( windowHandles[chatEdit],EM_LIMITTEXT,(WPARAM)LOB_MAX_INPUT,NULL);
	RegisterEditHWND( windowHandles[chatEdit] );
	SetPlayerDat(1,4);
	ExtendChat("     Lobby Chat     ");
	ExtendChat("********************");
	ExtendChat("");
}
LobbyInt::~LobbyInt()
{
	UnRegisterEditHWND(  );
	for( int index = 0; index < LOBBY_NITEMS ; index++ )
	{
		DestroyWindow( windowHandles[index]);
	}

}
void LobbyInt::ExtendChat( char * szMessage )
{
	
	assert( strlen(szMessage) <= MAX_CHAT_MSGLEN );
	wchar_t wideSzMessage[MAX_CHAT_MSGLEN];
	mbstowcs(wideSzMessage,szMessage,MAX_CHAT_MSGLEN);
	editString.append( wideSzMessage );
	editString.append( L"\r\n" );
		
	SendMessage( windowHandles[chatDisplay],WM_SETTEXT,NULL,(LPARAM)editString.c_str());
	
	SendMessage( windowHandles[chatDisplay],EM_SCROLL,(WPARAM)SB_PAGEDOWN,NULL);
	
}
void LobbyInt::SetPlayerDat( int nPlayers, int maxPlayers )
{
	wchar_t * plyrBuf = new wchar_t[255];
	ZeroMemory( plyrBuf,255 );
		
	swprintf( plyrBuf,L"No of players: %d\nMax Players: %d",nPlayers,maxPlayers );
	SetWindowText( windowHandles[lobbyData],plyrBuf );
	delete[] plyrBuf;
}
void LobbyInt::SetLobbyName( char * lobbyName)
{
	wchar_t * wLobbyName = new wchar_t[NAMESIZE+1];
	ZeroMemory(wLobbyName,NAMESIZE+1);
	mbstowcs( wLobbyName,lobbyName,sizeof(wchar_t)*strlen(lobbyName));
	SetWindowText( windowHandles[lobbyTitle],wLobbyName);
	delete[] wLobbyName;
}
void LobbyInt::AddPlayer( const char * playerName  )
{
	wchar_t name_wchar[NAMESIZE+1];
	ZeroMemory( name_wchar,NAMESIZE+1);
	int len = strlen(playerName)+1;
		
	mbstowcs( name_wchar,playerName,len);
	
	SendMessage( windowHandles[playerList],LB_ADDSTRING,NULL,(LPARAM)name_wchar);


}
void LobbyInt::RemovePlayer( UINT index )
{
	//int len = strlen(playerName)+1;
	//wchar_t * name_wchar = new wchar_t[NAMESIZE+1];
	//ZeroMemory( name_wchar,20);
	//mbstowcs( name_wchar,playerName,len);
	//int i = SendMessage( windowHandles[playerList],LB_FINDSTRING,-1,(LPARAM)name_wchar);
	///*wchar_t * tmpBuf = new wchar_t[NAMESIZE+1];
	//	
	//int listItems = SendMessage( windowHandles[playerList],LB_GETCOUNT,NULL,NULL );
	//for( int listIndex = 0; listIndex < listItems; listIndex++ )
	//{
	//	ZeroMemory( tmpBuf,NAMESIZE+1 );
	//	SendMessage( windowHandles[playerList],LB_GETTEXT,listIndex,(LPARAM)tmpBuf );
	//	if( !(lstrcmpW( tmpBuf,name_wchar)))
	//	{
	//		
	//		SendMessage( windowHandles[playerList],LB_DELETESTRING,(WPARAM)(listIndex),NULL);
	//		break;
	//	}
	//}*/
	//if( i >= 0 )
	SendMessage( windowHandles[playerList],LB_DELETESTRING,(WPARAM)index,NULL);
	/*delete name_wchar;*/
	/*delete tmpBuf;*/
	
}
int LobbyInt::Go()
{
	return menu.Go();
}
int LobbyInt::GetSelPlayer()
{
	return SendMessage( windowHandles[playerList],LB_GETCURSEL,WPARAM(0),LPARAM(0));
}
void LobbyInt::GetPlrName( UINT index,char szName[NAMESIZE+1] )
{
	wchar_t wszName[NAMESIZE+1];
	SendMessage( windowHandles[playerList],LB_GETTEXT,index,(LPARAM)wszName);
	wcstombs( szName, wszName, NAMESIZE + 1 );
	
}
