/****************************************************************************************** 
 *	Chili DirectX Framework Version 12.04.24											  *	
 *	Game.cpp																			  *
 *	Copyright 2012 PlanetChili.net														  *
 *																						  *
 *	This file is part of The Chili DirectX Framework.									  *
 *																						  *
 *	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
 *	it under the terms of the GNU General Public License as published by				  *
 *	the Free Software Foundation, either version 3 of the License, or					  *
 *	(at your option) any later version.													  *
 *																						  *
 *	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
 *	GNU General Public License for more details.										  *
 *																						  *
 *	You should have received a copy of the GNU General Public License					  *
 *	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
 ******************************************************************************************/
#include "Master.h"


Master::Master( HWND hWnd,const KeyboardServer& kServer,ButtonServer& wServ ,D3DGraphics& rGfx)
:	
	state( MENU ),
	buttonClient( wServ ),
	audio( hWnd ),
	kbd( kServer ),
	gfx( rGfx ),
	GameInstance(NULL),
		
	networker(),
	cClient( gfx,kbd),
	hwnd(hWnd)
	
{
	gfx.End_DrawLoadIco();
	name = new char[NAMESIZE+1];
	ZeroMemory( name,NAMESIZE+1);
	GenerateRandName( name );
	menu = new MasterMenu( hwnd,buttonClient,networker,name,gfx );
	

}
Master::~Master()
{
	switch (state)
	{
	case MENU:
		delete menu;
		break;
	default:
		DeleteGame();
		break;

	}
	delete name;
}
void Master::DeleteGame()
{
	
	
	switch (gameType)
	{
	case GameType::HOST:
		delete (GameMaster_Host*)GameInstance;
		break;
	case GameType::CLIENT:
		delete (GameMaster_Client*)GameInstance;
		break;
	case GameType::OFFLINE:
		delete (GameMaster_Offline*)GameInstance;
		break;
	default:
		break;
	}
	ClearFrame();
	
}
void Master::Go()
{
	
	
	ReturnGameVals gameDat;
	switch (state)
	{
	case MENU:
		BeginFrame( false );
		state = menu->Go( (ReturnGameVals*)&gameDat );
		EndFrame( false  );

		if( state != MENU )
		{
			delete menu;
			gameType = gameDat.type;
			switch (gameDat.type)
			{
			case GameType::OFFLINE:
				GameInstance = new GameMaster_Offline(cClient,gameDat.nPlayers);
				break;
			case GameType::HOST:
				GameInstance = new GameMaster_Host(gameDat.nPlayers,buttonClient,hwnd,gameDat.lobbyName,cClient,name,networker,gameDat.IPrecv );
				break; 
			case GameType::CLIENT: 
				GameInstance = new GameMaster_Client( hwnd,buttonClient,cClient,gameDat.IPrecv,name,networker);
			default:
				break;
			};
		};

		break;
	//if state is lobby or in game do not break. check if state has been changed to
	//MENU. if so, clear screen buf delete game instance and create menu
	case LOBBY:
		BeginFrame( false );
		state = GameInstance->Go();
		EndFrame( false  );
		if( state == MENU )
		{
			DeleteGame();
			menu = new MasterMenu( hwnd,buttonClient,networker,name,gfx );
		}
		break;
		
	default:
		bool d3d = (state != ProgramState::LOBBY );
		BeginFrame( d3d );
		state = GameInstance->Go();	
		d3d = (state != ProgramState::LOBBY );
			EndFrame( d3d  );
		if( state == ProgramState::MENU )
		{
			DeleteGame();
			menu = new MasterMenu( hwnd,buttonClient,networker,name,gfx );
		}
		break;
	};
	

}

