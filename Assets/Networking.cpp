
#include "Networking.h"


UINT32 JNP::Pack_Packet(char * szBuf, const char * format,...)
{
	UINT16 d;
	const char * s;
	
	UINT32 i;

	UINT32 len = 0;
	char * p = szBuf;

	va_list l;
	va_start( l,format);


	for( const char * t = format; *t != NULL; t++ )
	{
		char b = *t;
		bool isCaps = 0 != isupper( b );
		b = tolower( b );
		assert( strchr( JNP::ValidFormats,b));
		int varSize;
		switch (b)
		{
		case 'd':
			{
				varSize = sizeof(UINT16);
				d = (UINT16) va_arg( l, int);
				if( !isCaps )
					d = htons( d );
				memcpy( p, &d, varSize );
			}
			break;
		case 's':
			{
				s = va_arg( l,const char*);
				varSize = strlen(s)+1;
				strcpy( p, s );
			}
			break;
		case 'i':
			{
				i = va_arg( l, UINT32 );
				if( !isCaps )
					i = htonl( i );
				varSize = sizeof(UINT32 );
				memcpy( p, &i, varSize );
			}
			break;
		case 'c':
			{
				varSize = sizeof(char);
				*p = (char)va_arg( l, int );
			}
			break;
		default:
			break;
		}
		len += varSize;
		p += varSize;
	}
	return len;
}
int JNP::UnpackPacket( const char * szBuf, const char * format, ... )
{
	int len = 0;
	UINT16 * d;
	char * s;
	UINT32 * i;
	
	const char * p = szBuf;

	va_list l;
	va_start( l,format);

	for( const char * t = format; *t != NULL; t++ )
	{
		char b = *t;
		bool isCaps = 0 != isupper( b );
		b = tolower( b );

		assert( strchr( JNP::ValidFormats,b));
		int varSize;
		switch (b)
		{
		case 'd':
			varSize = sizeof(UINT16);
			d = va_arg( l,UINT16*);
			memcpy( d, p , varSize );
			if( !isCaps )
				*d = ntohs( *d);
			break;
		case 's':
			{
				s = va_arg( l,char*);
				strcpy( s, p );
				varSize = strlen(s)+1;
			}
			break;
		case 'i':
			varSize = sizeof( UINT32 );
			i = va_arg( l, UINT32*);
			memcpy( i, p, varSize );
			if( !isCaps )
				*i = ntohl( *i );
			break;
		case 'c':
			varSize = sizeof(char);
			s = va_arg( l, char*);
			*s = *p;
		default:
			break;
		}
		len += varSize;
		p += varSize;
	}
	return len;
}

JNP::Timeout_Timer::Timeout_Timer( NID_BIG theTimeout )
	:timeout(theTimeout)
{

}
bool JNP::Timeout_Timer::HasTimedOut()
{
	if( timer.GetTimeMilli() < timeout)
		return false;
	else
	{
		timer.ResetWatch();
		return true;
	}
}
void JNP::Timeout_Timer::ResetWatch()
{
	timer.ResetWatch();
}

byte JNP::HeartBeat::GetPacketID()
{
	return packetID;
}
void JNP::HeartBeat::Update()
{
	packetID++;
	sendBuffer[JNP::BYTE_PN] = packetID;
}
JNP::HeartBeat::HeartBeat( )
{
	packetID = 0;
	sendBuffer[JNP::BYTE_CM] = JNP::CM_HEARTBEAT;
}
const char * JNP::HeartBeat::GetPacket()
{
	return sendBuffer;
}

JNP::Update::Update( BYTE& IDOfUpdate,NID updateType  ,const char * szExData, UINT32 len, bool isForceSend)
	:updateID( ++IDOfUpdate),
	 packLen(len + BYTE_START),
	forceSend( isForceSend )
{

	szMessage = NULL;
	szMessage = new char[packLen];
	szMessage[BYTE_CM] = CM_UPDATE_SND;
	szMessage[BYTE_UP] = IDOfUpdate;
	szMessage[BYTE_UT] = updateType;
	if( szExData )
		memcpy( szMessage + BYTE_START,szExData,len);
}
JNP::Update::~Update()
{
	if( szMessage )
		delete[] szMessage;
}
const char * JNP::Update::GetUpdate()
{
	return szMessage;
}
JNP::Update& JNP::Update::operator=( const Update& u )
{
	memcpy( this,&u,sizeof(JNP::Update));
	
	JNP::Update& o = (JNP::Update&)u;
	o.szMessage = NULL;
	
	return *this;
}
JNP::Update::Update( const Update& u )
	:updateID(u.updateID),
	packLen( u.packLen ),
	forceSend( u.forceSend ),
	szMessage(u.szMessage)

{
	Update& o = (Update&)u;
	o.szMessage = NULL;
}

JNP::IPComm::IPComm(UINT numberOfLists, BaseNetworking& net)
: UPTimer( JNP::TO_SENDMILLI ),
  HBTimer( JNP::TO_HEARTBEAT ),
  curUPID(0),
  heartbeat( ),
  networker(net),
  nLists( numberOfLists )
{
	contactLists = new std::vector< IPContact >[nLists];
}
JNP::IPComm::~IPComm()
{
	delete[] contactLists;
}
void JNP::IPComm::SendData()
{
	int nContacts;

	for( UINT32 u = 0; u < updateList.size(); u++ )
	{
		bool updateSent = true;
		for( UINT i = 0; i < nLists; i++ )
		{
			nContacts = contactLists[i].size();
			if( nContacts )
			{
				if( !IsUpdateSent( &contactLists[i].at(0),	nContacts,	u))
					updateSent = false;
			}
		}

		if( updateSent )
		{
			updateList.erase( updateList.begin() + u );
		}
	}
	
	
	for( UINT i = 0; i < nLists; i++ )
	{
		nContacts = contactLists[i].size();
		if( nContacts)
			SendUpdates( &contactLists[i].at(0),nContacts,true);
	}	

	
	if( HBTimer.HasTimedOut() )
	{
		heartbeat.Update();
		for( UINT i = 0; i < nLists; i++ )
		{
			nContacts = contactLists[i].size();
			if( nContacts )
				SendHeartBeats( &contactLists[i].at(0),nContacts);
		}
	}
	if( UPTimer.HasTimedOut() )
	{
		for( UINT i = 0; i < nLists; i++ )
		{
			nContacts = contactLists[i].size();
			if( nContacts )
			{
				SendUpdates( &contactLists[i].at(0),nContacts,false);
				SendUpdateAccepts( &contactLists[i].at(0),nContacts);
			}
		}
	}
}
void JNP::IPComm::AddContact_Client( UINT listIndex, const  char * recvBuf, int sockID )
{
	assert( listIndex < nLists );
	contactLists[listIndex].push_back( IPContact( recvBuf,sockID,false));
}
void JNP::IPComm::RemoveContact( UINT listIndex, UINT contactIndex )
{
	assert( listIndex < nLists);
	std::vector<IPContact>& curList = contactLists[listIndex];
	assert( contactIndex < curList.size() );
	curList.erase( curList.begin() + contactIndex );

}
bool JNP::IPComm::HasTimedOut( UINT listIndex, UINT contactIndex )
{
	assert( listIndex < nLists);
	std::vector<IPContact>& curList = contactLists[listIndex];
	assert( contactIndex < curList.size() );
	return curList[contactIndex].timeout.HasTimedOut();

}
bool JNP::IPComm::IsUpdateSent(IPContact* list,  UINT32 size , UINT32 uIndex )
{


	for(unsigned  int playerIndex = 0; playerIndex < size; playerIndex++ )
	{
		if(list[playerIndex].FindUpdate( updateList[uIndex].updateID,true ) > -1 )
		{
			return false;
		}
	}

	return true;
}
void JNP::IPComm::FormRequest( char * szMessage, int requestID )
{
	szMessage[JNP::BYTE_CM] = CM_NORMREQEST;
	szMessage[JNP::BYTE_PN] = requestID;
	int place = 2;
	szMessage[place] = heartbeat.GetPacketID();
	place++;
	szMessage[place] = curUPID;

}
void JNP::IPComm::AddUpdate( JNP::UPDATE_TYPES updateType,const char * msg, UINT32 len , bool forceSend,UINT  dest)
{
	updateList.push_back( JNP::Update( curUPID, updateType,msg,len,forceSend ));

	for( UINT i = 0; i < nLists; i++ )
	{
		int nContacts = contactLists[i].size();
		if( (dest & (1 << i)) &&
			nContacts)
		{
			InitUpdate( curUPID, &contactLists[i].at(0),nContacts);
		}
	}
}
void JNP::IPComm::InitUpdate(  byte updateID, IPContact * list, int size  )
{
	for( int index = 0; index < size; index++ )
	{
		list[index].updateIDList.push_back( updateID );
	}
}
void JNP::IPComm::SendHeartBeats(  IPContact* list,  UINT32 size  )
{
	for(UINT32 contact = 0; contact < size; contact++ )
	{
		networker.SendData(
			list[contact].socketID,
			HeartBeat::size,
			heartbeat.GetPacket( ),
			list[contact].ContactIPData() );
	}
}
void JNP::IPComm::SendPacket( UINT listIndex, UINT contactIndex , char * szBuffer, UINT32 len )
{
	assert( listIndex < nLists);
	std::vector<IPContact>& curList = contactLists[listIndex];
	assert( contactIndex < curList.size() );
	networker.SendData( curList[contactIndex].socketID,len,szBuffer,curList[contactIndex].ContactIPData() );
}
void JNP::IPComm::SendUpdates( IPContact* list, UINT32 size , bool forceSend)
{
	for( UINT c = 0; c < size; c++)
	{
		IPContact& cur = list[c];
		for( UINT u = 0; u < updateList.size(); u++ )
		{
			if(	( updateList[u].forceSend && forceSend			)||
				(!updateList[u].forceSend && forceSend == false ))
			{
				networker.SendData( cur.socketID,updateList[u].packLen,updateList[u].GetUpdate(),cur.ContactIPData());
			}
		}
	}
}
void JNP::IPComm::SendUpdateAccepts( IPContact* list, UINT32 size )
{
	const int len = 2;

	char sendingBuf[len];
	sendingBuf[ JNP::BYTE_CM ] = JNP::CM_UPDATE_RCV;

	for( UINT32 contactIndex = 0; contactIndex <size; contactIndex++ )
	{

		for(UINT32 updateIndex = 0; updateIndex <list[contactIndex].updateAcceptList.size(); updateIndex++ )
		{


			sendingBuf[ JNP::BYTE_PN ] =list[contactIndex].updateAcceptList[updateIndex];

			networker.SendData(list[contactIndex].socketID,len,sendingBuf,list[contactIndex].ContactIPData() );
		}
		list[contactIndex].updateAcceptList.clear();
	}
}

JNP::IPComm_HostType::IPComm_HostType( UINT nLists, BaseNetworking& net )
	:IPComm(nLists, net )
{}
JNP::IPComm_HostType::~IPComm_HostType()
{

}
void JNP::IPComm_HostType::FormatReply( char * szBuffer, int requestID )
{
	szBuffer[JNP::BYTE_CM] = JNP::CM_NORMREPLY;

	int place = BYTE_PN;

	szBuffer[place] = requestID;
	place++;

	szBuffer[place] = heartbeat.GetPacketID();
	place++;

	szBuffer[place] = curUPID;
}
int JNP::IPComm_HostType::FindContact( IPContact * list, UINT size, sockaddr * IPData )
{
	for(UINT playerIndex = 0; playerIndex < size; playerIndex++ )
	{
		if( !memcmp( IPData,list[playerIndex].ContactIPData(),sizeof( sockaddr )))
		{
			return playerIndex;
		}

	}
	return -1;
}
void JNP::IPComm_HostType::AddContact_Host( 	UINT listIndex, const  char * recvBuf, int sockID, sockaddr socketData )
{
	assert( listIndex < nLists );
	contactLists[listIndex].push_back( IPContact( recvBuf,sockID,true,&socketData));
}


JNP::IPContact& JNP::IPContact::operator=( const IPContact& c )
{
	this->curHBID = c.curHBID;
	this->curUPID = c.curUPID;
	this->socketData = c.socketData;
	
	memcpy( (void*)&this->hostContact , &c.hostContact, sizeof( this->hostContact ) );
	memcpy( &this->timeout, &c.timeout, sizeof( c.timeout ));
	memcpy( (void*)&this->socketID,&c.socketID,sizeof(c.socketID));
	
	this->updateIDList = c.updateIDList;
	this->updateAcceptList = c.updateAcceptList;
	this->futureUpdates = c.futureUpdates;

	return * this;
}
JNP::IPContact::IPContact(const  char * recvBuf, int sockID, bool isHostContact,sockaddr * s )
	:socketID(sockID),
	timeout(TO_GENERAL),
	hostContact( isHostContact )
{
	if( s )
	{
		assert( hostContact );
		memcpy( &socketData, s, sizeof(sockaddr));
	}

	int place = 2;
	curHBID = recvBuf[place];
	place++;
	curUPID = recvBuf[place];
}
JNP::IPContact::~IPContact()
{

}
int JNP::IPContact::ProcessRcvData( char * recvBuf , int len)
{


	byte cmType = recvBuf[BYTE_CM];
	byte packetNumber = recvBuf[BYTE_PN];
	if( cmType == CM_HEARTBEAT )
	{
		if( IsPacketValid(packetNumber,curHBID))
		{
			curHBID = packetNumber;
			timeout.ResetWatch();
		}
		return -1;

	}
	else if(cmType == CM_UPDATE_RCV )
	{
		timeout.ResetWatch();

			int updateIndex = FindUpdate( packetNumber, true );
			if( updateIndex >= 0 )
			{
				updateIDList.erase( updateIDList.begin() + updateIndex );
			}

		return -1;
	}
	else if( cmType == CM_UPDATE_SND)
	{


			if( FindUpdate( packetNumber,false ) < 0 )
			{
				timeout.ResetWatch();
				updateAcceptList.push_back(packetNumber);
			}


		//if ready to send,send acceptance
		if( IsPacketValid( packetNumber,curUPID ))
		{
			if( packetNumber != byte( curUPID + 1 ))
				bool breakpoint = true;
			curUPID = packetNumber;
			return len;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		timeout.ResetWatch();
		return len;
	}
	/*

	if it is heartbeat byte and packet is valid, reset timeout timer
	if it is update_snd return comm val, but send update acceptance and process update
	if it is update_rcv return -1, process update
	 */

}
int JNP::IPContact::FindUpdate( byte updateID, bool IDListOrRcvSndList )
{
	std::vector<byte>*list;
	if( IDListOrRcvSndList )
		list = &updateIDList;
	else
		list = &updateAcceptList;
	if(list->size() )
	{
		return JNP::FindItem( &(list->at(0)),updateID,sizeof(byte),list->size());
	}
	else
	{
		return -1;
	}
}
bool JNP::IPContact::IsPacketValid( BYTE newPacketID, BYTE curPacketID)
{
	if( newPacketID > curPacketID ||
		( curPacketID - newPacketID ) > 230 )
	{
		return true;
	}
	else
	{
		return false;
	}
}
const sockaddr * JNP::IPContact::ContactIPData()
{
	if( hostContact )
		return &socketData;
	else
		return NULL;
}

BaseNetworking::BaseNetworking(UINT16 numberOfSockets)
	:sockAddrSize(sizeof(sockaddr)),
	nSockets(numberOfSockets)
{

	sockets = new int[nSockets];
	socketTypeList = new SocketType[nSockets];

	ZeroMemory( sockets,sizeof(int)* nSockets );

#if PLATFORM == PLATFORM_WINDOWS
	WSADATA winsockDat;
	RunFunction( WSAStartup( MAKEWORD( 2,2 ) , &winsockDat ),-1,"initializing winsock");
#endif



}
BaseNetworking::~BaseNetworking()
{
	for( UINT32 index = 0; index < nSockets; index++ )
	{
		if( sockets[index] )
		{
#if PLATFORM == PLATFORM_WINDOWS
			RunFunction( closesocket( sockets[index]),-1,"closing sockets");
#else
			RunFunction( close( sockets[index]),-1,"closing sockets" );
#endif
		}
	}

#if PLATFORM == PLATFORM_WINDOWS
	RunFunction( WSACleanup(),-1,"cleaning up winsock");
#endif

	delete[] socketTypeList;
	delete[] sockets;



}
void BaseNetworking::SendData(UINT32 sock,int size, const char* buffer,const sockaddr* toAddr)
{
	int rVal;

	assert( sockets[sock] );
	assert( sock >= 0 && sock < nSockets );


	if( toAddr )
	{
		assert( socketTypeList[sock] == SOCKET_HOST );
		rVal =  sendto(		sockets[sock],
							buffer,
							size ,
							0,
							toAddr,
							sockAddrSize);
	}
	else
	{
		assert( socketTypeList[sock] == SOCKET_CLIENT );
		rVal = send(		sockets[sock],
							buffer,
							size,
							0 );
	}





	RunFunction(	rVal,
					-1 ,
					"sending data" );


}
int BaseNetworking::RecieveData( UINT32 sock,int size,char * recvBuf , sockaddr* fromAddr )
{
	int rVal;

	assert( sockets[sock] );
	assert( sock >= 0 && sock < nSockets );

	if( fromAddr )
	{
		assert( socketTypeList[sock] == SOCKET_HOST );
		rVal = recvfrom(		sockets[sock],
								recvBuf,
								size,
								0,
								fromAddr,
								&sockAddrSize);

	}
	else
	{
		assert( socketTypeList[sock] == SOCKET_CLIENT );
		rVal = recv(		sockets[sock],
							recvBuf,
							size,
							0);
	}







	if( !RunFunction(	rVal,
						-1,
						"Receiving data"))
	{
		return 0;
	}
	else
	{

		return rVal;
	}
}
bool BaseNetworking::RunFunction( int rVal,int wrongVal,const char * szOperation )
{
	if( rVal == wrongVal )
	{
#if PLATFORM == PLATFORM_WINDOWS
		wchar_t wszOperation[255];
		mbstowcs( wszOperation,szOperation,255 );

		int error=  WSAGetLastError();

		if( error == WSAEWOULDBLOCK) //tiemout expected so justt return false without displaying error
			return false;


		LPTSTR msg;
		HMODULE lib = NULL;
		FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        0,
                        (LPCVOID)lib,
                        error,
                        0, // language ID
                        (LPTSTR)&msg,
                        0, // size ignored
                        NULL); // arglist

		wchar_t * errorString = new wchar_t[255];
		ZeroMemory( errorString,sizeof(wchar_t)*255);

		swprintf(errorString,L"error doing: %s\nerror = %s\nprogram terminated",wszOperation,(wchar_t*)msg);




		DoMessageBox( errorString,L"error");
		bool networkError = true;
		assert(!networkError);
		PostQuitMessage(0);
		delete[] errorString;

#else
		if( errno != 11) //not recieved data yet
		{
			printf( "error performing: %s\nerror num = : %s",szOperation,strerror(errno));
			exit(0);
		}
#endif
		return false;
	}
	return true;

}
void BaseNetworking::InitHostSocket(UINT32 sock,  UINT32 port)
{
	assert( sock < nSockets );
	assert( !sockets[sock] );

	sockets[sock] = socket( AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if( sockets[sock] == INVALID_SOCKET )
		RunFunction( -1,-1,"Initialising host socket");

	socketTypeList[sock] = SOCKET_HOST;

	sockaddr_in local;
	memset( &local,0,sockAddrSize);
	local.sin_family = AF_INET;
#if PLATFORM == PLATFORM_WINDOWS
	local.sin_addr.S_un.S_addr = htonl( INADDR_ANY );
#else
	local.sin_addr.s_addr = htonl( INADDR_ANY );
#endif
	local.sin_port = htons(port);

	RunFunction( bind( sockets[sock],(sockaddr*)&local,sockAddrSize),-1,"binding host socket");

#if PLATFORM == PLATFORM_WINDOWS
	DWORD nonBlocking = 1;
	RunFunction( ioctlsocket( sockets[sock],FIONBIO,&nonBlocking),-1,"setting host socket to non blocking" );
#else
	RunFunction( fcntl( sockets[sock], F_SETFL,O_NONBLOCK),-1,"setting socket to non blocking");
#endif
}
void BaseNetworking::InitClientSocket(UINT32 sock, unsigned long IPAddress, UINT32 port )
{
	assert( !sockets[sock] );
	assert( sock < nSockets );



	sockets[sock] = socket( AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if( sockets[sock] == INVALID_SOCKET )
		RunFunction( -1,-1,"initilaising client socket");

	socketTypeList[sock] = SOCKET_CLIENT;
	
	
	sockaddr_in IPData;
	memset( &IPData,0,sockAddrSize);
#if PLATFORM == PLATFORM_WINDOWS
	IPData.sin_addr.S_un.S_addr =  IPAddress ;
#else
	IPData.sin_addr.s_addr = IPAddress;
#endif
	IPData.sin_family = AF_INET;
	IPData.sin_port = htons( port );

	RunFunction( connect(  sockets[sock],(sockaddr*)&IPData,sockAddrSize),-1,"connecting client socket");

#if PLATFORM == PLATFORM_WINDOWS
	DWORD nonBlocking = 1;
	RunFunction( ioctlsocket( sockets[sock],FIONBIO,&nonBlocking),-1,"setting client socket to non blocking" );
#else
	RunFunction( fcntl( sockets[sock], F_SETFL,O_NONBLOCK),-1,"setting socket to non blocking");
#endif
}
void BaseNetworking::DestroySocket( UINT32 sock )
{
	assert( sock < nSockets );
	assert( sockets[sock] );

#if PLATFORM == PLATFORM_WINDOWS
	RunFunction( closesocket( sockets[sock] ),-1,"closing socket");
#else
	RunFunction( close( sockets[sock] ),-1,"closing socket");
#endif

	sockets[sock] = NULL;
}
