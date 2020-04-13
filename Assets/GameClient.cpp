#include "Game.h"

void GameMaster_Client::DeleteInstance(	ProgramState	state,
							void*			pLobby,
							void*			pGame,
							void*			pCountDown )
{
	
		switch (state)
		{
		case LOBBY:
			delete (GameMaster_Client::Lobby_Client*)(pLobby);
			break;
		case PLAYING:
			delete ( InGame_Client *) pGame;
			break;
		case COUNTDOWN:
			delete (GameMaster_Client::CountDown_Client*)pCountDown;
			break;
		}
		
}

GameMaster_Client::GameMaster_Client( HWND window, ButtonClient& buttonRef,
baseClass& baseRef,const char * IPrecvBuf, char*name, GameNetworking& networker )
	:vars( baseRef, networker,name,IPrecvBuf),
	lobbyDat( buttonRef,window,ButtonData(	LOBBY_XCOORD_RIGHT,						
											LOBBY_YCOORD_TOP,						
											MENU_BTN_WIDTH,							
											MENU_BTN_HEIGHT,						
											MENU_BTN_YDIS,							
											LOBBY_CLIENT_NBUTTONS,					
											L"Quit", LOBBY_QUIT ))
{
	currentState = ProgramState::LOBBY;

	
	
	
	
	//upon recieveing acceptance into lobby, lobby data starts at the BYTE_START place in packet
	int place = JNP::BYTE_START;									
	lobby = new Lobby_Client( lobbyDat,vars, IPrecvBuf + place);
		
};
GameMaster_Client::~GameMaster_Client()
{
	
	vars.netBase.DestroyClient();

	DeleteInstance( currentState, lobby,game,countdown );
}
ProgramState GameMaster_Client::Go()
{
	
	//check for timeout to host
	if( vars.gameNet.HasTimedOut( 0,0 ))
	{
		DoMessageBox( L"timed out",L"error", MB_ICONINFORMATION );
		return MENU;
			
	}
	ProgramState newState;
	
	assert( currentState != ProgramState::MENU );
	
	/* lobby/countdown/game instance returns new programstate upon exiting go function
	processing of state returned by Go function is below*/

	//run instance depending on its type
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

	//if instance returns difference programstate, e.g. game has changed state from lobby to countdown
	if( newState != currentState && newState != ProgramState::MENU)
	{
		//delete current instance
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

		//init new instance
		switch (newState)
		{
		case LOBBY:
			ClearFrame();
			//if host sends update to change state to lobby, lobby data starts at BYTE_START + 1 in packet
			lobby = new Lobby_Client( lobbyDat,vars,vars.rcvBuf + JNP::BYTE_START + 1 );
			break;
		case PLAYING:
			game = new InGame_Client( vars );
			break;
		case COUNTDOWN:
			countdown = new CountDown_Client( STARTINGSECS,vars );
			break;
		default:
			break;
		}
	}
		
	vars.gameNet.SendData();	//send data to host

	/* if instance returns menu then quit to menu, otherwise set state to returned state(newState variable)*/
	if( newState == ProgramState::MENU )
		return ProgramState::MENU;
	else
	{
		currentState = newState;
		return currentState;
	}
		
};
			
GameMaster_Client::GameVars_Client::GameVars_Client(baseClass& bRef,GameNetworking& net, char*name, const char * hostReply)
	:GameVars_Online(bRef,net,name),
	gameNet((BaseNetworking&) net,hostReply,JNP::SOCK_CLIENT )
{};
int GameMaster_Client::GameVars_Client::RecvData( ProgramState& currentState )
{
	//set all bytes recv buffer to NULL
	ZeroMemory( rcvBuf,JNP::PS_MAX );

	int len = gameNet.RecvData( rcvBuf );	//run recvieve data from networking object
	//if returned value is 0, no data has been recieved. if < 0, then data has already been processed by networking object
	//in either case, no further processing required.
	if( len <= 0)
	{
		return len;
	}
	
	//process recieved data from host common to all instances (countdown/lobby/in game)
	byte commType = rcvBuf[JNP::BYTE_CM];

	switch( commType )
	{
		
	case JNP::CM_QUIT:
		DoMessageBox( L"host closed game",L"game closed",MB_ICONINFORMATION);
		currentState =  ProgramState::MENU;
		break;
	case JNP::CM_KICK:
		DoMessageBox( L"you have been kicked :(",L"kicked",MB_ICONINFORMATION);
		currentState =  ProgramState::MENU;
		break;

	case JNP::CM_UPDATE_SND:
	{
		if( rcvBuf[JNP::BYTE_UT] == JNP::CHANGE_STATE )
			currentState = ProgramState( rcvBuf[JNP::BYTE_START] );
	}
	default:
		break;
	
	}
	return len;
	
}

void GameMaster_Client::GameVars_Client::SendQuit()
{
	const int size = JNP::BYTE_CM+1;

	char sendingBuf[size];
	sendingBuf[ JNP::BYTE_CM ] = JNP::CM_QUIT;
	gameNet.SendPacket( 0,0,sendingBuf,size);

}

GameMaster_Client::Lobby_Client::Lobby_Client(MenuData& menuData, GameVars_Client& dat, const char * lobbyData  )
	:
	window(menuData.window),
	vars( dat ),
	lobby( menuData)
{
	//process lobby init packet from host
	
	//place is incremented as variables are retrieved (UnpackPacket returns no. of bytes contain by variables)
	int place = 0;
	


	/* first get number of players, max players and lobby name. then loop through gettting player names and displaying them*/
	place = JNP::UnpackPacket( lobbyData,"dds",&vars.maxPlayers,&vars.nPlayers,vars.lobbyName);

	
	lobby.SetLobbyName( vars.lobbyName );

	for( int index = 0; index < vars.nPlayers; index ++ )
	{

		char player[NAMESIZE+1];
		place += JNP::UnpackPacket( lobbyData + place, "s",player );
		lobby.AddPlayer(player ); 
	}

	
}
ProgramState GameMaster_Client::Lobby_Client::LobbyGo()
{
		
	ProgramState newState = ProgramState::LOBBY;
	newState = TranslateRecvData( vars.rcvBuf );
	
	//the "lobby" object is the win32 interaction for the lobby, and lobby.Go() returns the button pressed
	//newstate is just a reference 
	ButtonPressed( lobby.Go(),newState );
	


	return newState;
	
}
ProgramState GameMaster_Client::Lobby_Client::TranslateRecvData( char * data )
{
	ProgramState newState = ProgramState::LOBBY;

	while(1)
	{
		/*RecvData returns length of data returned.*/
		int len = vars.RecvData( newState );

		//if len == 0, no data recieved, if -1 data already processed
		if(!len)
		{
			break;
		}
		if( len < 0 )
			continue;
		int commType = data[ JNP::BYTE_CM ];
		
		//if there is an update to read
		if (commType == JNP::CM_UPDATE_SND)
		{
							
			int place = JNP::BYTE_UT;
			
			//get update type
			JNP::UPDATE_TYPES updateType = (JNP::UPDATE_TYPES) vars.rcvBuf[place];
			
			place = JNP::BYTE_START;
			const char * p = vars.rcvBuf + place;
			switch ( updateType )
			{
			case JNP::ADD_PLAYER:
				{
					//get new number of players and name of new player
					char szName[NAMESIZE+1];
					ZeroMemory( szName, NAMESIZE+1);
					JNP::UnpackPacket( p, "ds", &vars.nPlayers, szName );

					//update lobby data box
					lobby.SetPlayerDat( vars.nPlayers,vars.maxPlayers);
					
					//upon joining update saying player has joined the game is sent.
					//evidently we dont want to see "me" has joined the game so if player name is not equal to enter loop
					if( strcmp( szName,vars.name ))
					{
						//format lobby chat message
						char szChatUpdate[MAX_CHAT_MSGLEN];
						strcpy( szChatUpdate,szName );
						strcpy( szChatUpdate + strlen(szName)," has joined the game");
						lobby.ExtendChat( szChatUpdate );
					}
					//add player to lobby playerlist
					lobby.AddPlayer( szName );
				}
				break;
			case JNP::REMOVE_PLAYER:
				{
					//get new number of players and index of player to remove
					UINT16 pIndex;
					JNP::UnpackPacket( p, "dd",&vars.nPlayers,&pIndex);

					
					char szName[NAMESIZE+1];
					lobby.GetPlrName(pIndex, szName ); //retrieve name of player to remove
										

					char szChatUpdate[MAX_CHAT_MSGLEN];
					strcpy( szChatUpdate, szName );
					strcpy( szChatUpdate + strlen( szName )," has left the game");
					
					lobby.RemovePlayer( pIndex );		//remove player from list of players
					lobby.ExtendChat( szChatUpdate );	//extend lobby chat to say ""Player" has left the game"
					lobby.SetPlayerDat( vars.nPlayers,vars.maxPlayers);	//update lobby info box
					

				}
				
				break;
			case JNP::CHAT_XTND:
				{
					//first byte is sound index, (if any), to play. if there is a sound index to play, the byte will be >0
					char szChatUpdate[ MAX_CHAT_MSGLEN ];
					ZeroMemory( szChatUpdate, MAX_CHAT_MSGLEN );
					byte soundIndex;

					//get sound index and string to print to lobby screen
					JNP::UnpackPacket( p, "cs", &soundIndex, szChatUpdate );
					
					//if sound index is >0, i.e. there is a sound to play
					if( soundIndex )
						vars.PlayChatSound( soundIndex );

					lobby.ExtendChat( szChatUpdate );
				}
				break;
			default:
				break;
			}
		/************************/
		}
			
		
		
	}



	return newState;
}
void GameMaster_Client::Lobby_Client::ButtonPressed( int buttonID,ProgramState& newState )
{
	switch (buttonID)
	{
	case LOBBY_QUIT:
		vars.SendQuit();				//tell host you've quit
		newState = ProgramState::MENU;	//change current state to menu, so program exits game
		break;
	case LOBBY_CHAT_EDIT:
		/* upon hitting enter LOBBY_CHAT_EDIT is stored in button client. if something was actually entered,
		i.e. first byte is not NULL add update for chat message*/
		char szLobbyInput[LOB_MAX_INPUT+1];
		GetEditHWNDText( szLobbyInput );
		if( *szLobbyInput )
			vars.gameNet.AddUpdate( JNP::UPDATE_TYPES::CHAT_XTND,szLobbyInput,strlen(szLobbyInput) + 1 );
		break;
	}

}

GameMaster_Client::InGame_Client::~InGame_Client()
{
	delete[] vars.playerList;
}
GameMaster_Client::InGame_Client::InGame_Client( GameVars_Client& gameVars )
	:
	InGame_Base( gameVars,1 , false),	//1 player keys, only 1 player can play online at a time
	vars( gameVars )
{
	

	const char * p = vars.rcvBuf + JNP::BYTE_START;
	char t;	//temp char containing program state

	int place = JNP::UnpackPacket( p, "cd",&t,&vars.nPlayers );
	
	int sqrSideLen =	(int) ceil(sqrt( (float)vars.nPlayers )) - 1;
	borderBottom = borderRight = ( 2 * GAME_VIEWBORDER) - 1;
	borderBottom	+= sqrSideLen * GAME_START_XDIF;
	borderRight		+= sqrSideLen * GAME_START_YDIF;
	
	ProgramState curState = ProgramState((int)t);	//convert t to programstate, enums require ints for converting integer to enum though
	assert( curState == PLAYING );
	
	vars.playerList = new Player[ vars.nPlayers ];	//init player list pointer

	myIndex = -1;	//set my index to not found

	for( int i = 0; i < vars.nPlayers; i++ )
	{
		

		Player& cur = vars.playerList[i];
		cur.lineList.clear();
		cur.lineList.push_back( Line(0,0,0,0));
		Line& curLine = cur.lineList[0];

		byte plrDat;
		//get and set player data from host init buffer
		place += JNP::UnpackPacket( p + place, "cddddis",
			&plrDat,
			&curLine.x1,
			&curLine.x2,
			&curLine.y1,
			&curLine.y2,
			&cur.playerColour,
			&cur.name 
			);
		cur.SetPlayerData( plrDat );

		if( !strcmp( cur.name , vars.name ))	//if name if yours set your index to that of the current player
			myIndex = i;
	}

	assert( myIndex > -1 );
}
ProgramState GameMaster_Client::InGame_Client::Go()
{
	ProgramState newState =  ProcessRecvData();
	if( newState == MENU )
		return MENU;
	Player& me = vars.playerList[myIndex];

	
	bool draw = ( timer.GetTimeMilli() > GAME_UPDATE_TO );	//is game ready to update?

	//if you have lost draw string informing
	if( me.GetPlayerVar( PD_PLAYERLOST ))
		gfx.DrawString( "You Lost!\n",300,500,FONT_BIG,RED,1 );

	
	DrawPlayerLines();	//draw lines on screen

	if( !CheckForGameOver() )	//if game is in progress enter loop
	{		
		directionState curDir = (directionState)me.GetPlayerVar( PD_PLAYERDIRECTION );
		UpdateView( curDir, me.lineList.back() );

		//if you are alive enter loop
		if(! me.GetPlayerVar( PD_PLAYERLOST ))
		{
			//take direction input
			directionState newDir =  TakeDirectionInput( &me ,pKeys[0] );
			if( newDir != me.GetPlayerVar( PD_PLAYERDIRECTION ))
			{
				/* if new dir is different to current, init changes on screen, and create update with
				new direction and current line data to sync with host */
				const int len = 1 + sizeof(Line);
				char szUpdate[len];
				Line& curLine = vars.playerList[myIndex].lineList.back();
				JNP::Pack_Packet( szUpdate,"cdddd",
					char(newDir),
					curLine.x1,
					curLine.x2,
					curLine.y1,
					curLine.y2 );
			
				vars.gameNet.AddUpdate( JNP::NEW_DIR, szUpdate, len ,true);
				SetNewDirection( myIndex,newDir);
			}
		}
		for ( UINT16 index = 0; index < vars.nPlayers; index++ )
		{
			Player& curPlayer = vars.playerList[index];
			
			/* if player is alive*/
			if( !curPlayer.GetPlayerVar( PD_PLAYERLOST ) && 
				curPlayer.GetPlayerVar( PD_PLAYERCONNECTED) &&
				draw )
			{
					UpdateLine( &curPlayer);
			}
							
		}
	}
	else
	{
		/* if game is over, draw ending game text*/
		if( !me.GetPlayerVar( PD_PLAYERLOST ))
			gfx.DrawString( "You Win!",300,500,FONT_BIG,GREEN,1);
		gfx.DrawString( "game over!",300,300,FONT_BIG,BLUE,1);
		gfx.DrawString( "waiting for host to restart or quit..",300,400,FONT_BIG,YELLOW,1);
	}
	if( draw )
	{
		//if it is time to update, having updated all the lines reset the watch
		timer.ResetWatch();
	}
	return newState;
}
ProgramState GameMaster_Client::InGame_Client::ProcessRecvData()
{
	ProgramState newState = ProgramState::PLAYING;
	
	while(1)
	{
		int len = vars.RecvData( newState );

		if(!len)
		{
			break;
		}
		if( len < 0 )
			continue;
		if( newState != ProgramState::PLAYING )
			return newState;
	
				
				
		/********** updates *******/
		if( vars.rcvBuf[ JNP::BYTE_CM ] == JNP::CM_UPDATE_SND )
		{
						
			int place = JNP::BYTE_UT;
			JNP::UPDATE_TYPES updateType = (JNP::UPDATE_TYPES) vars.rcvBuf[place];
			
			place = JNP::BYTE_START;
			
			switch ( updateType )
			{
			case JNP::UPDATE_TYPES::NEW_DIR:
				{
					UINT16 playerIndex;
					place += JNP::UnpackPacket( vars.rcvBuf + place,"d",&playerIndex);
					
					if( playerIndex == myIndex )
						break;

					Player& cur = vars.playerList[playerIndex];
					Line& curLine = cur.lineList[ cur.currentLine ];

					char newDir;
					UINT16 iCurrentLine = cur.currentLine;

					JNP::UnpackPacket( vars.rcvBuf + place, "dcdddd",
						&cur.currentLine,
						&newDir,
						&curLine.x1,
						&curLine.x2,
						&curLine.y1,
						&curLine.y2
						);

					if( iCurrentLine != cur.currentLine )
					{
						
						DoMessageBox( L"Went out of sync :(",L"error");
						newState = ProgramState::MENU;
					}
					else
					{
						
						directionState newDirection = directionState(int(newDir));
						SetNewDirection( playerIndex,newDirection );
					}

				}
				break;
			case JNP::UPDATE_TYPES::PLR_VAR:
				{
					UINT16 playerIndex;
					char playerDat;
					JNP::UnpackPacket( vars.rcvBuf + place, "dc" , &playerIndex,&playerDat);
					Player& curPlayer = vars.playerList[playerIndex];
					curPlayer.SetPlayerData( playerDat );
				}
				break;
			case JNP::GAME_SYNC:
				for( int i = 0; i < vars.nPlayers; i++ )
				{

					Player& curPlayer = vars.playerList[i];
					Line& curLine  = curPlayer.lineList[ curPlayer.currentLine ];
					UINT16 curLineNo;
					
					place += JNP::UnpackPacket( vars.rcvBuf + place,"d",&curLineNo );
					if( curPlayer.currentLine == curLineNo )
					{
						place += JNP::UnpackPacket( vars.rcvBuf + place, "dddd",
							 &curLine.x1,
							 &curLine.x2,
							 &curLine.y1,
							 &curLine.y2 );
					}
					else
					{
						place += (sizeof( UINT16 ) * 4);
					}

	
				}
			}
			break;
		/************************/
		}
			
		
		
	}
	return newState;

}

GameMaster_Client::CountDown_Client::CountDown_Client( UINT32 startSecs,GameVars_Client& dat )
	:CountDown_Base( startSecs,dat.baseRef ),
	vars(dat)
{
	
}
ProgramState GameMaster_Client::CountDown_Client::Go()
{
	ProgramState newState;
	
	newState = TranslateRecvData();
	UpdateCountDown( true );
	if( !CountDownFinished())
	{
		DrawCountDown( COUNTDOWN_X,COUNTDOWN_Y );
	}
	countdownTimer.ResetWatch();
	return newState;
}
ProgramState GameMaster_Client::CountDown_Client::TranslateRecvData( )
{
	ProgramState newState = ProgramState::COUNTDOWN;
	
	while(1)
	{
		int len = vars.RecvData( newState );
		if( !len )
			break;
		if( len < 0 )
			continue;

		if( newState != ProgramState::COUNTDOWN )
			return newState;
		
		if( vars.rcvBuf[ JNP::BYTE_CM ] == JNP::CM_UPDATE_SND )
		{
				
			JNP::UPDATE_TYPES updateType = (JNP::UPDATE_TYPES) vars.rcvBuf[JNP::BYTE_UT];
			int place = JNP::BYTE_START;
			switch (updateType)
			{
			case JNP::COUNTDOWN_SYNC:
				secondsElapsed = vars.rcvBuf[place];
				milSecsElapsed = 0;
				countdownTimer.ResetWatch();
				break;
			default:
				break;
			}
		}
			
		
		
	}
	return newState;
}

