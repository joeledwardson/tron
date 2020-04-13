#include "ServerConnection.h"

ServerWindow::ServerWindow( HWND window,ButtonClient& bRef )
	:buttonDat( bRef,window,SVRBUTTONS),
	buttonList( buttonDat )
{
	extWnds[SVRWND_WID_TITLE] =	CreateWindow(	L"static",
												SVRWND_TITLE_DISP,
												WS_CHILD | WS_VISIBLE | WS_BORDER | SS_EDITCONTROL,
												SVRWND_STD_X,
												SVRWND_TITLE_Y,
												SVRWND_TITLE_WIDTH,
												MENU_BTN_HEIGHT,
												window,
												NULL,					
												NULL,NULL);
	extWnds[SVRWND_WID_HOSTLIST] = CreateWindow(L"listbox",
												L"Server list",
												WS_VISIBLE | WS_CHILD | WS_BORDER,
												SVRWND_STD_X,
												SVRWND_STD_Y,
												SVRWND_HOSTLIST_WIDTH,
												SVRWND_HOSTLIST_HEIGHT,
												window,
												(HMENU)SVRWND_HOSTLIST,
												NULL,NULL);	
	extWnds[SVRWND_WID_HOSTDATA] = CreateWindow(L"static",
												NULL,
												WS_CHILD |WS_VISIBLE | WS_BORDER | SS_EDITCONTROL,
												SVRWND_BUTTONS_X,
												SVRWND_HOSTDATA_Y,
												MENU_BTN_WIDTH,
												SVRWND_HOSTDATA_HEIGHT,
												window,
												(HMENU)SVRWND_HOSTDATA,
												NULL,NULL);


}
int ServerWindow::Go()
{
	return buttonList.Go();
}
void ServerWindow::AddHost(const char * lobName )
{
	wchar_t wszLobName[NAMESIZE+1];
	mbstowcs( wszLobName,lobName,NAMESIZE+1);
	SendMessage( extWnds[ SVRWND_WID_HOSTLIST ],LB_ADDSTRING,NULL,(LPARAM)wszLobName);
}
void ServerWindow::RemoveHost(UINT index )
{
	SendMessage( extWnds[SVRWND_WID_HOSTLIST],LB_DELETESTRING,(WPARAM)index,NULL);
}
int ServerWindow::GetSelServer()
{
	return SendMessage( extWnds[SVRWND_WID_HOSTLIST],LB_GETCURSEL,NULL,NULL);
}
ServerWindow::~ServerWindow()
{
	for( int i = 0; i < SVRWND_N_EXT_WNDS; i++ )
		DestroyWindow( extWnds[i] );
}
void ServerWindow::SetDataWindowText( const char * szMessage )
{
	int len = strlen(szMessage) + 1;
	wchar_t * wszMsg = new wchar_t[len];
	mbstowcs( wszMsg, szMessage, len );
	int g = SendMessage( extWnds[ SVRWND_WID_HOSTDATA ], WM_SETTEXT,NULL,(LPARAM)wszMsg);
	delete[] wszMsg;
}

ServerConnection::ServerConnection( HWND hwnd,D3DGraphicsClient& rGfx,char * playerName,  ButtonClient& bRef,const char * piReply,GameNetworking& networker )
:IPComm( networker, piReply,JNP::SOCK_SVR),
window( hwnd,bRef),
net(networker),
gfx(rGfx),
name(playerName)
{
	recvBuf = new char[ JNP::PS_MAX ];

	

	const char * p = piReply + JNP::BYTE_START;
	UINT16 nHosts;
	p += JNP::UnpackPacket( p, "d", &nHosts );

	for( int i = 0; i < nHosts; i++ )
	{
		p += AddHost( p  );
	}

	if( nHosts == 0 )
		window.SetDataWindowText( "no servers found" );
}
ServerConnection::~ServerConnection()
{
	char szQuit[JNP::BYTE_CM+1];
	szQuit[JNP::BYTE_CM] = JNP::CM_QUIT;
	IPComm.SendPacket(0,0, szQuit,JNP::BYTE_CM+1);
	delete[] recvBuf;
}
int ServerConnection::AddHost( const char * hostDat )
{
	if( hostList.size() == 0 )
		window.SetDataWindowText( "" );

	

	hostList.push_back( JNP::Host() );
	JNP::Host& cur = hostList.back();

	int len = JNP::UnpackPacket( hostDat, "Isddc",
		&cur.IPAddress,
		cur.lobName,
		&cur.maxPlayers,
		&cur.noOfPlayers,
		&cur.state );
	
	window.AddHost( hostList.back().lobName );

	return len;
}
void ServerConnection::RemoveHost( UINT index )
{
	window.RemoveHost( index );
	hostList.erase( hostList.begin() + index );
	if( hostList.size() == 0 )
		window.SetDataWindowText( "no servers found" );
}
ProgramState ServerConnection::Go( ReturnGameVals* dat,MenuState& s )
{
	ProgramState ps = ProgramState::MENU;
	s = MenuState::ServerList;

	RecvData();
	IPComm.SendData();

	if( IPComm.HasTimedOut( 0,0 ))
	{
		DoMessageBox( L"timed out",L"error" );
		s = MenuState::mainmenu;
	}

	int bPressed = window.Go();

	
	int newSelHost = window.GetSelServer();
	if( newSelHost != selHost && newSelHost >= 0 && newSelHost < (int)hostList.size() )
	{
		UpdateDataWindow( hostList[newSelHost] );
	}
	selHost = newSelHost;
	
	if( bPressed ==  SVRWND_CANCEL )
	{
		s = MenuState::mainmenu;
	}
	else if( bPressed == SVRWND_CONNECT )
	{
		selHost = window.GetSelServer();
		if( selHost >= 0 )
		{
			dat->IPrecv = new char[JNP::PS_MAX];
			gfx.Begin_DrawLoadIco( SVRWND_BUTTONS_X,SVRWND_LOADICO_Y );
			t.Parent_BeginFunc( KeepConnection,(void*)this );
			
			bool success = net.ConnectToHost( /*inet_addr( "82.15.13.139")*/hostList[selHost].IPAddress,name,dat->IPrecv);
			
			t.Parent_EndFunc();
			gfx.End_DrawLoadIco();
			if( success )
			{
				dat->type = GameType::CLIENT;
				ps = ProgramState::LOBBY;
			}
			else
			{
				net.DestroyClient();
			}
			
		}
	}
	return ps;
}
void ServerConnection::RecvData()
{
	while(1)
	{
		int len = IPComm.RecvData( recvBuf );
		if( !len )
			break;
		if( len < 0 )
			continue;
		if( recvBuf[JNP::BYTE_CM] == JNP::CM_UPDATE_SND )
		{
			const char * p = recvBuf + JNP::BYTE_START;
			UINT16 hIndex;
			p += JNP::UnpackPacket( p, "d", &hIndex );

			switch (JNP::UPDATE_TYPES( recvBuf[JNP::BYTE_UT] ))
			{
			case JNP::UPDATE_TYPES::ADD_HOST:
				AddHost( recvBuf + JNP::BYTE_START );
				break;
			case JNP::UPDATE_TYPES::REMOVE_HOST:
				RemoveHost( hIndex );
				selHost = window.GetSelServer();
				break;
			case JNP::UPDATE_TYPES::ADD_PLAYER:
				JNP::UnpackPacket( p, "d", &hostList[hIndex].noOfPlayers );
				break;
			case JNP::UPDATE_TYPES::REMOVE_PLAYER:
				JNP::UnpackPacket( p, "d", &hostList[hIndex].noOfPlayers );
				break;
			case JNP::UPDATE_TYPES::CHANGE_STATE:
				hostList[hIndex].state = (ProgramState)((int)*p);
				break;
			default:
				break;
			}
			if( selHost == hIndex )
				UpdateDataWindow( hostList[hIndex] );
		}
	}
}
void ServerConnection::UpdateDataWindow( JNP::Host& h )
{
	char * szState = NULL;
	switch (h.state)
	{
	case LOBBY:
		szState = "lobby";
		break;
	case PLAYING:
		szState = "playing";
		break;
	case COUNTDOWN:
		szState = "countdown";
		break;
	default:
		break;
	}

	assert( szState );
		
	char szBuf[255];
	ZeroMemory( szBuf, 255 );

	sprintf( szBuf, "Number of players: %d\nMax players: %d\nState = %s",h.noOfPlayers,h.maxPlayers,szState);

	window.SetDataWindowText( szBuf );
}
void ServerConnection::KeepConnection( void * pServerConn )
{
	ServerConnection* p = (ServerConnection*) pServerConn;
	while( !p->t.Child_CheckForEnd() )
	{
		p->RecvData();
		p->IPComm.SendData();
	}
	p->t.Child_EndFunc();
}
