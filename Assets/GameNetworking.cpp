#include "GameNetworking.h"

GameNetworking::GameNetworking()
	:BaseNetworking( JNP::SOCK_NSOCKETS ),
	requestID(0)
{
	
	ULONG piIPAdress = GetIpFromHostName( JNP::SVR_HOSTNAME );	//translate server hostname to ipaddress
	if( piIPAdress )
	{
		hostNameResolved = true;
		InitClientSocket( JNP::SOCK_SVR,piIPAdress,JNP::PORT_SVR );	//if ip address was found initialise server socket
	}
	else
		hostNameResolved = false;
	
}
GameNetworking::~GameNetworking()
{
	//if hostname not resolved then socket not created
	if( hostNameResolved )
		DestroySocket( JNP::SOCK_SVR );
}
void GameNetworking::InitClient( unsigned long IPAddress )
{
	InitClientSocket( JNP::SOCK_CLIENT,IPAddress,JNP::PORT_GAME );
}
bool GameNetworking::InitHost( bool& piConn,char * szPiReply, UINT16 nPlrs, UINT16 mPlrs,char * szLobName, ProgramState state)
{
	piConn = false;
	bool success = false;
	bool keepRcv = true;

	requestID++;

	

	
	InitHostSocket( JNP::SOCK_HOST,JNP::PORT_GAME );


	unsigned long extIP;
	if( GetExternalIP( extIP ) )
	{
		InitClient( extIP );
		char * testStr = "this is a test string. repeat. this is a test string.";
		int len = strlen(testStr) + 1;
		char * recvBuf = new char[JNP::PS_MAX];
		ZeroMemory( recvBuf,JNP::PS_MAX);
		SendData( JNP::SOCK_CLIENT,len,testStr );
		sockaddr fromAddr;
		JNP::Timeout_Timer timeout( 1000 );
		while(1)
		{
		
			int recvLen = RecieveData( JNP::SOCK_HOST,JNP::PS_MAX,recvBuf,&fromAddr);
			if( recvLen == len )
			{
				if( !strcmp( recvBuf,testStr ))
				{
					success = true;
					break;
				}
			}
			else if( timeout.HasTimedOut() )
			{
				break;
			}
		}
		delete[] recvBuf;
		if( !success )
		{
			DoMessageBox( L"port fowarding not configured correctly",L"error" );
		}

		DestroyClient();
	}
	else
	{
		DoMessageBox( L"unable to access the web.\nplease check internet connection",L"error");
	}

	if( !success )
	{
		DestroyHost();
		return false;
	}

	if( !hostNameResolved )
	{
		DoMessageBox( L"pi server hostname not resolved. could not connect",L"error");
		return success;
	}

	const int maxLen = JNP::BYTE_START + 1 + sizeof( JNP::Host);
	char sndBuf[maxLen];
	ZeroMemory( sndBuf,maxLen);
	sndBuf[JNP::BYTE_CM] = JNP::CM_NORMREQEST;
	sndBuf[JNP::BYTE_PN] = requestID;
	
	int place =  JNP::BYTE_START;

	sndBuf[place] = 66;
	place++;

	place += JNP::Pack_Packet( sndBuf + place, "sddc",
		szLobName,
		mPlrs,
		nPlrs,
		state);

	
	SendData( JNP::SOCK_SVR,place,sndBuf );
	Timer timer;
	while( keepRcv)
	{
		if( timer.GetTimeMilli() > JNP::TO_CLIENT_CONNECT )
		{
			DoMessageBox( L"no response from master server",L"error");
			break;
		}
		int rLen = RecieveData( JNP::SOCK_SVR,100,szPiReply);
		if( rLen )
		{
			if( szPiReply[JNP::BYTE_PN] == requestID )
			{

				switch (szPiReply[JNP::BYTE_CM])
				{
				case JNP::CM_FULL:
					DoMessageBox( L"Host list full",L"error");
					keepRcv = false;
					break;
				case JNP::CM_NAMETAKEN:
					DoMessageBox( L"lobby name taken",L"error");
					keepRcv = false;
					success = false;
					break;
				case JNP::CM_NORMREPLY:
					 keepRcv = false;
					 piConn = true;
					 break;
				default:
					break;
				}

			}
		}

	}

	if( !success )
		DestroyHost();

	return success;
}
void GameNetworking::DestroyHost()
{
	DestroySocket( JNP::SOCK_HOST );
}
void GameNetworking::DestroyClient()
{
	DestroySocket(JNP::SOCK_CLIENT );
}
bool GameNetworking::ConnectToHost(  unsigned long IPAddress, char * name, char * rcvBuf)
{
	
	requestID++;	//increment request ID so no there is no confusion between replies from different requests sent

	InitClient( IPAddress );	//initilaise client

	char buffer[JNP::PS_CH_RQ_C];
	ZeroMemory( buffer,JNP::PS_CH_RQ_C );

	buffer[JNP::BYTE_CM] = JNP::CM_NORMREQEST;	//set comm byte to request byte
	buffer[JNP::BYTE_PN] = requestID;			//set ID byte to the request ID

	//next two bytes contain heartbeat ID and update ID of connector. however, when this function is called
	//the client should be initialising their networking object which means these values will be set to 0
	
	int place = JNP::BYTE_START;

	//copy name into buffer
	memcpy( buffer + place,name,strlen(name)+1);

	ZeroMemory( rcvBuf,JNP::PS_MAX);
	Timer timeout;


	SendData( JNP::SOCK_CLIENT, JNP::PS_CH_RQ_C,buffer);
	
	
	/*sit and wait for a reply. if data recieved translate the reply	*/
	while( timeout.GetTimeMilli() < JNP::TO_CLIENT_CONNECT )
	{
		if( RecieveData( JNP::SOCK_CLIENT,3000,rcvBuf) > 0 )
		{
			if( rcvBuf[JNP::BYTE_PN] == requestID )
			{
				int returnVal = rcvBuf[JNP::BYTE_CM];
				switch(returnVal)
				{
				case JNP::CM_NORMREPLY:	//success!
					return true;
				case JNP::CM_NAMETAKEN:	//client in game has already been taken name
					DoMessageBox( L"name taken",L"error" );
					return false;
				case JNP::CM_FULL:		//game is alrady full
					DoMessageBox( L"host full",L"error");
					return false;
				case JNP::CM_INGAME:	//game is in the countdown or in game
					DoMessageBox( L"host is in game",L"error");
					return false;
				}
			

			}
				
			
		}
	}
	//if function has not exited yet than there was no response from host
	DoMessageBox( L"could not connect, no response",L"error");
	return false;
}
bool GameNetworking::GetExternalIP(ULONG& externalIP)
{

	

	struct IPWebsite
	{
		//szUniquePreIPText - CAN appear more than once. first appearance must directly preceed IP address string
		//afterIPChar - character directly proceeding ip address string
		char  * szUniquePreIPText, * afterIPChar;
		//szUrl - url of site to download
		wchar_t * szURL;
		IPWebsite( wchar_t * szURL, char * szUniquePreIPText, char * afterIPChar )
		{
			this->szURL = szURL;
			this->szUniquePreIPText = szUniquePreIPText;
			this->afterIPChar = afterIPChar;
			assert( strlen(afterIPChar) == 1 );
		}
		bool GetIPFromSite(ULONG& ipAddress)
		{
			char szPath[256];		//path of temp file contaning site HTML
			wchar_t wszPath[256];	//path in widestring form
			GetTempPath(256, wszPath);	//store temporary path
			lstrcatW(wszPath, L"my_ip.txt");	//add file name to path
			wcstombs( szPath,wszPath,lstrlenW(wszPath)+1);	//convert to ANSII string

			//function URLDOwnloadToFIle() if succeeds stores HTML in given path
			if( URLDownloadToFile( 0,szURL,wszPath,0,0 ) == S_OK )
			{
				const int len = 3000;
				char szRead[len];		//buffer to store html
				ZeroMemory(szRead,len );

				char * Token;

				FILE * file = fopen( szPath,"r" );	//open file in read mode
				if( file )
				{
					int preLen = strlen(szUniquePreIPText);	//length of pre string

					fread(szRead,len,1,file );	//move HTML into read buffer

					fclose(file);	//close file
					remove(szPath);	//delete temp file containing html

					//function finds first appearance of pre-string in szRead and returns pointer to store in Token
					Token = strstr(szRead,szUniquePreIPText);	
					if( !Token )
						return false;

					Token = Token + preLen;	//token is incremented to after the pre-string (at the place of the ip address)
					//strtok() splits string where afterIPChar is found. therefore it should return a pointer to just the IP address
					Token = strtok( Token,afterIPChar );	
					if( !Token )
						return false;

					ULONG tempIP = inet_addr( Token );
					if( tempIP == INADDR_NONE || tempIP == 0 )	//if string is not a valid IP address then function has failed
						return false;
					else
					{
						ipAddress = tempIP;
						return true;
					}
				
				}
			}

			return false;

		}
	};


	// pointer to list of sites. 
	IPWebsite * sites;
	const int nSites = 3;
	sites = (IPWebsite*) malloc( sizeof( IPWebsite ) * nSites );

	sites[0] = IPWebsite(		L"http://checkip.dyndns.org/Current IP Check.htm",
								": ",
								"<");
	sites[1] = IPWebsite(		L"http://www.myip.ru/",
								"<TR><TD bgcolor=white align=center valign=middle>",
								"<" );
	sites[2] = IPWebsite(		L"http://www.whatsmyip.us/",
								"class=\"ip\" onclick=\"copyClip()\" onmouseover=\"copyClip()\">\n",
								"<" );
	
	bool success = false;
	//loop through each site and try to get IP address
	for( int index = 0; index < nSites; index++ )
	{
		if( sites[index].GetIPFromSite( externalIP ))
		{
			success = true;
			break;	
			
		}
	}
	//free sites pointer
	free( sites );
	return success;
}
ULONG GameNetworking::GetIpFromHostName( const char * hostName )
{
	
	sockaddr_in theIp;

	hostent * theHost;

	theHost =  	gethostbyname( hostName );

	if( theHost )
	{
		memcpy( (void*) &theIp.sin_addr,theHost->h_addr_list[0],theHost->h_length );

		return theIp.sin_addr.S_un.S_addr;
	}
	else
	{
		DoMessageBox( L"could not resolve server hostname. This is normally because:\n\
1: You are not connected to the internet, or\n\
2: I have forgotten to update the server hostname",L"error");
		return 0;
	}
	
}
bool GameNetworking::ConnectToServer( char * recvBuf)
{
	requestID++;
	if( !hostNameResolved )
	{
		DoMessageBox(L"pi server hostname not resolved to IP",L"error" );
		return false;
	}
		

	const int len = JNP::BYTE_START+1;
	char sndBuf[len];
	ZeroMemory( sndBuf,len );
	sndBuf[JNP::BYTE_CM] = JNP::CM_NORMREQEST;
	sndBuf[JNP::BYTE_PN] = requestID;
	sndBuf[JNP::BYTE_START] = 69;


	SendData( JNP::SOCK_SVR,len,sndBuf);

	
	Timer timer;
	while( timer.GetTimeMilli() < 2000 )
	{
		int len = RecieveData( JNP::SOCK_SVR,1000,recvBuf);
		if( len )
		{
			if( sndBuf[JNP::BYTE_PN] == requestID )
			{
				switch (recvBuf[JNP::BYTE_CM])
				{
				case JNP::CM_NORMREPLY:
					{
							
					}
					return true;
				case JNP::CM_FULL:
					DoMessageBox( L"server busy",L"error");
					return false;
				default:
					break;
				}
			}

		}
	}
	DoMessageBox( L"no response from server.",L"error");
	return false;
		
}

IPComm_Client::IPComm_Client( BaseNetworking& net, const char * recvBuf, int socketID  )
	:IPComm( 1,net )
{
	AddContact_Client( 0,recvBuf,socketID);
}

int IPComm_Client::RecvData( char * recvBuf )
{
	UINT nContacts = contactLists[0].size();
	for( UINT i = 0; i < nContacts; i++ )
	{
		int len = networker.RecieveData( contactLists[0].at(i).socketID,JNP::PS_MAX,recvBuf);
		if( len )
		{
			return contactLists[0].at(i).ProcessRcvData( recvBuf, len );
		}
	}
	return 0;
}


IPComm_Host::IPComm_Host( BaseNetworking& net   )
	:IPComm_HostType( 2,net)
{}
int IPComm_Host::RecvClientData( char * rcvBuf, int& contactIndex, sockaddr * IPRecvData )
{
	assert( IPRecvData );
	
	
	int len = networker.RecieveData( JNP::SOCK_HOST, JNP::PS_MAX,rcvBuf,IPRecvData );
	
	if( !len )
	{
		return false;
	}
	contactIndex = -1;

	int nCLients = contactLists[clientListID].size();
	if( nCLients )
	{
		contactIndex = FindContact( &contactLists[clientListID].at(0),nCLients,IPRecvData);
		if( contactIndex > -1 )
			return contactLists[clientListID].at(contactIndex).ProcessRcvData( rcvBuf,len );
	}
	
	return len;
}
int IPComm_Host::RecvServerData( char * rcvBuf )
{
	assert( contactLists[serverListID].size() == 1);

	int len = networker.RecieveData( contactLists[serverListID].at(0).socketID,JNP::PS_MAX,rcvBuf);

	if( len )
		return contactLists[serverListID].at(0).ProcessRcvData( rcvBuf,len);
	else
		return 0;
}
bool IPComm_Host::HasClientTimedOut( UINT32 contactIndex )
{
	return HasTimedOut( clientListID,contactIndex);
}
bool IPComm_Host::HasServerTimedOut()
{
	assert( contactLists[serverListID].size() == 1);
	return HasTimedOut( serverListID, 0 );
}
void IPComm_Host::AddClient( const  char * recvBuf, sockaddr socketData )
{
	AddContact_Host( clientListID,recvBuf,JNP::SOCK_HOST,socketData);
}
void IPComm_Host::RemoveClient( UINT32 contactIndex  )
{
	RemoveContact( clientListID, contactIndex );
}
