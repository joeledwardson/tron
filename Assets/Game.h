#ifndef GAME_H
#define GAME_H

#include "Player.h"
// **player.h**
//#include <time.h>
//#include <vector>
//#include "Timer.h"
//#include <assert.h>
//#include "D3DGraphics.h"

#include "GameNetworking.h"
//	**gamenetworking.h**
//#include <stdlib.h>
//#include <stdio.h>
//#include <vector>
//#include "WinSys.h"
	//winsys.h
	//#include <assert.h>	 //assertions
	//#include <Windows.h> //win32 calls
	//#include <process.h> //threading

#include "Lobby.h"
#include "Keyboard.h"


#define STARTINGSECS		5		//starting seconds for countdown in games
#define COUNTDOWN_ENLARGE	7		//enlargement of text in countdown
#define COUNTDOWN_X			200		//countdown text coordinates
#define COUNTDOWN_Y			200

#define GAME_SPEED 2				//gamespeed. (number of pixels to upate by each frame)

#define GAME_VIEWBORDER		100		//view border. i.e. in an online game once x coordinate of current line falls bellow 100, screen is moved left
#define GAME_START_XDIF		500		//difference in x coordinates for each players in the start of the game
#define GAME_START_YDIF		500		//difference in y coordinates for each players in the start of the game
#define GAME_UPDATE_TO		15		//update timeout for updating coordinates for game. (so faster computers dont update faster)


struct GameVars_Online;

//base class, for directx  based classes.
class baseClass
{
public:
	baseClass( D3DGraphicsClient& rGfx,KeyboardClient& kServ)
		:gfx( rGfx ),
		kbd( kServ )
	{		
	}

public:
	
	virtual ~baseClass()
	{
	}
	
public:
	/*MouseClient mouse;*/
	D3DGraphicsClient& gfx;
	KeyboardClient& kbd;
		


};

/*********Master Game Classes. created directly from master class***/
//abstract master class for game
class GameMaster_Base
{
	friend GameVars_Online;
public:
	
	//main function for running game, called by master class. returns 
	virtual ProgramState Go() = 0;

protected:
	/*game variables. shared between lobby,in-game and countdown instances
	variables global within gamemaster classes that are passed to lobby/ingame/countdown instances*/
	struct GameVars_Base
	{
		

		UINT16 nPlayers;		//number of players
		Player * playerList;	//pointer to a list of players
		
		GameVars_Base(baseClass& bRef);
		baseClass& baseRef;		//reference to directx class

	};

	//countdown abstract class
	class CountDown_Base: public baseClass
	{
	public:
		CountDown_Base(int startSecs,baseClass& bRef);
	
		virtual ProgramState Go() = 0; //returns true if countdown finished 
	protected:
		void DrawCountDown(int x, int y );	//draw the countdown
	
		/*update countdown - increases seconds and milseconds elapsed according 
		to how much time has passed since last update.
		if isChangeDelta is false - seconds and milseconds elapsed are overwritten with time elapsed from
		countdowntimer.gettimemilli().
		if isChangeDelta is true - seconds and milseconds elapsed are incremented with the time returned
		from countdowntimer.gettimemilli()*/	
		void UpdateCountDown( bool isChangeDelta );	

		bool CountDownFinished(); //returns true if countdown has finished. i.e. has reached zero

		byte secondsElapsed;		//no. of seconds elapsed since start
		UINT16 milSecsElapsed;		//milsecs elapsed since last second. cannot be above 1000
		byte startingSeconds;		//amount of seconds countdown starts at
		Timer countdownTimer;
	};

	//abstract class for in the game.
	class InGame_Base : public baseClass
	{
	public:
		InGame_Base( GameVars_Base& pVars, UINT nKeys, bool asignColours );
	
		virtual ~InGame_Base();

		//main function for running game. return state. ie lobby or ingame, and menu if player quits menu.
		ProgramState virtual Go() = 0;
	protected:
		//draws player lines in game
		void DrawPlayerLines();

		/*Calculates if player has changed direction. returns direction out of function.
		DOES NOT set new line*/
		directionState TakeDirectionInput( Player* player, const PlayerDirectionKeys playerKeys);
		
		//set new direction for player. increments linelist for new direction
		void SetNewDirection( UINT16 playerIndex, directionState newDir );

		//checks of player has crashed
		bool BoundaryCollisionCheck(UINT16 playerIndex);
	
		//moves current line along according to player direction
		bool UpdateLine(Player *cPlayer);

		//has game ended? i.e. are less than 2 players alive?
		bool CheckForGameOver();

		//resets game
		void reset();

		void UpdateView(directionState curDir,Line curLine);

		Timer timer;

		UINT16 borderRight;
		UINT16 borderBottom;

		float zoom;
		int screenX;
		int screenY;
		
		GameVars_Base& vars;
		PlayerDirectionKeys * pKeys; //this is removed by the destructor
	};
	
	ProgramState currentState;

	/*virtual function for destroying current instance. declared in each instance of game
	as each game instance will be different. i.e. if state equals programstate::lobby, delete
	pLobby, but cast to clientlobby if the gametype is client*/
	virtual void DeleteInstance( 	
							ProgramState	state,
							void*			pLobby,
							void*			pGame,
							void*			pCountDown ) = 0;
	
};

//offline game master class
class GameMaster_Offline : public GameMaster_Base
{
	

public:
	GameMaster_Offline( baseClass& bRef,int nOfPlayers );

	~GameMaster_Offline();

	void DeleteInstance(	ProgramState	state,
							void*			pLobby,
							void*			pGame,
							void*			pCountDown );

	ProgramState Go();
private:
	
	class CountDown_Offline : CountDown_Base
	{
	public:
		CountDown_Offline( baseClass& bRef,int startingSeconds );

		ProgramState Go();
	private:

	};

	class InGame_Offline : public InGame_Base
	{
	public:
		
		InGame_Offline::InGame_Offline( GameVars_Base& gameVars );

		InGame_Offline::~InGame_Offline();
 
		ProgramState InGame_Offline::Go();

		

	};

	InGame_Offline * game;
	CountDown_Offline * countdown;
	
	GameVars_Base vars;
	
};

//online derived version of game variables struct (GameVars_Base)
struct GameVars_Online : public GameMaster_Base::GameVars_Base
{
	GameVars_Online(baseClass& bRef, GameNetworking& net, char*nameRef);
	virtual ~GameVars_Online();

	
	GameNetworking& netBase;	//game networking reference
	char lobbyName[NAMESIZE+1];	//lobby name

	char * rcvBuf;				//buffer to recieve data over the internet
	UINT16 maxPlayers;			//max number of players in a game
	/*player name. pointer DOES NOT contain name data. data is contained in master class*/
	char * name;				

	void PlayChatSound( int soundIndex );	//play sound (see sounds folder for sound data)
};

//host game master class
class GameMaster_Host : public GameMaster_Base
{
	
public:
	/* slightly confuzzled constructor. maxplayers and lobby name passed as they are inputted when host
	game created. 
	piReply is a pointer to a buffer containing reply from pi server when request to register
	host was sent. if pointer is passed as NULL, then the game is not registered to the pi server and it is
	not connected.
	
	standard parameters required -	hwnd to window
									buttonclient for lobby
									pointer to player name
									reference to game networking class
	*/
	GameMaster_Host(	int maxPlayers,
						ButtonClient& buttonRef,
						HWND hwnd,
						char * lobbyInitName,
						baseClass& bRef,
						char *nameRef,
						GameNetworking& networker,
						char * piReply);
	
	void DeleteInstance(	ProgramState	state,
							void*			pLobby,
							void*			pGame,
							void*			pCountDown );
	

	~GameMaster_Host();

	ProgramState Go();

	
private:
	struct GameVars_Host : GameVars_Online
	{
		GameVars_Host(baseClass& bRef, GameNetworking& net,int noOfMaxPlayers,char*name);
		~GameVars_Host();
		
		/*list of players stored in vector (playerList pointer points to this), can dynamically change 
		as players leave and join. upon changing playerList pointer MUST be updated as vector changes memory location */
		vector<Player> hPlayerList; 
		
		//networking class for host game
		IPComm_Host gameNet;

		
		bool connectedToServer; //connected to pi or not?
		int RecvData( int& playerIndex,sockaddr * pRecvIPData = NULL); //returns communication byte, (0 if no data)
				
				
		void KickPlayer( int playerIndex );
		//sends quit to everyone connected including clients and server
		void SendQuit();					

		 /*sends reply to player upon recieveing request
		in game this will always be JNP::REPLY_INGAME*/
		void SendPlayerReply(  sockaddr* IPDat,byte cmReplyType, int pnPacketNo );

		
	};

	class CountDown_Host : public CountDown_Base
	{
	public:
		CountDown_Host( int startingSecs,GameVars_Host& rVars );

		~CountDown_Host();

		ProgramState Go();

		
	private:
		
		GameVars_Host& vars;
	};
		
	class InGame_Host : InGame_Base
	{
	public:
		InGame_Host( GameVars_Host& gameVars );

		ProgramState Go();
	
	private:
		//sets new direction for player (and creates relevant update for clients)
		void NewDirection( UINT16 pIndex, directionState newDir);

		//upon a player having its Player::PlayerData changing this function initiliases the update to send to clients
		void InitPlrVarChange( UINT16 pIndex );

		//process recv data from clients
		void ProcessRecvData( );
		
		
		GameVars_Host& vars;
		//timer for game sync. once this exceeds the time period for syncing game, sync update packet is created
		JNP::Timeout_Timer SyncTimer;
	};

	class Lobby_Host
	{
	public:
		Lobby_Host(	MenuData& lobbyData, GameVars_Host& dat );
	
		ProgramState LobbyGo();
	private:
		//checks if a given name is the same as any of the player names in vars.playerlist
		bool IsNameTaken( char * checkName );

		/*processes a button being pressed. usually returns programstate::lobby but will return
		ProgramState::MENU upon clicking quit or ProgramState::starting upon clicking start*/
		ProgramState ButtonPressed( int buttonID );
			
		/*deals with various procedures when removing a player from the lobby, from sending updates to
		dealing with the win32 side of the removal*/
		void LobbyRemovePlayer( UINT16 index );

		//more or less the opposite of lobbyremoveplayer
		void LobbyAddPlayer( sockaddr * IPData,const char * data );
		
		//processes request from player. char * data is incoming buffer from player
		void ProcessRequest(sockaddr& recvIPData, char * data );

		//sends a buffer to player which has been accepted. formats sending buffer with lobby data.
		void SendPlayerAcceptance( sockaddr IPDat, int requestID );

		//processes data recieved from clients
		void LobbyRecvData();

		//creates and initialises a chat networking update to send to clients.
		void InitChatUpdate( const char * chatMsg, int pIndex );

		/*formats chat message from format "chatMsg" to "PlayerName: chatMsg". e.g. "hi there" to "steve: hi there"
		Returns index of sound file, if chatMsg was a number within the range of the sound files. if not returns 0. */
		byte FormatChatMessage( char * szMessage,  int playerIndex );
		

		LobbyInt lobby; //lobby interaction object for win32 side of things
		HWND window;
		GameVars_Host& vars;
		
					
	
	};

	MenuData lobbyDat;
	GameVars_Host vars;

	Lobby_Host * lobby;
	CountDown_Host * countdown;
	InGame_Host * game;

};

//client game master class
class GameMaster_Client	: public GameMaster_Base
{
public:
	void DeleteInstance(	ProgramState	state,
							void*			pLobby,
							void*			pGame,
							void*			pCountDown );

	~GameMaster_Client();

	ProgramState Go();
	
	/*muddled constructor as usual
	standard parameters, window, buttonclient, baseclass, name, and networking reference.
	IPrecvBuf is the buffer recieved from the host accepting the client request to join the game*/
	GameMaster_Client( 
		HWND window, ButtonClient& buttonRef,
		baseClass& baseRef,const char * IPrecvBuf, 
		char*name, GameNetworking& networker );



private:
	
	
	struct GameVars_Client : GameVars_Online
	{
		GameVars_Client(baseClass& bRef, GameNetworking& net, char*name, const char * hostReply);

		int RecvData(ProgramState& currentState ); //returns comm type if any, 0 if buffer empty

		//client networking object
		IPComm_Client gameNet;

		//sends quit to host
		void SendQuit();

		/*host socket ID. NOT the the same as JNP::SOCK_HOST which refers to the actual socket ID
		to call when calling networking functions.this simply to refers to the first index in the
		gameNet object which is the connection to the host in the IPComm_Client object*/
		static const int hostSock = 0;

	};

	class Lobby_Client
	{
	public:
		Lobby_Client( MenuData& menuData, GameVars_Client& dat, const char * lobbyData );

		ProgramState LobbyGo();
		ProgramState TranslateRecvData( char * data );
	private:
						
		void ButtonPressed( int buttonID,ProgramState& newState );

		LobbyInt lobby;
		GameVars_Client& vars;
		HWND window;
	};

	class CountDown_Client : CountDown_Base
	{
	public:
		CountDown_Client( UINT32 startSecs ,GameVars_Client& dat );

		ProgramState Go();
		
		ProgramState TranslateRecvData( );
	private:
			

		GameVars_Client& vars;
	};

	class InGame_Client : public InGame_Base
	{
	public:
		InGame_Client( GameVars_Client& gameVars );
		~InGame_Client();
		ProgramState Go();
	private:
		ProgramState ProcessRecvData();

		GameVars_Client& vars;

		int myIndex;

	};

	MenuData lobbyDat;
	GameVars_Client vars;

	InGame_Client * game;
	Lobby_Client * lobby;
	CountDown_Client * countdown;
	

};

#endif