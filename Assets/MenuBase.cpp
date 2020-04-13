#include "MenuBase.h"

ReturnGameVals::ReturnGameVals()
{
	ZeroMemory( lobbyName,NAMESIZE+1);
	IPrecv = NULL;
	
}
ReturnGameVals::~ReturnGameVals()
{
	if( IPrecv )
		delete[] IPrecv;
}

ButtonData::ButtonData(	int menuXCoord,int menuYCoord, int menuButtonWidth, int menuButtonHeight, int menuYButtonDis, int nOfButtons, ...)
	:xCoord(menuXCoord),
	yStart(menuYCoord),
	buttonWidth(menuButtonWidth),
	buttonHeight(menuButtonHeight),
	yButtonDis(menuYButtonDis),
	nButtons(nOfButtons)
{
	int size = sizeof(Button) * nOfButtons;

	buttonList = (Button*) malloc( size );

	va_list vl;
	va_start( vl, nOfButtons );

	
	for( int index = 0; index < nButtons; index++ )
	{

		wchar_t * wideString = va_arg( vl,wchar_t*);
		assert( wideString );
		int buttonID = va_arg( vl,int );
		assert( buttonID );
		Button tempB = Button( wideString,buttonID);
		memcpy(	buttonList + index, &tempB,	sizeof(Button));
	}
	
	va_end( vl );
}
ButtonData::ButtonData( const ButtonData& u )
{
	memcpy( this, &u, sizeof(ButtonData ));

	ButtonData& o = (ButtonData&)u;
	o.buttonList = NULL;
}
ButtonData::~ButtonData()
{
	if( buttonList )
		free( buttonList );

}
ButtonData::Button::Button( wchar_t * display, int buttonIdentifier )
	:wszDisplay(display),
	identifier(buttonIdentifier)
{};
	

BaseMenu::BaseMenu( MenuData& menuDat )
	:
	hwnd(menuDat.window),
	nButtons(menuDat.buttonDat.nButtons),
	buttonRef(menuDat.buttonRef )
{

	buttonHandles = new HWND[nButtons];
	
	
	for( int index = 0; index < nButtons; index++ )
	{
		buttonHandles[index] = CreateMenuButton( menuDat.buttonDat ,index );
													
	}
};
BaseMenu::~BaseMenu()
{
	for( int index = 0; index < nButtons; index++ )
	{
		DestroyWindow( buttonHandles[index] );
	}
	delete[] buttonHandles;
}
int BaseMenu::Go()
{
	return buttonRef.GetButtonPressed();
}
HWND BaseMenu::CreateMenuButton( ButtonData& b , int buttonNumber )
{
	ButtonData::Button& button = b.buttonList[buttonNumber];
	return CreateWindow(	L"button",
							button.wszDisplay,
							WS_VISIBLE | WS_CHILD | BS_CENTER,
							b.xCoord ,		b.yStart + (b.yButtonDis*buttonNumber),
							b.buttonWidth,	b.buttonHeight,
							hwnd,
							(HMENU)button.identifier,
							NULL,NULL);
};

void ButtonServer::NoButtonsPressed()
{
	buttonPressed = 0;
}
ButtonServer::ButtonServer()
{
	NoButtonsPressed();
}
void ButtonServer::SetButtonPressed( int bPressed )
{
	buttonPressed = bPressed;
}

int ButtonClient::GetButtonPressed()
{
	return bServ.buttonPressed;
};
ButtonClient::ButtonClient( ButtonServer& b )
	:bServ(b)
{}

MenuData::MenuData( ButtonClient& bRef , HWND hwnd, ButtonData& buttonData )
	:buttonRef(bRef ),
	window(hwnd),
	buttonDat(buttonData)
{
	int g = 1;
}
MenuData::~MenuData()
{

	if( buttonDat.buttonList )
	{
		free( buttonDat.buttonList );
		buttonDat.buttonList = NULL;
	}
}