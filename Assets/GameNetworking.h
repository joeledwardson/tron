#include "Networking.h"

#ifndef GAMENET_H
#define GAMENET_H

//derivation of networking class, to be used in host and client instances
//should only bo declared once
class GameNetworking : public BaseNetworking
 {
 public:


	 
	GameNetworking();
	~GameNetworking();
	//initilaise client socket connection
	void InitClient(unsigned long IPAddress);
	//initilaise host connection. piConn = reference to bool variable changed depending on connection succes to pi.
	//piReply = buffer to store reply from pi server. pass number of players, max number of players, lobby name and current state
	//to other variables
	bool InitHost( bool& piConn, char * szPiReply,UINT16 nPlrs, UINT16 mPlrs,char * szLobName, ProgramState state);
	//destroy client socket
	void DestroyClient();
	//destroy host socket
	void DestroyHost();
	//connect to master server. recvBuf will store server's reply if success
	bool ConnectToServer( char * recvBuf);
	//connect to host. again rcvBuf will store Host's reply on success, pass player name to name
	bool ConnectToHost( unsigned long IPAddress, char * name, char * rcvBuf);
 private:
	 //gets external IP address and stores it in externalIP
	bool GetExternalIP(ULONG& externalIP);

	//gets IP address from host name, if it fails, returns 0
	ULONG GetIpFromHostName( const char * hostName );
	//request ID for sending requests to hosts/server
	 byte requestID;
	 bool hostNameResolved;
};

//client networking class, with only one connection (used for connection to master server and host)
class IPComm_Client : public JNP::IPComm
{
public:
	IPComm_Client( BaseNetworking& net, const char * recvBuf,int socketID );
	
	//recieves data
	int RecvData( char * recvBuf );

	
};

//host networking class
class IPComm_Host : public JNP::IPComm_HostType
{
public:
	static const int serverListID = 1;
	static const int clientListID = 0;

	IPComm_Host(BaseNetworking& net );

	//add client to hContactList
	void AddClient(const  char * recvBuf, sockaddr socketData  );

	//remove client from hContactList
	void RemoveClient( UINT32 contactIndex );

	bool HasClientTimedOut( UINT32 contactIndex );
	bool HasServerTimedOut( );
	
	//recieve data from server
	int RecvServerData( char * rcvBuf );
	//recieve data from client, index of client stored in contactIndex
	int RecvClientData( char * rcvBuf, int& contactIndex, sockaddr * IPRecvData );

	
};


#endif