#ifndef LOBBY_H
#define LOBBY_H

#include "MenuBase.h"
#include <wchar.h>
#include <string>

#define MAX_CHAT_MSGLEN		LOB_MAX_INPUT +	NAMESIZE + 4

#define SOUND_FIRSTSOUND 1
#define SOUND_LASTSOUND 5
#define SOUND_LOCATION L".\\Sounds\\"
#define SOUND_LOC_LEN lstrlenW( SOUND_LOCATION )

#define LOBBY_ID const static int
#define LOBBY_NITEMS 5

#define LOBBY_XCOORD_LEFT	200
#define LOBBY_XCOORD_RIGHT	800
#define LOBBY_YCOORD_TOP	200
#define LOBBY_YCOORD_BOTTOM	

#define LOBBY_XCOORD_CHAT	450
#define LOBBY_CHAT_WIDTH	300
#define LOBBY_CHATOT_HEIGHT	260
#define LOBBY_CHATIN_YCOORD	500

#define LOBBY_STD_WIDTH		200
#define LOBBY_STD_HEIGHT	50

#define LOBBY_TITLE_YCOORD	100
#define LOBBY_TITLE_WIDTH	800


#define LOBBY_PL_HEIGHT		360

#define LOBBY_LD_YCOORD		500

#define LOB_MAX_INPUT		40
/*create a button with with set x coord width and height (see definitions) and assigns
the handle the place in the buttonHandles array given by the identifier param */
//HWND CreateButton( HWND parent,wchar_t * displayString, int xCoord,int yCoord,  int identifier, int bWidth, int bHeight );

class LobbyInt
{
private:
	
public:
	void ExtendChat( char * szMessage );
	
	void SetPlayerDat( int nPlayers, int maxPlayers );

	LobbyInt(MenuData& lobbyDat );
	
	void SetLobbyName( char * lobbyName);

	void AddPlayer( const char * playerName  );

	void RemovePlayer( UINT index );

	void GetPlrName( UINT index,char szName[NAMESIZE+1] );

	int GetSelPlayer();

	int Go();

	~LobbyInt();
protected:
	
	HWND windowHandles[LOBBY_NITEMS];
	BaseMenu menu;
	
	std::wstring editString;

	LOBBY_ID playerList = 1;
	LOBBY_ID lobbyData = 2;
	LOBBY_ID lobbyTitle = 0;
	LOBBY_ID chatDisplay= 3;
	LOBBY_ID chatEdit = 4;
};

#endif