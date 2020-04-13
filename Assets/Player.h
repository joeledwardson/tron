#ifndef PLAYER_H
#define PLAYER_H


#include <time.h>
#include <vector>
#include "Timer.h"
#include <assert.h>
#include "D3DGraphics.h"

#define snprintf _snprintf
#define PD_PLAYERLOST 1
#define PD_PLAYERCONNECTED 2
#define PD_PLAYERDIRECTION 3


#define PLAYERDATALEN 31

#define NAMESIZE 20

void GenerateRandName( char * name );
using namespace std;

//structure containing int values for direction key identifiers
struct PlayerDirectionKeys
{
	int up;
	int down;
	int left;
	int right;
	void SetKeys( int up, int down,int left,int right );
	PlayerDirectionKeys( int playerNumber );

};

enum directionState
{
	RIGHT,DOWN,UP,LEFT
};

class Player
{
public:
	
	Player(const char * playerName);
	Player();

	~Player();

	
			
	std::vector<Line> lineList;

	// player variables
	void SetPlayerVar( short variable,int var );

	//for bool variables in some cases will return 2 or 3 if true, not 1
	int GetPlayerVar( short variable );
	
	BYTE GetPlayerData();
	void SetPlayerData( char data );

	UINT16 currentLine;
	D3DCOLOR playerColour;

	char name[NAMESIZE+1];
private:
	/*
	1st bit = PD_PLAYERLOST;
	2nd bit = player connected
	;
	3rd and 4th bits = current direction;
	*/
	BYTE playerData;
};



#endif