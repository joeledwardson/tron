/****************************************************************************************** 
 *	Chili DirectX Framework Version 12.04.24											  *	
 *	Keyboard.cpp																		  *
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
#include "Keyboard.h"


KeyboardClient::KeyboardClient( const KeyboardServer& kServer )
	: server( kServer )
{}

bool KeyboardClient::keyPressed( int key )
{
	
	return server.keys[key];
}

bool KeyboardClient::keyPressedSF( int key )
{
	if( server.keys[key] == true && server.keysPressedLastFrame[key] == false )
		return true;
	else
		return false;
}

KeyboardServer::KeyboardServer()
{
	keys = new bool[NKEYS];
	keysPressedLastFrame = new bool[NKEYS];
	for( int index = 0; index < NKEYS; index++ )
	{
		keys[index] = false;
		keysPressedLastFrame[index] = false;
	}
}
KeyboardServer::~KeyboardServer()
{
	if( NKEYS )
	{
		delete[] keys;
		delete[] keysPressedLastFrame;
	}
}

void KeyboardServer::OnKeyPressed( int key )
{
	keys[key] = true;
}

void KeyboardServer::OnKeyReleased( int key )
{
	keys[key] = false;
	keysPressedLastFrame[key] = false;
}

void KeyboardServer::Update()
{
	for( int index = 0; index < NKEYS; index++ )
	{
		if( keys[index] == true )
			keysPressedLastFrame[index] = true;
	}
}