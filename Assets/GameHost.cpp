#include "Game.h"

void GameMaster_Host::DeleteInstance(	ProgramState	state,
										void*			pLobby,
										void*			pGame,
										void*			pCountDown )
{
	switch (state)
	{
		case LOBBY:
			delete (GameMaster_Host::Lobby_Host*)(pLobby);
			break;
		case PLAYING:
			delete (GameMaster_Host::InGame_Host*)pGame;
			break;
		case COUNTDOWN:
			delete (GameMaster_Host::CountDown_Host*)pCountDown;
			break;
	}
}
GameMaster_Host::GameMaster_Host(int maxPlayers,
						ButtonClient& buttonRef,
						HWND hwnd,
						char * lobbyInitName,
						baseClass& bRef,
						char *nameRef,
						GameNetworking& networker,
						char * piReply)
	:
	
	vars( bRef,networker,maxPlayers,nameRef),
	lobbyDat( buttonRef , hwnd ,ButtonData( LOBBY_XCOORD_RIGHT,						
											LOBBY_YCOORD_TOP,						
											MENU_BTN_WIDTH,							
											MENU_BTN_HEIGHT,						
											MENU_BTN_YDIS,							
											LOBBY_HOST_NBUTTONS,					
											L"Quit",			LOBBY_QUIT,			
											L"Boot player",		LOBBY_HOST_BOOT,	
											L"Start game",		LOBBY_HOST_START))
{
	currentState = ProgramState::LOBBY;

	vars.nPlayers = 1;

	
	
	
	memcpy(vars.lobbyName,lobbyInitName,strlen(lobbyInitName)+1);	//copy lobby name passed in parameters to local var

	

	ClearFrame();	//clear graphics grame
		

	//add host (yourself) to playerlist. must set playerlist pointer to relevant pointer though
	vars.hPlayerList.push_back( Player( vars.name));
	vars.playerList = &vars.hPlayerList.at(0);
	
	

	lobby =  new Lobby_Host( lobbyDat, vars);

	/* pi reply is the buffer which was sent by the master server in reply to the host registration.
	if it failed, pireply is NULL. if true, add contact for master server*/
	if( piReply )
	{
		vars.connectedToServer = true;
		vars.gameNet.AddContact_Client( IPComm_Host::serverListID,piReply,JNP::SOCK_SVR );
	}
	else
	{
		vars.connectedToServer = false;
		DoMessageBox( L"not connected to master server",L"error");
	}
		
};
GameMaster_Host::~GameMaster_Host()
{
	vars.SendQuit();				//tell clients and master server that game has closed

	vars.netBase.DestroyHost();		//close networking object

	DeleteInstance( currentState,lobby,game,countdown);	//delete current instance
	
}
ProgramState GameMaster_Host::Go()
{
	//( see GameMaster_Client::Go for explanation on switching current state and running instances)

	ProgramState newState;

	assert( currentState != ProgramState::MENU );
	
	switch (currentState)
	{
	case LOBBY:
		newState = lobby->LobbyGo();
		break;
	case COUNTDOWN:
		newState = countdown->Go();
		break;
	case PLAYING:
		newState = game->Go();
		break;
	}

	if( newState != currentState && newState != ProgramState::MENU)
	{
		switch (currentState)
		{
		case LOBBY:
			delete lobby;
			break;
		case PLAYING:
			delete game;
			break;
		case COUNTDOWN:
			delete countdown;
			break;
		}

		switch (newState)
		{
		case LOBBY:
			ClearFrame();
			lobby = new Lobby_Host( lobbyDat,vars );

			break;
		case PLAYING:
			game = new InGame_Host( vars );
			break;
		case COUNTDOWN:
			countdown = new CountDown_Host( STARTINGSECS,vars );
			break;
		default:
			break;
		}
	}
	vars.gameNet.SendData();
	if( newState == ProgramState::MENU )
		return ProgramState::MENU;
	else
	{
		currentState = newState;
		return currentState;
	}


}


GameMaster_Host::GameVars_Host::GameVars_Host(baseClass& bRef, GameNetworking& network, int noOfMaxPlayers,char*name)
	:GameVars_Online( bRef,network, name ),
	gameNet( network  )
{
	
	maxPlayers = noOfMaxPlayers;
}
GameMaster_Host::GameVars_Host::~GameVars_Host()
{
		
}
void GameMaster_Host::GameVars_Host::SendPlayerReply(  sockaddr* IPDat,byte cmReplyType, int pnPacketNo )
{
	
	const int len = 4;
	char sendSuccess[len];
	ZeroMemory( sendSuccess,len );
	sendSuccess[JNP::BYTE_CM] = cmReplyType;
	sendSuccess[JNP::BYTE_PN] = pnPacketNo;
	netBase.SendData( JNP::SOCK_HOST,len,sendSuccess,IPDat );
	
	
}
void GameMaster_Host::GameVars_Host::SendQuit()
{
	const int size = JNP::BYTE_CM + 1;
	char sendingBuffer[size];
	sendingBuffer[ JNP::BYTE_CM ] = JNP::CM_QUIT;
	for( int index = 1; index < nPlayers; index++ )
	{
		gameNet.SendPacket(IPComm_Host::clientListID ,index - 1,sendingBuffer,size);
	}
	if( connectedToServer )
		gameNet.SendPacket( IPComm_Host::serverListID,0,sendingBuffer,size);
}
void GameMaster_Host::GameVars_Host::KickPlayer( int playerIndex )
{
	const int size = JNP::BYTE_CM + 1;
	char sendingBuffer[size];
	sendingBuffer[JNP::BYTE_CM] = JNP::CM_KICK;
	gameNet.SendPacket( IPComm_Host::clientListID ,playerIndex-1,sendingBuffer,size);
}
int GameMaster_Host::GameVars_Host::RecvData( int& playerIndex,sockaddr * pRecvIPData  )
{
	if( connectedToServer )
		while( gameNet.RecvServerData( rcvBuf )) {}

	ZeroMemory( rcvBuf,JNP::PS_CH_CM_C);
	int returnVal = gameNet.RecvClientData( rcvBuf,playerIndex,pRecvIPData);

	if(returnVal <= 0 ) //if no data recieved or data already processed exit function
	{
		return returnVal;
	}

	if( playerIndex < 0 )
	{
		/* if playerIndex < 0, i.e. source is not recognised as from a client:
			if it is a request pass on buffer by returning length,
			otherwise ignore by returning -1*/
		if( rcvBuf[ JNP::BYTE_CM ] == JNP::CM_NORMREQEST )
		{
			return returnVal;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		/*if playerindex >= 0, i.e. source is from a client increment player index by 1.
		networking list of clients starts with first client. vars.playerlist
		is stored with the first index to be the host, so to get the index to refer to the revelant
		client it must be incremented by 1*/
		playerIndex++;
	}
	return returnVal;		
}



GameMaster_Host::CountDown_Host::CountDown_Host( int startingSecs,GameVars_Host& rVars )
	:CountDown_Base(startingSecs,rVars.baseRef),
	vars(rVars)
{
	/*update msg. this is sent to clients and master server. update is single byte is programstate for new
	state, (being programstate::Countdown*/
	char szUpdate  = ProgramState::COUNTDOWN;
	vars.gameNet.AddUpdate( JNP::UPDATE_TYPES::CHANGE_STATE,
		&szUpdate,
		1,
		false,
		JNP::HOST_DEST_CLIENTS | JNP::HOST_DEST_SERVER );
}
GameMaster_Host::CountDown_Host::~CountDown_Host()
{

}
ProgramState GameMaster_Host::CountDown_Host::Go()
{
	//originalseconds - var to store seconds elapsed which remains the same after the countdown is updated
	byte originalSeconds = secondsElapsed;
	
	//update countdown - ischagedelta param is false, because countdown is updated normally (see full description in declaration of func)
	UpdateCountDown( false );
	
	//having updated countdown, if new no. of seconds elapsed is greater than that before the update, create an update to sync with clients
	if( secondsElapsed > originalSeconds )
	{
		//update is a single byte with new no. of seconds elapsed
		char szUpdate  = secondsElapsed;
		vars.gameNet.AddUpdate( JNP::UPDATE_TYPES::COUNTDOWN_SYNC ,&szUpdate,1 );
	}
	//if countdown is finished return programstate::playing so GameMaster will change instance to game, otherwise draw countdown
	if( !CountDownFinished())
	{
		DrawCountDown( COUNTDOWN_X,COUNTDOWN_Y );
	}
	else
	{
		return PLAYING;
	}
	//press escape to cancel, returns to lobby upon pressing this
	if( kbd.keyPressedSF( VK_ESCAPE ))
		return LOBBY;
	//recieve data
	int playerIndex;
	sockaddr temp;
	while(1)
	{
		int len = vars.RecvData( playerIndex,&temp);
		//if buffer empty exit loop
		if( !len )
		{
			break;
		}
		else
		{
			/* if data recieved is a request, send reply telling client they cannot join because
			the game has started*/
			if( len > 0 && playerIndex == -1 )
			{
				if( vars.rcvBuf[ JNP::BYTE_CM ] == JNP::CM_NORMREQEST )
				{
					vars.SendPlayerReply( &temp,JNP::CM_INGAME,vars.rcvBuf[JNP::BYTE_PN] );
				}
			}
		}
	
	}
	return ProgramState::COUNTDOWN;
			
}

		
GameMaster_Host::InGame_Host::InGame_Host(GameVars_Host& gameVars )
	:vars(gameVars),
	InGame_Base( gameVars, 1 , true)	,
	SyncTimer( 300 )
{
	//make size hplayerlist has something in it. again, check playerlist is set to correct pointer.
	//this is essential as functions from InGame_Base utilises vars.playerList pointer
	assert( vars.hPlayerList.size() );
	vars.playerList = &vars.hPlayerList[0];

	int lineSize = sizeof(Line);		
	int clrSize = sizeof(D3DCOLOR );

	/*borders start as 2 times game view border for the top and bottom / left and right.
	
	players are arranged row by row to form a sqaure, even if the last row is not complete.
	square root the number of players and round up by adding 1.0f gives the number of players in 
	a row. -1 from this as we want the number of lengths rather than players 
	
	i.e. consider 7 players, players will form in a 3x3 square, although the last 2 spaces on the bottom
	row will not be filled. distance between these players is 2 distance between players (3 players - 1).
	therefore total border distance = 2 times viewborder + length between players * (nPLayerPerRow - 1)

	*/
	int sqrSideLen =	(int) ceil(sqrt( (float)vars.nPlayers )) - 1;
	borderBottom = borderRight = ( 2 * GAME_VIEWBORDER) - 1;
	borderBottom	+= sqrSideLen * GAME_START_XDIF;
	borderRight		+= sqrSideLen * GAME_START_YDIF;

	reset( );
	
	//plrDatLen. maz size for a player. 1 for playerdata, lineSize for first Line struct, 
	//clrSize for size of D3dColor, and NameSIze + 1 for player name
	const int plrDatLen = 1 + lineSize + clrSize + (NAMESIZE+1);
	int bufLen = 1 + 2 + (plrDatLen* vars.nPlayers);
	char * szInitUpdate = new char[bufLen];

	//first pack first 3 bytes of packet with new programstate programstate::playing and number of players
	int place = JNP::Pack_Packet( szInitUpdate,"cd",PLAYING,vars.nPlayers );
	
	//loop through all players packing player data into buffer.  place variable is incremented with size of
	//each player as packet is made (this can vary as some players have longer names than others)
	for( int index = 0; index < vars.nPlayers ; index++ )
	{
		Player& curPlayer = vars.playerList[index];
		Line& curLine = curPlayer.lineList[0];

		place += JNP::Pack_Packet( szInitUpdate + place,"cddddis",
			curPlayer.GetPlayerData(),
			curLine.x1,
			curLine.x2,
			curLine.y1,
			curLine.y2,
			curPlayer.playerColour,
			curPlayer.name );
	}

	//add update. sent to server as well
	vars.gameNet.AddUpdate( JNP::UPDATE_TYPES::CHANGE_STATE,szInitUpdate,place,false,JNP::HOST_DEST_CLIENTS | JNP::HOST_DEST_SERVER);
	delete[] szInitUpdate;


};
void GameMaster_Host::InGame_Host::NewDirection( UINT16 pIndex,  directionState newDir  )
{
	
	Player& curPlayer = vars.playerList[pIndex];				//makes calls to the revelant player easier
	Line& curLine = curPlayer.lineList[ curPlayer.currentLine ];//reference to player's current line

	


	int lineSize = sizeof(Line);

	
	/* length = 2 (player index) + 2(player current line) + 1 (new direction of player) and sizeof line
	for current line */
	int len = 2 + 2 + 1 + lineSize;
	char * szUpdate = new char[ len ];
	int place = 0;
	
	
	JNP::Pack_Packet( szUpdate, "ddcdddd" , 
		pIndex,
		curPlayer.currentLine,
		newDir,
		curLine.x1,
		curLine.x2,
		curLine.y1,
		curLine.y2 );

	vars.gameNet.AddUpdate( JNP::UPDATE_TYPES::NEW_DIR,szUpdate,len,true );
	delete[] szUpdate;

	//call to InGame_Base to change direction on screen one networking has been dealt with
	SetNewDirection( pIndex, newDir );
}
void GameMaster_Host::InGame_Host::InitPlrVarChange( UINT16 pIndex )
{
	//size is 3, 2 (player index) + 1 (player data)
	const int size = 3;
	char szUpdate[size];

	JNP::Pack_Packet( szUpdate,"dc",pIndex,vars.playerList[pIndex].GetPlayerData());
	
	vars.gameNet.AddUpdate( JNP::UPDATE_TYPES::PLR_VAR,szUpdate,size );
}
ProgramState GameMaster_Host::InGame_Host::Go()
{
	//function to recieve and processing incoming networking data
	ProcessRecvData();

	//if you have lost, if the game has or hasn't finished, display you lost
	if( vars.playerList[0].GetPlayerVar( PD_PLAYERLOST ))
		gfx.DrawString( "You Lost!",300,500,FONT_BIG,RED,1 );

	
	//draw screen
	DrawPlayerLines();
	//if game in progress
	if( !CheckForGameOver() )
	{		
		directionState curDir = (directionState)vars.hPlayerList[0].GetPlayerVar( PD_PLAYERDIRECTION );
		UpdateView( curDir, vars.hPlayerList[0].lineList.back() );

		//boolean var indicating whether program is where to update or not
		bool draw = ( timer.GetTimeMilli() > GAME_UPDATE_TO );

		//process direction input for host
		int hostIndex = 0;
		//pkeys only has one entry (up,down,left,right)
		directionState newDir =  TakeDirectionInput( &vars.playerList[hostIndex],pKeys[0] );
		//if new direction different to current set new direction and add networking update (see NewDirection() ) 
		if( newDir != curDir)
			NewDirection( hostIndex,newDir);

		//loop through clients
		for ( UINT16 index = 0; index < vars.nPlayers; index++ )
		{


			Player& curPlayer = vars.playerList[index];
			//check for clients timeout. host is index 0, so evidently they cannot timeout
			if( index >= 1 )
			{
				//networking index of clients starts at 0, vars.playerlist client indexing starts at 1
				if( vars.gameNet.HasClientTimedOut( index - 1 ))
				{
					curPlayer.SetPlayerVar( PD_PLAYERCONNECTED, false );	//player disconnected
					curPlayer.SetPlayerVar( PD_PLAYERLOST, true );			//player therefore dead
					InitPlrVarChange( index );								//create update telling clients that someone has died
				}
			}

			//if player is alive
			if( !curPlayer.GetPlayerVar( PD_PLAYERLOST )  )
			{
				//if it is time to update lines, update. if UpdateLine() returns true, then the player has crashed into a wall
				if( draw )
				{
					if( UpdateLine( &curPlayer))
					{
						curPlayer.SetPlayerVar( PD_PLAYERLOST,true);
						InitPlrVarChange( index ); //initialise the networking update for clients
					}
				}
				// if the player has crashed into someone else (or themself) they are dead.
				if( BoundaryCollisionCheck( index ))
				{
					curPlayer.SetPlayerVar( PD_PLAYERLOST,true );
					InitPlrVarChange( index );	//initialise the networking update for clients
				}
				
			}
			
		
		} //end looping through players

		//synctimer - if it has timed out, time to send sync update to clients
		if( SyncTimer.HasTimedOut() )
		{
			//size, 4 times UINT16 for 4 points in line + 1 times UINT16 for current line index for player.
			//so, 5 times UINT16, all times by number of players
			int size = (sizeof(UINT16) * 5) * vars.nPlayers;
			char * szUpdate = new char[size];
			int place = 0;
			for( int i = 0; i < vars.nPlayers; i++ )
			{
				Player& cur = vars.playerList[i];
				Line& curLine = cur.lineList[ cur.currentLine ];
				place += JNP::Pack_Packet( szUpdate + place, "ddddd",
					cur.currentLine,
					curLine.x1,
					curLine.x2,
					curLine.y1,
					curLine.y2 );

			}
			vars.gameNet.AddUpdate( JNP::GAME_SYNC, szUpdate,size );
			delete[] szUpdate;
			/*creates update to send to clients making sure they are in sync with host by sending the current
			coordinates of every client with their respective current line indexes*/
		}

		//if it was time to update lines, reset watch so you have to wait again
		if( draw )
		{
		
			timer.ResetWatch();
		}
	}//end of if game in progress code
	else
	{
		//if game not in progress i.e. its ended
		if( !vars.hPlayerList[0].GetPlayerVar( PD_PLAYERLOST ))
			gfx.DrawString( "You Won!",300,500,FONT_BIG,GREEN,1);	//if host wins (you)
		gfx.DrawString( "game over!\n",300,300,FONT_BIG,BLUE,1);
		gfx.DrawString( "press esc to exit\n",300,400,FONT_BIG,YELLOW,1);
		if( kbd.keyPressed( VK_ESCAPE ))	//escape to lobby
			return ProgramState::LOBBY;
	}
	
	return PLAYING;
};	
void GameMaster_Host::InGame_Host::ProcessRecvData()
{
	sockaddr recvIPData; //IP data for recieveing from payers
	ZeroMemory( &recvIPData,sizeof(sockaddr));

	int playerIndex;	//index of client
	
	while(1)
	{
		int len = vars.RecvData( playerIndex,&recvIPData  );
		if( !len )
			return; //if buffer empty exit
		if( len < 0 )
			continue;	//if data already processed re-enter loop
		byte cmByte = vars.rcvBuf[JNP::BYTE_CM];
		//if request recieved, send reply that we are ingame and cannot accept players
		if( playerIndex < 0 )	
		{
			if(cmByte == JNP::CM_NORMREQEST )
			{
				vars.SendPlayerReply( &recvIPData,JNP::CM_INGAME,vars.rcvBuf[JNP::BYTE_PN] );
			}
			continue;
		}
		
		Player& curPlayer = vars.hPlayerList[playerIndex];

		switch (vars.rcvBuf[JNP::BYTE_CM])
		{
		
			
		case JNP::CM_UPDATE_SND:
			{
				//update recieved from client
				
				char * p = vars.rcvBuf + JNP::BYTE_START;	//pointer to start of buffer
				JNP::UPDATE_TYPES updateType = (JNP::UPDATE_TYPES)vars.rcvBuf[JNP::BYTE_UT];
				switch (updateType)
				{
				case JNP::UPDATE_TYPES::NEW_DIR:
					directionState newDir = directionState( *p );	//first byte is new direction of client
					Line& curLine = curPlayer.lineList.back();	//reference to current players current line
					//client sends its current coordinates with its new direction. unpack and store coordinates
					JNP::UnpackPacket( p+1, "dddd",
						&curLine.x1,
						&curLine.x2,
						&curLine.y1,
						&curLine.y2 );
					NewDirection( playerIndex, newDir);	/*initiliases new direction of client on screen and
					creates update to send to other clients */
					break;
				}
			}
			break;
		case JNP::CM_QUIT:
			//if client quits
			curPlayer.SetPlayerVar( PD_PLAYERCONNECTED, false );	//client is disconnected
			curPlayer.SetPlayerVar( PD_PLAYERLOST, true );			//client is dead
			InitPlrVarChange( playerIndex );						//send update to other clients informing them
			break;
		}
	}

}


GameMaster_Host::Lobby_Host::Lobby_Host( MenuData& lobbyData , GameVars_Host& dat )
	:
	window(lobbyData.window),
	vars(dat),
	lobby( lobbyData )
{
	/** win32 stuff (display) */
	lobby.SetPlayerDat(vars.nPlayers,vars.maxPlayers);	//put relevant playerdata in info box
	lobby.SetLobbyName( vars.lobbyName );				//set title
	
	for( UINT32 index = 0; index < vars.hPlayerList.size(); index++ )
	{
		lobby.AddPlayer( vars.hPlayerList[index].name);	//add players to player listbox
	}
	/***********************/

	/* initial update contains number of players, max players, lobby name, and list of playernames */
	
	int maxLen = 
		sizeof( ProgramState) +		
		sizeof(vars.nPlayers) +		
		sizeof(vars.maxPlayers)+ 
		sizeof(vars.lobbyName) + 
		(vars.nPlayers * (NAMESIZE+1));
	char  * szUpdate = new char[ maxLen ];
	//pack programstate, nPlayers, maxPlayers and lobbyname into packet
	int len = JNP::Pack_Packet( szUpdate,"cdds",(char)LOBBY,vars.maxPlayers,vars.nPlayers,
		vars.lobbyName );
	//loop through playerlist and add add player names into packet
	for( int index = 0; index < vars.nPlayers; index++ )
	{
		len+= JNP::Pack_Packet( szUpdate + len,"s",vars.hPlayerList[index].name);
	}
	vars.gameNet.AddUpdate( JNP::UPDATE_TYPES::CHANGE_STATE,szUpdate,len,false,JNP::HOST_DEST_CLIENTS | JNP::HOST_DEST_SERVER );
	delete[] szUpdate;
	
}
ProgramState GameMaster_Host::Lobby_Host::LobbyGo()
{
	
	LobbyRecvData(); //process data recieved
	//loop through clients starting from 1 (0 is host), if they haved timed out, remove them from the lobby
	for( int index = 1; index < vars.nPlayers; index++ )
	{
		if( vars.gameNet.HasClientTimedOut( index - 1) )
		{
			LobbyRemovePlayer(index);
		}
	}
	
	//if connection to the master server fails, exit the lobby
	if( vars.connectedToServer)
	{
		
		if( vars.gameNet.HasTimedOut(IPComm_Host::serverListID, 0 ))	
		{
			vars.SendQuit();
			DoMessageBox( L"lost connection to master server",L"error");
			return ProgramState::MENU;
		}
	}
	//if this stage is reached return programstate return from ButtonPressed(). Id of button pressed is passed
	//from lobby.Go() which returns button pressed
	return ButtonPressed( lobby.Go() );
}
bool GameMaster_Host::Lobby_Host::IsNameTaken( char * checkName )
{
	for( int index = 0; index < vars.nPlayers; index++ )
	{
		if( !strcmp( checkName, vars.hPlayerList[index].name ))
			return true;
	}
	return false;
};
void GameMaster_Host::Lobby_Host::LobbyRemovePlayer( UINT16 index )
{
	char * name = vars.hPlayerList[index].name;

	//create message and print in lobby chat
	char szChatUpdate[255];
	strcpy( szChatUpdate,name );
	strcpy( szChatUpdate + strlen( name )," has left the game");
	lobby.ExtendChat( szChatUpdate );


	vars.nPlayers--;	//decrement number of players
	
	const int len = 4;	//length  = 2 (new number of players) + 2 ( index of player to remove)
	char szUpdate[len];
	int place = 0;

	JNP::Pack_Packet( szUpdate,"dd",vars.nPlayers,index);	//format packet

	vars.gameNet.AddUpdate( JNP::UPDATE_TYPES::REMOVE_PLAYER,szUpdate,len,false,JNP::HOST_DEST_CLIENTS | JNP::HOST_DEST_SERVER );
	vars.gameNet.RemoveClient( index - 1 );			//remove networking contact
			
	lobby.RemovePlayer( index );							//remove from win32 listbox
	lobby.SetPlayerDat( vars.nPlayers, vars.maxPlayers );	//set lobby data with new decremented number of players
	
	vars.hPlayerList.erase( vars.hPlayerList.begin() + index );	//remove fom players list
	vars.playerList = &vars.hPlayerList[0];	//reset playerlist pointer
}
void GameMaster_Host::Lobby_Host::LobbyAddPlayer(  sockaddr * IPData,const char * data)
{
	
	//send reply
	SendPlayerAcceptance(	*IPData,
							vars.rcvBuf[JNP::BYTE_PN]);
				
	const char * name = data + JNP::BYTE_START;

	//add to list and reset pointer
	vars.hPlayerList.push_back( Player(name));
	vars.playerList = &vars.hPlayerList[0];

	//increment number of players
	vars.nPlayers++;
	
	//add networking contact for client
	vars.gameNet.AddClient(  data,*IPData);
	
	

	char szUpdate[2 + NAMESIZE + 1];

	//update consists of number of new players, followed by name of player to add
	int len = JNP::Pack_Packet( szUpdate,"ds",vars.nPlayers,name);

	vars.gameNet.AddUpdate( JNP::UPDATE_TYPES::ADD_PLAYER ,szUpdate,len ,false,JNP::HOST_DEST_CLIENTS | JNP::HOST_DEST_SERVER);
	
	lobby.AddPlayer( name );
	
	//extend lobby chat
	char szChatUpdate[255];
	strcpy( szChatUpdate,name );
	strcpy( szChatUpdate + strlen(name)," has joined the game");
	lobby.ExtendChat( szChatUpdate );
	lobby.SetPlayerDat( vars.nPlayers, vars.maxPlayers );
}
byte GameMaster_Host::Lobby_Host::FormatChatMessage( char * szMessage,  int playerIndex )
{
	//format raw chat message to be printable to lobby chat, returns sound index if any
	byte soundIndex;

	//if chat message is a number between the first sound index (1) and the last sound index, then return the value
	//other wise return 0, because there is no sound to play
	int inNumber = atoi( szMessage );	
	if( inNumber >= SOUND_FIRSTSOUND &&
		inNumber <= SOUND_LASTSOUND)
	{
		soundIndex = inNumber;
	}
	else
	{
		soundIndex = 0;
	}

	char szTmp[ LOB_MAX_INPUT + 1 ];
	strcpy( szTmp,szMessage );

	//format from message such as "hi there" from steve to "steve: hi there"
	sprintf( szMessage, "%s: %s",
		vars.hPlayerList[playerIndex].name,
		szTmp );
	
	return soundIndex;
}
ProgramState GameMaster_Host::Lobby_Host::ButtonPressed( int buttonID )
{
	
	
	//process a button pressed for the lobby menu

	switch( buttonID )
	{
	case LOBBY_CHAT_EDIT:
		//firstly get the chat message. it is wiped from the screen when user presses enter.
		//call special function GetEditHWNDText() text to grab txt

		char szChatMsg[ MAX_CHAT_MSGLEN ];
		ZeroMemory( szChatMsg, MAX_CHAT_MSGLEN );
		GetEditHWNDText( szChatMsg );	
		if( !*szChatMsg ) //first character is NULL, i.e. no text entered ignore
			break;
		InitChatUpdate( szChatMsg,0);//otherwise create chat update, (player index 0 for host)
		break;
	case LOBBY_QUIT:
		return ProgramState::MENU;	//return to menu
		break;
	case LOBBY_HOST_START:
		if( vars.hPlayerList.size() > 1 )
		{
			return ProgramState::COUNTDOWN;	
		}
		else
		{
			lobby.ExtendChat( "You need more than 1 player to start!");
		}
		break;
	case LOBBY_HOST_BOOT:
		{
			int pIndex = lobby.GetSelPlayer();	//get index of selected player (if any)
			if( pIndex >= 1 )	//cannot kick index 0 as that is yourself
			{
				vars.KickPlayer( pIndex );	//sends packet to player informing them they have been kicked
				LobbyRemovePlayer( pIndex );//remove player and creates update to inform clients
			}

		}
		
				
		break;
	};
	return ProgramState::LOBBY;
}
void GameMaster_Host::Lobby_Host::InitChatUpdate( const char * chatMsg, int pIndex )
{
	char szChatInput[	MAX_CHAT_MSGLEN ];	//chat input buffer
	char updateMsg[		MAX_CHAT_MSGLEN ];	//update buffer
	strcpy( szChatInput, chatMsg );			//copy raw chat message into chat input bufer



	byte sIndex = FormatChatMessage( szChatInput, pIndex );	//format raw message and get sound index (sIndex)
	
	lobby.ExtendChat( szChatInput );	//extend lobby chat on screen
	
	if( sIndex  )
		vars.PlayChatSound( sIndex );	//if there is a sound , play it

	JNP::Pack_Packet( updateMsg,"cs",sIndex,szChatInput );	//packet is: 1 byte for sound index followed by chat message
	//length = length of chat message + 1 ( NULL ending char for string ) + 1 (sound index )

	//add networking update
	vars.gameNet.AddUpdate( JNP::CHAT_XTND,updateMsg,strlen(szChatInput)+2);

}
void GameMaster_Host::Lobby_Host::ProcessRequest( sockaddr& recvIPData, char * data )
{
	byte packetNo = data[JNP::BYTE_PN];


	if( vars.nPlayers >= vars.maxPlayers )	
	{
		//if game if full
		vars.SendPlayerReply(	&recvIPData,
								JNP::CM_FULL,
								packetNo );
	}
	else
	{
		if( !IsNameTaken( data + JNP::BYTE_START ))
		{
			//success
			LobbyAddPlayer( &recvIPData,data);	
		}
		else
		{
			//name is taken
			vars.SendPlayerReply(	&recvIPData,
									JNP::CM_NAMETAKEN,
									packetNo);
		}
	}
}
void GameMaster_Host::Lobby_Host::LobbyRecvData()
{
	sockaddr recvIPData; //IP data for recieveing from payers
	ZeroMemory( &recvIPData,sizeof(sockaddr));

	int playerIndex;
	
	while(1)
	{
		int len = vars.RecvData( playerIndex,&recvIPData  );
		if( !len )	//nothing recieved
			return;
		if( len < 0 )	//data doesnt need proessing
			continue;
		byte cmByte = vars.rcvBuf[JNP::BYTE_CM];
		if( playerIndex < 0 )
		{
			if(cmByte == JNP::CM_NORMREQEST )
			{
				//if request recieved, process it
				ProcessRequest( recvIPData,vars.rcvBuf );
			}
			continue; //go to end of loop
		}
		

		switch (vars.rcvBuf[JNP::BYTE_CM])
		{
		
			//update recieved from client
			case JNP::CM_UPDATE_SND:
			{
				
				int place = JNP::BYTE_START;

				//get update type
				JNP::UPDATE_TYPES updateType = (JNP::UPDATE_TYPES)vars.rcvBuf[JNP::BYTE_UT];	
				

				const char * p = vars.rcvBuf + place;
				
				switch (updateType)
				{
				case JNP::CHAT_XTND:
					//client update for extending chat. recieved update is character string for lobby input
					InitChatUpdate( p, playerIndex );
					break;
				}
			}
			break;
		case JNP::CM_QUIT:
			LobbyRemovePlayer( playerIndex );	//player has quit
			break;
		}
	}
	
	
}
void GameMaster_Host::Lobby_Host::SendPlayerAcceptance( sockaddr IPDat, int requestID )
{
	int maxLen = 
		JNP::BYTE_START +
		sizeof(vars.nPlayers)+
		sizeof(vars.maxPlayers)+
		sizeof(vars.lobbyName)+
		vars.nPlayers*(NAMESIZE+1);
	char * sendSuccess = new char[maxLen];
	ZeroMemory( sendSuccess,maxLen );

	vars.gameNet.FormatReply( sendSuccess,requestID );	//get networking to add networking stuff to the beginning bytes

	int len = JNP::BYTE_START;

	//add number of players, max players and lobby name to buffer
	len += JNP::Pack_Packet( sendSuccess + len,"dds",vars.maxPlayers,vars.nPlayers,vars.lobbyName );
	//loop through players and add names to buffer
	for( int index = 0; index < vars.nPlayers; index++ )
	{
		len += JNP::Pack_Packet( sendSuccess + len,"s",vars.hPlayerList[index].name);
	}
	
	//send to client
	vars.netBase.SendData( JNP::SOCK_HOST,len,sendSuccess,&IPDat );
	
	delete[] sendSuccess;
		
}
