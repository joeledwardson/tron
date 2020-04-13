#include "Player.h"


void PlayerDirectionKeys::SetKeys( int up, int down,int left,int right )
{
	this->up = up;
	this->down = down;
	this->left = left;
	this->right = right;
};
PlayerDirectionKeys::PlayerDirectionKeys( int playerNumber )
{
		
	switch(playerNumber)
	{
	case 0:
		SetKeys( VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT );
		break;
	case 1:
		SetKeys( 'W','S','A','D' );
		break;
	case 2:
		SetKeys( VK_NUMPAD8,VK_NUMPAD5,VK_NUMPAD4,VK_NUMPAD6 );
		break;
	case 3:
		SetKeys( 'I','K','J','L' );
		break;

	}
	
}


Player::Player(const char * playerName)
{
	if( playerName )
		strcpy( name,playerName );
	 
	ZeroMemory( &playerData,1 );

	

	SetPlayerVar( PD_PLAYERCONNECTED, true );
	SetPlayerVar( PD_PLAYERLOST, false );
	currentLine = 0;



}
Player::Player()
{


	ZeroMemory(name,NAMESIZE+1);
	ZeroMemory( &playerData,1 );

	SetPlayerVar( PD_PLAYERCONNECTED, true );
	SetPlayerVar( PD_PLAYERLOST, false );
	currentLine = 0;
}
Player::~Player()
{
	/*delete name;*/
}

void Player::SetPlayerVar( short variable,int var )
{
	switch( variable)
	{
			
	case PD_PLAYERLOST:
		if( var > 0 )
			playerData = playerData | 8; // binary = 00001000
		else
			playerData = playerData & 7; // binary = 00000111
		break;
	case PD_PLAYERCONNECTED:
		if( var > 0 )
			playerData = playerData | 4; // binary = 00000100
		else
			playerData = playerData & 11;// binary = 00001011
		break;
	case PD_PLAYERDIRECTION:
		if( var <= 4 && var >= 0 )
		{
			playerData = playerData & 12; //binary = 00001100. set variable to 0
			playerData = playerData | var; //move var left twice and set to playerdata
			break;
		}
		else
		{
			MessageBox( NULL,L"error assigning player direction",L"error",NULL);
		}
	}
}
int Player::GetPlayerVar( short variable )
{
	switch( variable)
	{
			
	case PD_PLAYERLOST:
			
		return playerData >> 3;
		break;
	case PD_PLAYERCONNECTED:
			
		return playerData & 4; //binary = 00000100
		break;
	case PD_PLAYERDIRECTION:
		return ( playerData & 3 ); //binary = 00000011
		break;
	default:
		MessageBox( NULL,L"invalid option for getplayervar",NULL,NULL);
		return 0;
	}
		
}
BYTE Player::GetPlayerData()
{
	return playerData;
}
void Player::SetPlayerData( char data )
{
	playerData = data;
}

	


void GenerateRandName( char * name )
{
	
	srand( (UINT32) time(NULL) );
	const int nNames = 10;
	char ** nameList = new char*[nNames];
	nameList[0] = "BECAUSE I'M BATMAN";
	nameList[1] = "Overturned hedgehog";
	nameList[2] = "Darth maul";
	nameList[3] = "epic bacon";
	nameList[4] = "A burnt crumpet";
	nameList[5] = "ignorant pumpkin";
	nameList[6] = "confused penguin";
	nameList[7] = "bent.I.am";
	nameList[8] = "bob";
	nameList[9] = "general taboo";
	for( int index = 0; index < nNames; index++ )
	{
		assert( strlen(nameList[index]) <= NAMESIZE );
	}
	int i = (rand() % nNames);
	memcpy( name,nameList[i],strlen(nameList[i])+1);
	delete[] nameList;
}