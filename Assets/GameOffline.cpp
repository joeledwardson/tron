#include "Game.h"

void GameMaster_Offline::DeleteInstance(	ProgramState	state,
							void*			pLobby,
							void*			pGame,
							void*			pCountDown )
{
	
		switch (state)
		{
		case PLAYING:
			delete (InGame_Offline*)pGame;
			break;
		case COUNTDOWN:
			delete (CountDown_Offline*)pCountDown;
			break;
		default:
			break;
		}
		
}


GameMaster_Offline::GameMaster_Offline( baseClass& bRef,int nOfPlayers )
	:vars( bRef )
{
	vars.nPlayers = nOfPlayers;
	currentState = COUNTDOWN;	//game starts with countdown in offline mode
	countdown = new CountDown_Offline( bRef,STARTINGSECS );
	vars.playerList = new Player[nOfPlayers];
};
GameMaster_Offline::~GameMaster_Offline()
{
	delete[] vars.playerList;
	DeleteInstance( 
		currentState,
		NULL,
		(InGame_Base*)game,
		(CountDown_Base*)countdown);
}
ProgramState GameMaster_Offline::Go()
{
	assert( currentState == ProgramState::COUNTDOWN || ProgramState::PLAYING );
	ProgramState newState;

	if( currentState == COUNTDOWN )
	{
		//coutdown->Go() returns PLAYING when it has finished
		newState = countdown->Go();
		if(  newState== PLAYING )
		{
			delete countdown;
			game = new InGame_Offline( vars );
			currentState = ProgramState::PLAYING;
		}
	}

	else if( currentState== ProgramState::PLAYING )
	{
		newState =  game->Go();
	}
	
	currentState = newState;
	return currentState;
}



GameMaster_Offline::CountDown_Offline::CountDown_Offline( baseClass& bRef,int startingSeconds )
	:CountDown_Base( startingSeconds , bRef)
{

};
ProgramState GameMaster_Offline::CountDown_Offline::Go()
{
	CountDown_Offline::UpdateCountDown( false );
	
	if( kbd.keyPressedSF( VK_ESCAPE ))	//escape to menu
	{
		return MENU;
	}
	else if(  !CountDownFinished() )	
	{
		DrawCountDown( 300,200);
		
		return COUNTDOWN;
	}
	else
	{
		return PLAYING;
	}
}



GameMaster_Offline::InGame_Offline::InGame_Offline( GameVars_Base& gameVars )
	:InGame_Base( gameVars,gameVars.nPlayers,true )
{
	
	assert( gameVars.nPlayers >= 1 && gameVars.nPlayers <=4 );		
		
	borderBottom = SCREEN_HEIGHT - 1;
	borderRight = SCREEN_WIDTH - 1;

	reset(  );
};
GameMaster_Offline::InGame_Offline::~InGame_Offline()
{
	
}
ProgramState GameMaster_Offline::InGame_Offline::Go()
{
		
	if( kbd.keyPressedSF( 'B' ))
	{
		DebugBreak();
	}

	DrawPlayerLines();

	if( !CheckForGameOver() )
	{		
		

		
		for ( int index = 0; index < vars.nPlayers; index++ )
		{
			Player& curPlayer = vars.playerList[index];
			
			if( !curPlayer.GetPlayerVar( PD_PLAYERLOST ))
			{
				if( timer.GetTimeMilli() > GAME_UPDATE_TO )
				{
					if( UpdateLine( &curPlayer))
						curPlayer.SetPlayerVar( PD_PLAYERLOST,true);
				}
				directionState newDir =  TakeDirectionInput( &curPlayer,pKeys[index] );
				if( newDir != curPlayer.GetPlayerVar( PD_PLAYERDIRECTION ))
					SetNewDirection( index,newDir);
				if( BoundaryCollisionCheck( index ))
				{
					curPlayer.SetPlayerVar( PD_PLAYERLOST,true );
				}
				
			}
			
				
		}
	}
	else
	{
		gfx.DrawString( "Press escape to exit\nPress enter to restart",100,100,FONT_SMALL,RED,1);
		for( int index = 0; index < vars.nPlayers; index++ )
		{
			if( !vars.playerList[index].GetPlayerVar( PD_PLAYERLOST ))
			{
				char szStr[255];
				sprintf( szStr,"Player %d wins!\n",index + 1);
				gfx.DrawString( szStr,100,200 + index*100,FONT_SMALL,vars.playerList[index].playerColour,1);
			}
		}
		if( kbd.keyPressedSF( VK_ESCAPE ) )
			return (ProgramState)MENU;
		else if(kbd.keyPressedSF(VK_RETURN))
		{
			reset( );
		}
		
		
	}
	return  ProgramState::PLAYING;
};