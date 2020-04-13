
#ifndef NETWORK
#define NETWORK

#include "Timer.h"
#ifndef INVALID_SOCKET
#define INVALID_SOCKET ~0
#endif


#if PLATFORM == PLATFORM_WINDOWS

#pragma comment(lib, "urlmon.lib")
#include <urlmon.h>
#pragma comment(lib,"ws2_32.lib")
#include <winsock.h>
#include "WinSys.h"
//	**winsys.h**
//#include <assert.h>	 //assertions
//#include <Windows.h> //win32 calls
//#include <process.h> //threading

#else

#define NAMESIZE 20
#define PLATFORM PLATFORM_UNIX

#include <cstdarg>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <cstring>

typedef sockaddr_in * PSOCKADDR_IN;

#endif



#include <stdlib.h>
#include <stdio.h>
#include <vector>


class BaseNetworking;

/******Joel's networking protocol**********/
namespace JNP
{
	static const char * SVR_HOSTNAME = "joelyboy94.zapto.org";

	//see Networking.txt for detailed info on sending and receiving data protocols
	typedef const unsigned char		NID; //network ID  type
	typedef const UINT16	        NID_BIG;

	//socket identifiers to pass to send and receive functions
	NID SOCK_NSOCKETS				= 3;
	NID SOCK_HOST					= 0;
	NID SOCK_CLIENT					= 1;
	NID SOCK_SVR					= 2;

	NID SOCK_SVR_NSOCKETS			= 1;
	NID SOCK_SVR_STDSOCK			= 0;

	NID MAX_CLIENT_RQST				= 100; //max client requests at a time
	NID MAX_HOSTS					= 100; //max hosts at a time


	NID BYTE_CM						= 0; //position 1 in array is Communications byte
	/**** heartbeat packets ***/
	NID BYTE_PN						= 1; //packet number byte. increments as program continues
	/*** update packets********/
	NID BYTE_UP						= 1; //update byte
	NID BYTE_UT						= 2; //update type
	/*********************/
	NID BYTE_START					= 4; //position for start of packet after identifiers

	/******************  port IDs  ******************/
	NID_BIG PORT_GAME 				= 33333; //game comm  ip port
	NID_BIG PORT_SVR				= 33334; //sever comm ip port
	/************************************************/

	/******* timeouts  protocol (TO)*****/
	NID_BIG TO_SERVER					= 3500;
	NID_BIG	TO_CLIENT_CONNECT			= 2000;
	NID_BIG TO_GENERAL					= 3500;
	NID_BIG TO_SENDMILLI				= 50; //how often the program should send packets. set to 20ms so 50fps
	NID_BIG TO_HEARTBEAT				= 1000; //2fps send, does not require real time updating

	/* general identifiers*/
	NID ID_WRONG					= -1; //wrong identifying byte for game
	/* game IDs */
	NID GAMEID1						= 255;
	NID GAMEID2						= 23;
	NID GAMEID3						= 173;
	NID GAMEID4						= 69;
	/* note that these are appended to sending and processed on receiving in
	SendData and RecieveData, do not attempt to use outside these functions */
	/************************/


	/******* communication protocols (CM). *********/
	/*subdivision of protocols
	standard protocols are CM_NORMREQUEST and CM_NORMCOMM. below are special replies
	format: (id protocol - see above. except replace ID with CM. proceeds with underscore and communication)
	eg. CM_HS_R_FULL: CM_HS_R = ID_HS_R for host server request (server reply to host in this example), but
	replacing ID with CM; this precedes _FULL, 	indicating server is full      */
	NID CM_HEARTBEAT				= 18; //Heartbeat packet


	NID CM_FULL						= 6; // server	-> host reply. host list full
	NID CM_INGAME					= 7;
	NID CM_NAMETAKEN				= 11;// host	-> client. name taken
	NID CM_NORMREPLY				= 19; //acceptance/usual reply for standard communication
	NID CM_NORMREQEST				= 17; //use for normal request, where above does not specify

	NID CM_QUIT						= 8; // host	-> server. game is closing
	NID CM_KICK				 		= 20; //host kicks player

	NID CM_UPDATE_SND				= 21; //new update
	NID CM_UPDATE_RCV				= 22; //Acknowledgement of update

	/************************************************/



	/********* packet size protocol (PS) ************/
	//Maximum sizes for receiving data
	NID PS_S_RECV 					= 60; //server
	NID PS_H_RECV					= 60; //host
	NID PS_C_RECV					= 60; //client

	/****packet specific sizes***
	format: same as ID protocol except with PS_ rather than ID_. append end of identifier with
	underscore and name  sender. e.g for server requests sent from client (ID_CS_R) replace with PS
	gives PS_CS_R. sent from client so PS_CS_R_C. (h=host, c=client, s=server) */
	NID PS_HOST						= 41; //size of each host packet sending to client
	NID COUNTDOWN					= 8;
	NID PS_CS_RQ_C					= 2;
	NID_BIG PS_CS_RQ_S				= 4103;
	NID PS_HS_RQ_H					= 27;
	NID PS_HS_RQ_S					= 2;
	NID PS_HS_CM_H					= 27;
	NID PS_HS_CM_S					= 2;
	NID PS_CH_RQ_C					= 30;
	/*NID PS_CH_RQ_H					= 2;*/
	NID PS_CH_CM_C					= 33;

	struct Host
	{
		UINT16 maxPlayers;
		UINT16 noOfPlayers;
		char lobName[NAMESIZE+1];
		UINT32 IPAddress;
		ProgramState state;
	};

	NID_BIG PS_MAX					= 2000;
	NID_BIG PS_SVR_MAX				= sizeof(Host) + 2 + BYTE_START;
	/************************************************/

	/*** packet destinations *****/
	NID HOST_DEST_CLIENTS			= 1;
	NID HOST_DEST_SERVER			= 2;
	/*****************************/

	template<class type>
	int FindItem( const type * firstEntry,type val,UINT32 memDif, UINT32 nItems)
	{
		const char * p = (const char*)firstEntry;
		for(UINT32 iIndex = 0; iIndex < nItems; iIndex++ )
		{
			if( !memcmp(p,&val,sizeof(type)))
			{
				return iIndex;
			}
			p += memDif;

		}
		return -1;
	}

	

	struct Timeout_Timer
	{
		Timeout_Timer( NID_BIG theTimeout );
		void ResetWatch();
		bool HasTimedOut();
	private:
		NID_BIG timeout;
		Timer timer;

	};

	struct HeartBeat
	{
		static const int size = 2;
		byte GetPacketID();
		void Update();
		HeartBeat( );
		const char * GetPacket();
	private:
		byte packetID;
		char sendBuffer[size];
	};

	enum UPDATE_TYPES
	{
		CHANGE_STATE,
		ADD_PLAYER,
		REMOVE_PLAYER,
		EDIT_LOB_NAME,
		EDIT_MAX_PLYS,
		CHAT_XTND,
		COUNTDOWN_SYNC ,
		CATCHUP,
		NEW_DIR,
		PLR_VAR,
		GAME_SYNC,
		ADD_HOST,
		REMOVE_HOST

	};

	struct Update
	{

		const BYTE updateID;
		Update( BYTE& IDOfUpdate,NID updateType ,const char * szExData, UINT32 len, bool isForceSend);
		~Update();
		Update& operator = ( const Update& u);
		Update( const Update& u );
		const char * GetUpdate( );
		const UINT32 packLen;
		const bool forceSend;
	protected:
		char * szMessage;
	};



	struct IPContact
	{


		IPContact(const char * recvBuf , int sockID, bool isHostContact,sockaddr * s = NULL);

		~IPContact();
		
		IPContact& operator = ( const IPContact& c );

		const int socketID;
		
		const sockaddr * ContactIPData();



		Timeout_Timer timeout;

		
		std::vector<char *>futureUpdates;
		std::vector<byte>updateIDList;// (update list contact has not yet recieved)
		std::vector<byte>updateAcceptList; //update acceptance list. when contact sends update, this list is appended with its id

		int FindUpdate( byte updateID, bool IDListOrRcvSndList );
		bool IsPacketValid(BYTE newPacketID, BYTE curPacketID);

		byte curUPID; // (contact's current Update ID)
		byte curHBID; // (contact's current heartbeat ID)

		//-1 means no further processing required, 0 means no data retrieved, and >0 means further processing required
		int ProcessRcvData( char * rcvBuf, int len ) ;
	private:
		sockaddr socketData;
		const bool hostContact;
	};

	

	class IPComm
	{
	public:
		IPComm(UINT numberOfLists, BaseNetworking& net);
		virtual ~IPComm();

		void SendData();

		void AddContact_Client(	UINT listIndex, const  char * recvBuf, int sockID );
		
		int GetNUpdates()
		{
			return updateList.size();
		}

		void RemoveContact( UINT listIndex, UINT contactIndex );

		bool HasTimedOut( UINT listIndex, UINT contactIndex );

		void SendPacket(  UINT listIndex, UINT contactIndex , char * szBuffer, UINT32 len );

		void FormRequest( char * szMessage, int requestID );

		void AddUpdate( JNP::UPDATE_TYPES updateType, const char * msg, UINT32 len , bool forceSend = false,UINT  dest = 1);
	protected:
		const UINT nLists;

		Timeout_Timer UPTimer;
		Timeout_Timer HBTimer;

		byte curUPID;

		HeartBeat heartbeat;

		

		void SendUpdates( IPContact* list, UINT32 size , bool forceSend);
		void SendUpdateAccepts( IPContact* list, UINT32 size );
		void SendHeartBeats(  IPContact* list,  UINT32 size );
		bool IsUpdateSent( IPContact* list,  UINT32 size, UINT32 uIndex);

		std::vector<Update> updateList;
		std::vector<IPContact> * contactLists;

		BaseNetworking& networker;
		void InitUpdate( byte updateID, IPContact * list, int size );



	};

	class IPComm_HostType : public IPComm
	{
	public:

		IPComm_HostType(UINT nLists, BaseNetworking& net);

		virtual ~IPComm_HostType();

		void AddContact_Host( 	UINT listIndex, const  char * recvBuf, int sockID, sockaddr socketData );
			
		void FormatReply( char * szBuffer, int requestID );
		
	protected:
		
		int FindContact(		IPContact * list, UINT size, sockaddr * IPData );
		
	};

	//c = char, d = unsigned 16 bit decimal, i = unsigned 32 bit decimal s = string
	//normally function will convert host order to network order and visa versa. caps disables this
	static const char * ValidFormats = "cdsi";
	UINT32 Pack_Packet( char * szBuf, const char * format, ... );
	int UnpackPacket( const char * szBuf, const char * format,...);
	

	



};



/*base networking class. used for game interaction for sending/recieving
data and controls address data for server. UDP NON BLOCKING SOCKETS ONLY */
class BaseNetworking
{
public:

	enum SocketType
	{
		SOCKET_HOST,
		SOCKET_CLIENT,
	};

	BaseNetworking(UINT16 nSockets);
	virtual ~BaseNetworking();



	/* sends data (buffer param) to a speficied address and port (toAddr param)
	with NULL char appended. If toAddr is NULL, function uses server
	structure for sending. INCLUDE 4 BYTE GAME INDENTIFYING BYTES */
	void SendData( UINT32 sock, int size,const char* buffer,const sockaddr* toAddr = NULL);
	/* sends data for a bound socket. CANNOT BE HOST SOCKET. */


	/* recieves data from specified location (fromAddr). if fromAddr is NULL, function uses server address structure.
	returns NULL if no data	recieved.
	RETURNS -1 IF FIRST 4 BYTES ARE NOT GAME INDENIFYING BYTES
	returns no of bytes revieced if all goes well
	*/
	int RecieveData(UINT32 sock,int size,char * recvBuf, sockaddr* fromAddr = NULL );





	void InitClientSocket(UINT32 sock, unsigned long IPAddress, UINT32 port );
	void InitHostSocket( UINT32 sock, UINT32 port );

	void DestroySocket( UINT32 sock );


protected:
	

	bool RunFunction( int rVal,int wrongVal,const char * szOperation );

#if PLATFORM == PLATFORM_WINDOWS
	int sockAddrSize;
#else
    socklen_t sockAddrSize;
#endif

	UINT32 nSockets;
	int * sockets;
	SocketType * socketTypeList;

};



#endif
