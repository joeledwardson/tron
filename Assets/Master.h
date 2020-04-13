/****************************************************************************************** 
 *	Chili DirectX Framework Version 12.04.24											  *	
 *	
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
#pragma once

#ifndef THEMASTER_H
#define THEMASTER_H




#include "Sound.h"
#include "MasterMenu.h"
#include "Game.h"

extern int widthTotal;
extern int height;




class Master
{

public:
	Master( HWND hWnd,const KeyboardServer& kServer,ButtonServer& wServ,D3DGraphics& rGfx );
	~Master();

	void Go();	
	
private:
	D3DGraphicsClient gfx;

	

	void DeleteGame();

	



	
private:
	/*system variables */
	HWND hwnd;
	KeyboardClient kbd;
	/*MouseClient mouse;*/
	DSound audio;
	GameNetworking networker;
	Timer serverConnection;
	
	baseClass cClient;
	MasterMenu * menu;
	GameMaster_Base * GameInstance;
	ProgramState state;
	ButtonClient buttonClient;
	GameType gameType;
	char * name;
	/***/

private:
	
};

#endif