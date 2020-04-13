#include "GameNetworking.h"
#include "MenuBase.h"
#include "D3DGraphics.h"

#define SVRWND_N_EXT_WNDS		3
#define SVRWND_WID_TITLE		0
#define SVRWND_WID_HOSTLIST		1
#define SVRWND_WID_HOSTDATA		2

#define SVRWND_TITLE_DISP		L"Host list"

#define SVRWND_TITLE_Y			50
#define SVRWND_TITLE_WIDTH		600

#define SVRWND_HOSTLIST_WIDTH	300
#define SVRWND_HOSTLIST_HEIGHT	500

#define SVRWND_HOSTDATA_Y		550
#define SVRWND_HOSTDATA_HEIGHT	100

#define SVRWND_BUTTONS_X		500

#define SVRWND_STD_X			100
#define SVRWND_STD_Y			150

#define SVRWND_LOADICO_Y		330

#define SVRBUTTONS ButtonData(	SVRWND_BUTTONS_X,	\
								SVRWND_STD_Y,		\
								MENU_BTN_WIDTH,		\
								MENU_BTN_HEIGHT,	\
								MENU_BTN_YDIS,		\
								2,					\
								L"Connect to host",	SVRWND_CONNECT,	\
								L"Cancel",			SVRWND_CANCEL )


class ServerWindow
{
public:
	ServerWindow( HWND window,ButtonClient& bRef );
	~ServerWindow();
	int Go();
	void AddHost(const char * lobName );
	void RemoveHost( UINT index);
	int GetSelServer();
	void SetDataWindowText( const char * szMessage );
private:
	HWND extWnds[SVRWND_N_EXT_WNDS];
	MenuData buttonDat;
	BaseMenu buttonList;
};

class ServerConnection
{
public:
	ServerConnection( HWND window,D3DGraphicsClient& rGfx,char * playerName, ButtonClient& bRef, const char * piReply,GameNetworking& net );
	~ServerConnection();
	ProgramState Go( ReturnGameVals* dat,MenuState& s );
private:
	int AddHost( const char * hostDat );
	void RemoveHost( UINT index );
	void UpdateDataWindow( JNP::Host& h );
	void RecvData();
	static void KeepConnection( void * pServerConn );
	D3DGraphicsClient& gfx;
	GameNetworking& net;
	IPComm_Client IPComm;
	std::vector<JNP::Host> hostList;
	ServerWindow window;
	char * recvBuf;
	int selHost;
	char * name;
	Thread_Params t;
};