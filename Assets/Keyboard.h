/****************************************************************************************** 
 *	Chili DirectX Framework Version 12.04.24											  *	
 *	Keyboard.h																			  *
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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#define NKEYS 0x69

class KeyboardServer;

class KeyboardClient
{
public:
	KeyboardClient( const KeyboardServer& kServer );
	bool keyPressed( int key ); //return bool var if key is pressed or not
	bool keyPressedSF( int key ); //returns bool var if key is pressed for single frame. if held down will only be true for one frame
private:
	const KeyboardServer& server;
};

class KeyboardServer
{
	friend KeyboardClient;
public:
	KeyboardServer();
	~KeyboardServer();
	void Update();
	void OnKeyPressed( int key );
	void OnKeyReleased( int key );
private:
	bool * keys;
	bool * keysPressedLastFrame;
};

#endif