/****************************************************************************************** 
 *	Chili DirectX Framework Version 12.04.24											  *	
 *	D3DGraphics.cpp																		  *
 *	Copyright 2012 PlanetChili.net														  *
 *																						  *
 *	This file is part of The Chili DirectX Framework.									  *
 *																						  *
 *	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
 *	it under the terms of the GNU General Public License as published by				  *
 *	the Free Software Foundation, either version 3 of the License, or					  *
 *	(at your option) any later version.													  *
 *																						  *
 *	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
 *	GNU General Public License for more details.										  *
 *																						  *
 *	You should have received a copy of the GNU General Public License					  *
 *	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
 ******************************************************************************************/
#include "D3DGraphics.h"

Radian::Radian( double a)
{
	angle = a;
}
Radian::Radian()
{
	angle = 0;
}
Radian::~Radian()
{

}
	
Radian& Radian::operator = (const Radian& r )
{
	this->angle = fmod( abs(r.angle),(double)2 * PI );

	if( r.angle < 0 )
		this->angle = (2*PI) - this->angle ;

	return *this;
}

double CalculateAngle( double xDis, double yDis, bool angletype )
{
	double rawAngle =  atan( abs(yDis/xDis) ); //calculate raw angle in first quadrant
	//second quadrant
	if( xDis <= 0 && yDis >= 0 )
	{
		rawAngle = PI - rawAngle;
	}
	//third quadrant
	else if( xDis <= 0 && yDis < 0 )
	{
		rawAngle += PI;
	}
	//fourth quadrant
	else if( xDis >= 0 && yDis < 0 )
	{
		rawAngle = 2*PI - rawAngle;
	}
	//convert to degrees if necessary
	if( angletype == ANGLETYPE_DEGREES )
		rawAngle *= double( 180 / PI );

	return rawAngle;
}

D3DGraphics::Font::Font(int nCharsPerRow,  const char * filename,int charHeight, int charWidth )
{

	this->nCharsPerRow = nCharsPerRow;
	this->charHeight = charHeight;
	this->charWidth = charWidth;
	LoadBmp( filename, &surface );

}
D3DGraphics::Font& D3DGraphics::Font::operator=( const Font& f)
{
	memcpy( this,&f,sizeof(Font));
	Font& g = (Font&) f;
	g.surface = NULL;
	return *this;
}
D3DGraphics::Font::~Font()
{
	if( surface )
		delete[] surface;
}

Line::Line( UINT16 x1_, UINT16 x2_, UINT16 y1_, UINT16 y2_ )
	:
	x1(x1_),
	x2(x2_),
	y1(y1_),
	y2(y2_)
{};

D3DGraphics::D3DGraphics( HWND hWnd )
	:pSysBuffer(NULL)
{
	HRESULT result;

	backRect.pBits = NULL;
	
	pDirect3D = Direct3DCreate9( D3D_SDK_VERSION );
	assert( pDirect3D != NULL );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp,sizeof( d3dpp ) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    result = pDirect3D->CreateDevice( D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,&d3dpp,&pDevice );
	assert( !FAILED( result ) );

	result = pDevice->GetBackBuffer( 0,0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer );
	assert( !FAILED( result ) );

	pSysBuffer = new D3DCOLOR[ SCREEN_WIDTH*SCREEN_HEIGHT];
	
	fonts = (Font*)malloc( sizeof(Font) * FONT_NFONTS );
	
	Font f1 = Font(	32,"Consolas13x24.bmp",24,13);
	Font f2 = Font(	32,"fixedsys16x28.bmp",28,16);
	
	fonts[0] = f1;
	
	fonts[1] = f2;
	
	
	
	
}
D3DGraphics::~D3DGraphics()
{
	if( pSysBuffer )
		delete[] pSysBuffer;
	if( pDevice )
	{
		pDevice->Release();
		pDevice = NULL;
	}
	if( pDirect3D )
	{
		pDirect3D->Release();
		pDirect3D = NULL;
	}
	if( pBackBuffer )
	{
		pBackBuffer->Release();
		pBackBuffer = NULL;
	}
	for( int index = 0; index < FONT_NFONTS; index++ )
		fonts[index].~Font();
	free(fonts);
}
void D3DGraphics::ClearGraphicsFrame()
{
	
	memset( pSysBuffer, 0xFF , sizeof( D3DCOLOR) * SCREEN_WIDTH * SCREEN_HEIGHT );
	
}
void D3DGraphics::EndGraphicsFrame(RECT * dest)
{
	HRESULT result;

	result = pBackBuffer->LockRect( &backRect,NULL,NULL );
	assert( !FAILED( result ) );

	
	for( int y = 0; y < SCREEN_HEIGHT; y++ )
	{
		memcpy( &((BYTE*)backRect.pBits)[backRect.Pitch*y],
				&pSysBuffer[SCREEN_WIDTH*y] ,
				sizeof(D3DCOLOR)*SCREEN_WIDTH);
	}
	

	result = pBackBuffer->UnlockRect();
	assert( !FAILED( result ) );

	result = pDevice->Present( dest,dest,NULL,NULL );
	assert( !FAILED( result ) );

	
}

D3DGraphicsClient::D3DGraphicsClient( D3DGraphics& ref )
	:rGraphics( ref),
	loadIco( *this ),
	l( *this,RECT())
{
	Begin_DrawLoadIco( 200,200 );
}
D3DCOLOR D3DGraphicsClient::GetPixel( int x, int y)
{
	assert( x >= 0 );
	assert( y >= 0 );
	assert( x < 800 );
	assert( y < 600 );
	return rGraphics.pSysBuffer[ x + (SCREEN_WIDTH * y) ];
}
void D3DGraphicsClient::PutPixelAlpha( int x, int y, D3DCOLOR c )
{
	
	const D3DCOLOR src=	c;
	const D3DCOLOR dst = GetPixel(x,y);
			
	//extract channels
	const byte srcAlpha =	byte((src & 0xFF000000) >> 24	);
	const byte srcRed =		byte((src & 0x00FF0000) >> 16	);
	const byte srcGreen =	byte((src & 0x0000FF00) >> 8	);
	const byte srcBlue =	byte((src & 0x000000FF)			);
		  					
	const byte dstRed =		byte((dst & 0x00FF0000) >> 16	);
	const byte dstGreen =	byte((dst & 0x0000FF00) >> 8	);
	const byte dstBlue =	byte((dst & 0x000000FF)			 );

	//blend channels
	const byte rltRed =		(srcRed*srcAlpha + dstRed*(255-srcAlpha))/255;
	const byte rltGreen =	(srcGreen*srcAlpha + dstGreen*(255-srcAlpha))/255;
	const byte rltBlue =	(srcBlue*srcAlpha + dstBlue*(255-srcAlpha))/255;
			
	//pack channels back into pixel
	PutPixel(x,y,D3DCOLOR_XRGB(rltRed,rltGreen,rltBlue));

}
void D3DGraphicsClient::DrawVLine( int x1, int x2, int y1, int y2,D3DCOLOR color )
{
	int dx = x2 - x1;
	int dy = y2 - y1;

	assert( dx == 0 || dy == 0 );
	assert( x2 >= x1 );
	assert( y2 >= y1 );


	if( dy == 0 && dx == 0 )
	{
		PutPixel( x1,y1,color );
	}
	else if( dx == 0 )
	{
		for( UINT16 index = y1; index <= y2; index++ )
		{
			PutPixel( x1,index,color);
		}
	}
	else if( dy == 0 )
	{
		for( UINT16 index = x1; index <= x2; index++ )
		{
			PutPixel( index,y1,color);
		}
	}
}
void D3DGraphicsClient::PutPixel( int x,int y,int r,int g,int b )
{	
	assert( x >= 0 );
	assert( y >= 0 );
	assert( x < SCREEN_WIDTH );
	assert( y < SCREEN_HEIGHT );
	rGraphics.pSysBuffer[ x + SCREEN_WIDTH * y ] = D3DCOLOR_XRGB( r,g,b );
}
void D3DGraphicsClient::PutPixel( int x,int y,D3DCOLOR c )
{	
	assert( x >= 0 );
	assert( y >= 0 );
	assert( x < SCREEN_WIDTH );
	assert( y < SCREEN_HEIGHT );
	rGraphics.pSysBuffer[ x + SCREEN_WIDTH * y ] = c;
}
void D3DGraphicsClient::DrawVLine( Line line, D3DCOLOR color )
{
	DrawVLine( line.x1,line.x2, line.y1, line.y2 , color );
	

}
void D3DGraphicsClient::DrawVLine_VP( int x, int y, Line line, D3DCOLOR color )
{
	int dx = line.x2 - line.x1;
	int dy = line.y2 - line.y1;

	assert( dx == 0 || dy == 0 );
	assert( line.x2 >= line.x1 );
	assert( line.y2 >= line.y1 );

	INT32 x1,x2,y1,y2;
	x1 = line.x1;
	x2 = line.x2;
	y1 = line.y1;
	y2 = line.y2;
	
	x1 -= x;
	x2 -= x;
	y1 -= y;
	y2 -= y;

	bool x1Small = x1 < 0;
	bool x2Small = x2 < 0;
	bool y1Small = y1 < 0;
	bool y2Small = y2 < 0;
	bool x1Big	 = x1 >= SCREEN_WIDTH;
	bool x2Big	 = x2 >= SCREEN_WIDTH;
	bool y1Big	 = y1 >= SCREEN_HEIGHT;
	bool y2Big	 = y2 >= SCREEN_HEIGHT;

	if( x2Small || x1Big || y2Small || y1Big )
		return;

	if( x1Small )
		x1 = 0;
	if( x2Big )
		x2 = SCREEN_WIDTH - 1;
	if( y1Small )
		y1 = 0;
	if( y2Big )
		y2 = SCREEN_HEIGHT - 1;

	line = Line( x1,x2,y1,y2 );

	DrawVLine( line,color );

}
void D3DGraphicsClient::DrawRectangle( int x, int y, int width, int height, D3DCOLOR colour)
{
	for( int x_ = 0; x_ <= width; x_++ )
	{
		for( int y_ = 0; y_ <= height; y_++ )
		{
			PutPixel(x+x_,y+y_,colour);
		}
	}
}
void D3DGraphicsClient::DrawRectangle( int x, int y, int width, int height, int r, int g, int b)
{
	for( int x_ = 0; x_ <= width; x_++ )
	{
		for( int y_ = 0; y_ <= height; y_++ )
		{
			PutPixel(x+x_,y+y_,r,g,b);
		}
	}
}
void D3DGraphicsClient::DrawBitmap(int x, int y,int width, int height, D3DCOLOR background, D3DCOLOR* pic)
{
	for( int y_ = 0; y_ <= height-3; y_++ )
	{
		for( int x_ = 0; x_ <= width; x_++ )
		{
			if( pic[x_ + y_*width ] != background)
				PutPixel(x+x_,+y+y_,pic[ x_ + y_*width ]);
		}
	}
}
void D3DGraphicsClient::DrawChar( char c,int xOff,int yOff,int fontType,D3DCOLOR color,int enlargment )
{
	assert( fontType >= 0 && fontType < FONT_NFONTS );
	if( c < ' ' || c > '~' )
		return;

	const int sheetIndex = c - ' ';
	const int sheetCol = sheetIndex % rGraphics.fonts[fontType].nCharsPerRow;
	const int sheetRow = sheetIndex / rGraphics.fonts[fontType].nCharsPerRow;
	const int xStart = sheetCol * rGraphics.fonts[fontType].charWidth;
	const int yStart = sheetRow * rGraphics.fonts[fontType].charHeight;
	const int xEnd = xStart + rGraphics.fonts[fontType].charWidth;
	const int yEnd = yStart + rGraphics.fonts[fontType].charHeight;
	const int surfWidth = rGraphics.fonts[fontType].charWidth * rGraphics.fonts[fontType].nCharsPerRow;

	for( int y = yStart; y < yEnd; y++ )
	{
		for( int x = xStart; x < xEnd; x++ )
		{
			if( rGraphics.fonts[fontType].surface[ x + y * surfWidth ] == D3DCOLOR_XRGB( 0,0,0 ) )
			{
				for( int indexY = 0; indexY < enlargment; indexY++ )
				{
					for( int indexX = 0; indexX < enlargment; indexX++ )
					{
						PutPixel(indexX + xOff + (x-xStart)*enlargment,indexY + yOff + (y-yStart)*enlargment,color );
					}
				}
			
			}
		}
	}
}
void D3DGraphicsClient::DrawString( const char* string,int xoff,int yoff,int fontType,D3DCOLOR color,int enlargement )
{
	int xDis = 0;
	int yDis = 0;
	for( int index = 0; string[ index ] != '\0'; index++ )
	{
		DrawChar( string[ index ],xoff + xDis,yoff+yDis,fontType,color,enlargement );
		if( string[index] == '\n' )
		{
			xDis = 0;
			yDis += rGraphics.fonts[fontType].charHeight;
		}
		else
		{
			xDis += rGraphics.fonts[fontType].charWidth;
		}
		
	}
}
D3DCOLOR D3DGraphicsClient::FadePixel( int r, int g, int b, float fade )
{
	int newR = r + int((255 - r)*fade);
	int newG = g + int((255 - g)*fade);
	int newB = b + int((255 - b)*fade);
	return D3DCOLOR_XRGB(newR,newG,newB);
}
D3DCOLOR D3DGraphicsClient::GenRandCol( byte maxCol )
{
	byte r = rand() % maxCol;
	byte g = rand() % maxCol;
	byte b = rand() % maxCol;
	return D3DCOLOR_XRGB( r, g , b );
}
D3DCOLOR D3DGraphicsClient::CalcFadedColour( byte r, byte g, byte b, float fade )
{
		
	byte newR = r + byte((255 - r)*fade);
	byte newG = g + byte((255 - g)*fade);
	byte newB = b + byte((255 - b)*fade);
	return D3DCOLOR_XRGB(newR,newG,newB);
}

void D3DGraphicsClient::Begin_DrawLoadIco( int x, int y )
{
	//set dest positions
	l.dest.left		= x;
	l.dest.right	= x + LOADICO_OUTRAD*2 + 1;
	l.dest.top		= y;
	l.dest.bottom	= y + LOADICO_OUTRAD*2 + 1;
	
	//begin thread
	t.Parent_BeginFunc( DrawLoadIco_Wrap,(void*)&l );
	
}
void D3DGraphicsClient::End_DrawLoadIco()
{
	//end function
	t.Parent_EndFunc();
	//clear framefrom loading icon
	ClearFrame( &l.dest );
}
void D3DGraphicsClient::DrawLoadIco_Wrap( void * loadicoargs )
{
	//format arguments into loadicoargs format
	D3DGraphicsClient::LoadIcoArgs * l = (D3DGraphicsClient::LoadIcoArgs*)loadicoargs;
	//while the parent does not call exit, draw loading icon.
	while( !l->gfx.t.Child_CheckForEnd()  )
	{
		BeginFrame(true);
		l->gfx.loadIco.Draw( l->dest.left,l->dest.top);
		EndFrame( true,&l->dest);
	}
	l->gfx.t.Child_EndFunc();
}

D3DGraphicsClient::Circle::Circle(  int outerRadius, int innerRadius, int red, int green, int blue, D3DGraphicsClient& rGfx    )
	:
	outR(outerRadius),
	inR(innerRadius),
	r(red),
	g(green),
	b(blue),
	c( D3DCOLOR_XRGB(red,green,blue)),
	nValidPoints(0),
	gfx(rGfx)
{

	//initialise point list. nvalid points is set to the number of valid points. yes there will be wasted space in pointlist	
	pointList = new PCPoint[ (outR*2)*(outR*2) ];

	for( int y =  0; y <= outR*2; y++ )
	{
		for( int x =  0; x <= outR*2; x++ )
		{
		
			int r = int( sqrt( pow( (double)  x - outR  , 2 ) + pow( (double) y -outR,2 )) + 0.5);
			if( r <= outR && r >= inR )
			{
				pointList[ nValidPoints ].x = x;
				pointList[ nValidPoints ].y = y;
				pointList[ nValidPoints ].theta = (float)CalculateAngle( y - outR,x - outR );
				nValidPoints++;
						
			}
		}
	}
}
D3DGraphicsClient::Circle::~Circle()
{
	delete[] pointList;
}
void D3DGraphicsClient::Circle::Draw( int x, int y )
{
	for( int index = 0; index < nValidPoints; index++ )
		gfx.PutPixel( x + pointList[index].x , y - pointList[index].y,c );
}

D3DGraphicsClient::LoadingIcon::LoadingIcon(	D3DGraphicsClient& gfx, int outerRadius, int innerRadius )
	:Circle(	outerRadius,
				innerRadius,
				LOAD_RED,
				LOAD_GREEN,
				LOAD_BLUE,
				gfx),
	currentTheta( 2*PI )
{};
void D3DGraphicsClient::LoadingIcon::Draw(int xCoord, int yCoord)
{
	//currentTheta is the angle at which the fade starts
	//fadeTheta is the angle for the duration of fade. e.g. if it was PI/4 the fade would go from solid to white in quarter of a circle
	Update(); //update angle
	const float fadeTheta = PI; // fade duration is semi-circle
	for( int index = 0; index < nValidPoints; index++ )
	{
		float difTheta = pointList[index].theta - currentTheta; //diftheta is angle between point and angle to present
		if( difTheta < 0 )
			difTheta += 2*PI;
		/*if diftheta is in correct region (within semi-circle of display angle) draw point
		with colour faded proportionate to diftheta*/
		if( difTheta >= 0 && difTheta <= fadeTheta )
			gfx.PutPixel(	xCoord + pointList[index].x , 
							yCoord + pointList[index].y, 
							gfx.FadePixel(	r,	g, b,difTheta/fadeTheta ));
	}
}
void D3DGraphicsClient::LoadingIcon::Update()
{
	//update current theta
	currentTheta -= 0.05f;
	if( currentTheta <= 0 )
		currentTheta = 2*PI;
}


