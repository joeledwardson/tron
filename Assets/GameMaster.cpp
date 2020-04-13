#include "Game.h"



GameMaster_Base::GameVars_Base::GameVars_Base(baseClass& bRef)
	:baseRef(bRef),
	playerList(NULL)
{};


GameMaster_Base::CountDown_Base::CountDown_Base(int startSecs,baseClass& bRef)
	:secondsElapsed(0),
	milSecsElapsed(0),
	baseClass(bRef),
	startingSeconds(startSecs)
{};
void GameMaster_Base::CountDown_Base::DrawCountDown(int x, int y )
{
		
	float fade =(float) milSecsElapsed /1000.0f;	//fractional number for degree of fade

	UINT32 seconds = startingSeconds - secondsElapsed;	//seconds to draw - i.e. if 3 seconds passed and starting seconds is 5 then seconds to draw is 2
	assert( seconds > 0 && seconds <= startingSeconds );
	gfx.DrawChar( 48 + seconds,	//48 is starting value for numbers in ASKII codes
		x,
		y,
		FONT_BIG,	//id for big font
		gfx.CalcFadedColour( LOAD_RED,LOAD_GREEN,LOAD_BLUE,fade),	//function calculates faded colour for given r,g,b and fractional degree of fade value
		COUNTDOWN_ENLARGE);	//number of enlargement of font
	
}
void GameMaster_Base::CountDown_Base::UpdateCountDown( bool isChangeDelta )
{
	if( !isChangeDelta )
	{
		//change is not delta

		milSecsElapsed = (int) countdownTimer.GetTimeMilli();	//get total milseconds time elapsed
		secondsElapsed = milSecsElapsed / 1000;					//set seconds to total miliseconds / 1000 and rounded down
		milSecsElapsed %= 1000;									//set miliseconds elapsed to miliseconds remainder of total
	}
	else
	{
		//change is delta

		milSecsElapsed += (int) countdownTimer.GetTimeMilli();	//miliseconds elapsed is appended by time elapsed
		//if miliseconds elapsed has become greater than 1000, seconds is incremeted
		secondsElapsed += (milSecsElapsed / 1000);
		milSecsElapsed %= 1000;	//miliseconds is then cut down to just the miliseconds
	}
}
bool GameMaster_Base::CountDown_Base::CountDownFinished()
{
	return secondsElapsed >= startingSeconds;
}




GameMaster_Base::InGame_Base::InGame_Base( GameVars_Base& pVars, UINT nKeys, bool asignColours )
		:baseClass( pVars.baseRef ),
		vars(pVars),
		screenX(0),
		screenY(0)
{	
	assert( vars.nPlayers >= 2 );
	assert( nKeys <= vars.nPlayers );
	if( asignColours )
	{
		//if colours are to be assigned loop through players and set a random colour
		for( int index = 0; index < vars.nPlayers; index++ )
		{
			//maximum value for r/g/b is 200 or colour could be too light to see
			vars.playerList[index].playerColour = D3DGraphicsClient::GenRandCol( 200 );
		
		}
	}
	//allocate memory space for pdk (player direction keys) list
	pKeys = (PlayerDirectionKeys*) malloc( sizeof( PlayerDirectionKeys) * vars.nPlayers);

	//create temporary vector, and add the pdk values for each player to it
	vector<PlayerDirectionKeys> tempPDK;
	for( UINT index = 0; index < nKeys ; index++ )
	{
		tempPDK.push_back( PlayerDirectionKeys(index) );
	}
	//copy vector value to pKeys, vector will be automatically destroyed at end of constructor
	memcpy( pKeys,&tempPDK.at(0),sizeof(PlayerDirectionKeys) * vars.nPlayers );

}
GameMaster_Base::InGame_Base::~InGame_Base()
{
	//free pdk memory
	free( pKeys );
}
void  GameMaster_Base::InGame_Base::DrawPlayerLines()
{
	/*function calls for drawing lines are DrawVLine_VP() rather thatn DrawVLine() because some lines may be partially or completely
	off the screen in multiplayer games */

	//***draw border lines***
	gfx.DrawVLine_VP( screenX,screenY,Line(0,			borderRight,	0,				0			),	RED);
	gfx.DrawVLine_VP( screenX,screenY,Line(0,			borderRight,	borderBottom,	borderBottom),	RED);
	gfx.DrawVLine_VP( screenX,screenY,Line(0,			0,				0,				borderBottom),	RED);
	gfx.DrawVLine_VP( screenX,screenY,Line(borderRight,	borderRight,	0,				borderBottom),	RED);
	//*********

	//draw player lines
	for( int index = 0; index < vars.nPlayers; index ++ )
	{
		Player& curPlayer = vars.playerList[index];
		for( int index2 = 0; index2 <= curPlayer.currentLine; index2++)
		{
			gfx.DrawVLine_VP(screenX,screenY, curPlayer.lineList[index2], curPlayer.playerColour);
		}	
	};
};
directionState  GameMaster_Base::InGame_Base::TakeDirectionInput( Player* player, const PlayerDirectionKeys playerKeys)
{
	//get current direction
	directionState curDir = (directionState)player->GetPlayerVar( PD_PLAYERDIRECTION );
	
	//get boolean vars for if keys are pressed
	bool up = kbd.keyPressedSF( playerKeys.up );
	bool down = kbd.keyPressedSF( playerKeys.down );
	bool left = kbd.keyPressedSF( playerKeys.left );
	bool right = kbd.keyPressedSF( playerKeys.right );

	/* if player is going up, then if up is pressed it makes no difference,
	and if down is pressed you cant go directly back on yourself. the same works
	with down. therefore if up or down are pressed then if current direction is
	up or down ignore. the same with left or right.*/
	if( ((( up) || down ) && (	curDir != UP	&& curDir != DOWN) ) ||		//check if keys are pressed
		((right || left ) && (	curDir != RIGHT	&& curDir != LEFT) )	)
	{
		//return new direction based on key pressed
		if(up)
			return directionState::UP;		
		else if(down)
			return directionState::DOWN;
		else if(left)
			return directionState::LEFT;
		else if(right)
			return directionState::RIGHT;
	}
	
	//otherwise return standard normal direction
	return curDir;
	
	
}
void  GameMaster_Base::InGame_Base::SetNewDirection( UINT16 playerIndex, directionState newDir )
{
	//create useful references to current player, current direction, and current line
	Player& curPlayer = vars.playerList[playerIndex];
	directionState curDir = (directionState)curPlayer.GetPlayerVar( PD_PLAYERDIRECTION );
	Line& curLine = curPlayer.lineList[ curPlayer.currentLine ];
	
	Line newLine(curLine);	//create new line for new direction, set to same as original line

	switch (curDir)
	{
	case RIGHT:
		//new line will start at the right point of original line. therefore left point of new line = right point of old line
		newLine.x1 = newLine.x2;	
		break;
	case DOWN:
		newLine.y1 = newLine.y2;
		break;
	case UP:
		newLine.y2 = newLine.y1;
		break;
	case LEFT:
		newLine.x2 = newLine.x1; 
		break;
	default:
		break;
	}
		
	curPlayer.lineList.push_back( newLine );
		
	//increment current line by 1
	curPlayer.currentLine++;
	//change direction
	
	curPlayer.SetPlayerVar( PD_PLAYERDIRECTION,newDir);
	
}
bool  GameMaster_Base::InGame_Base::BoundaryCollisionCheck(UINT16 playerIndex)
{
	//create relevant refences to current player, current line and current direction
	Player& curPlayer = vars.playerList[playerIndex];
	Line& curLine = curPlayer.lineList[ curPlayer.currentLine ];
	directionState curDir = (directionState)curPlayer.GetPlayerVar( PD_PLAYERDIRECTION );
	
	
	/* take the current line of the player being checked.

	loop through every player (including themself) and check the current line (described previously)
	against all lines of the player being checked for collision*/
	for( int player = 0; player < vars.nPlayers ; player++ )
	{
		

		Player& chkPlayer = vars.playerList[player];

		for( int lineIndex = 0; lineIndex <= chkPlayer.currentLine; lineIndex++) 
		{
			/* the only lines exempt from this checking are the current and directly preceeding line
			of the player itself. clearly the player's current line will always be "colliding" with itself
			and the line preceeding it*/
			if( player == playerIndex && lineIndex >= (curPlayer.currentLine-1) )
				break;
			

			Line& chkLine = chkPlayer.lineList[lineIndex];

			/* main loop for checking for collisions. if the current direction is right or left,
			the y coordinate for checking can be either y2 or y1 seeing as they are the same
			( y2 is used below for consistency .)

			if the current direction is right, the coordinate for checking if the current line has
			collided is taken as x2, if left, x1.

			the same principle is done for up/down. x2 is used for consistency

			y1 is the used for checking if up, y2 used for down

			*/

			switch (curDir)
			{
			case RIGHT:
				if( curLine.x2 <= chkLine.x2 &&
					curLine.x2 >= chkLine.x1 &&
					curLine.y2 >= chkLine.y1 &&
					curLine.y2 <= chkLine.y2 )
				{
					curLine.x2 = chkLine.x1;
					return true;
				}
				break;
			case DOWN:
				if( curLine.x2 <= chkLine.x2 &&
					curLine.x2 >= chkLine.x1 &&
					curLine.y2 >= chkLine.y1 &&
					curLine.y2 <= chkLine.y2 )
				{
					curLine.y2 = chkLine.y1;
					return true;
				}
				break;
			case UP:
				if( curLine.x2 <= chkLine.x2 &&
					curLine.x2 >= chkLine.x1 &&
					curLine.y1 >= chkLine.y1 &&
					curLine.y1 <= chkLine.y2 )
				{
					curLine.y1 = chkLine.y2;
					return true;
				}
				break;
			case LEFT:
				if( curLine.x1 <= chkLine.x2 &&
					curLine.x1 >= chkLine.x1 &&
					curLine.y2 >= chkLine.y1 &&
					curLine.y2 <= chkLine.y2 )
				{
					curLine.x1 = chkLine.x2;
					return true;
				}
				break;
			default:
				break;
			}
				
				
		}
			
		
		
	}

	
	return false;
}
bool  GameMaster_Base::InGame_Base::UpdateLine(Player *cPlayer)
{

	Line& curLine = cPlayer->lineList[cPlayer->currentLine];
	//update current player position depending on current direction
	
	directionState pDir = (directionState)cPlayer->GetPlayerVar(PD_PLAYERDIRECTION);

	assert( pDir >= 0 && pDir <= 3 );

	assert( curLine.y1 == curLine.y2 ||
			curLine.x1 == curLine.x2 );
	/* depending on current direction update current line*/

	INT32 newCoord;
	switch( pDir )
	{
		case RIGHT:
			newCoord = curLine.x2 + GAME_SPEED;
			
			if( newCoord >=  (borderRight) )
			{
				curLine.x2 = borderRight;
				return true;
			}
			else
			{
				curLine.x2 = newCoord;
			}
			break;
		case LEFT:
			newCoord = curLine.x1 - GAME_SPEED;
			if(newCoord < 1 )
			{
				curLine.x1 = 1;
				return true;
			}
			else
			{
				curLine.x1 = newCoord;
			}
			break;
		case UP:
			newCoord = 	curLine.y1 - GAME_SPEED;
			if( newCoord < 1)
			{
				curLine.y1 = 1;
				return true;
			}
			else
			{
				curLine.y1 = newCoord;
			}
			break;
		case DOWN:
			newCoord =   curLine.y2 + GAME_SPEED;
			if( newCoord >= (borderBottom) )
			{
				curLine.y2 = (borderBottom);
				return true;
			}
			else
			{
				curLine.y2 = newCoord;
			}
			break;
	}	
	return false;
}
bool  GameMaster_Base::InGame_Base::CheckForGameOver()
{
	//loop through list of players, if player alive increment number of active players (nActive)
	int nActive = 0;
	for( int index = 0; index< vars.nPlayers; index++ )
		nActive += !vars.playerList[index].GetPlayerVar( PD_PLAYERLOST );
	if( nActive >= 2)
		return false;
	else
		return true;
}
void  GameMaster_Base::InGame_Base::reset()
{
		
	int squareSideLen; //length of side of stating square. for 4 players. starting pos is 2by2 so square side=2.
	//for 7 players formation will be row of 3, row of 3, row of 1. therefore squareSideLen = sqrt(nPlayers) rounded up;

	float temp = float(sqrt( (float)vars.nPlayers ));
	//if value is not an integer round up by 1.
	squareSideLen = int(ceil( sqrt( (float)vars.nPlayers )));
	
	
	

	int x = GAME_VIEWBORDER;
	int y = GAME_VIEWBORDER;
	int row = 0;
	int	col = 0;
	for( int i = 0; i < vars.nPlayers; i++ )
	{
		Player& curPlayer = vars.playerList[i];
		curPlayer.lineList.clear();
		curPlayer.SetPlayerVar( PD_PLAYERDIRECTION, directionState(rand()%4));
		curPlayer.currentLine = 0;
		curPlayer.SetPlayerVar( PD_PLAYERLOST, false);
		curPlayer.SetPlayerVar( PD_PLAYERCONNECTED, true );		

		curPlayer.lineList.push_back( Line(x,x,y,y) );


		col++;
		x += GAME_START_XDIF;
		if( col == squareSideLen )
		{
			x = GAME_VIEWBORDER;
			y += GAME_START_YDIF;
			row++;
			col = 0;
		}
	}
	
}
void GameMaster_Base::InGame_Base::UpdateView(directionState curDir,Line curLine)
{
	switch (curDir)
	{
	case RIGHT:
		if( curLine.x2 > screenX + SCREEN_WIDTH - GAME_VIEWBORDER && curLine.x2 < borderRight - GAME_VIEWBORDER)
			screenX = curLine.x2 - GAME_VIEWBORDER;
		
		break;
	case DOWN:
		if( curLine.y2 > screenY + SCREEN_HEIGHT - GAME_VIEWBORDER && curLine.y2 < borderBottom - GAME_VIEWBORDER )
			screenY = curLine.y2 - GAME_VIEWBORDER;
		break;
	case UP:
		if( curLine.y1 < screenY + GAME_VIEWBORDER && curLine.y1 > GAME_VIEWBORDER)
			screenY = curLine.y1 + GAME_VIEWBORDER - SCREEN_HEIGHT;
		break;
	case LEFT:
		if( curLine.x1 < screenX + GAME_VIEWBORDER && curLine.x1 > GAME_VIEWBORDER)
			screenX = curLine.x1 + GAME_VIEWBORDER - SCREEN_WIDTH;
		break;
	default:
		break;
	}
}

/********************************online game ********************************************************/


GameVars_Online::GameVars_Online(baseClass& bRef,GameNetworking& net,  char*nameRef)
	:name(nameRef),
	netBase(net),
	GameVars_Base( bRef )
{
	
	ZeroMemory( lobbyName,NAMESIZE+1);
	rcvBuf = new char[JNP::PS_MAX];
	ZeroMemory( rcvBuf, JNP::PS_MAX );
}
GameVars_Online::~GameVars_Online()
{
	delete[] rcvBuf;
}
void GameVars_Online::PlayChatSound( int soundIndex )
{
	assert( soundIndex >= SOUND_FIRSTSOUND && soundIndex <= SOUND_LASTSOUND );
	wchar_t szwFileLoc[255];				//buffer for file name
	ZeroMemory( szwFileLoc, sizeof(wchar_t)*255);
	lstrcpyW( szwFileLoc,SOUND_LOCATION);	//copy sound location to file name buffer (Sounds directory)
	_itow( soundIndex,szwFileLoc + SOUND_LOC_LEN,10 );	//append number of sound index to end of buffer
	lstrcat( szwFileLoc,L".wav");						//add .wav to the end
	assert(PlaySound( szwFileLoc,NULL,SND_ASYNC ));		//use PlaySound() macro. SND_ASYNC so it doesnt wait for sound to finish before continuing
}