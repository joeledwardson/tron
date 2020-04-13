/****************************************************************************************** 
 *	Chili DirectX Framework Version 12.04.24											  *	
 *	D3DGraphics.h																		  *
 *	Copyright 2012 PlanetChili <http://www.planetchili.net>								  *
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
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "WinSys.h"
// **winsys.h**
#include <assert.h>	 //assertions
#include <Windows.h> //win32 calls
#include <process.h> //threading
//*************

#include <vector>
#include <math.h>
#include "bitmap.h"
#include <d3d9.h>

//screen dimensions
#define SCREEN_WIDTH	1100
#define SCREEN_HEIGHT	700

//font IDs, to pass to calls to draw fonts
#define FONT_BIG 0
#define FONT_SMALL 1
#define FONT_NFONTS 2

//angle types to pass to calculate angle
#define ANGLETYPE_DEGREES false
#define ANGLETYPE_RADIANS true
#define PI (float) 3.14159265359

//useful pre-defined colours
#define RED D3DCOLOR_XRGB(255,0,0)
#define BLUE D3DCOLOR_XRGB(0,0,255)
#define GREEN D3DCOLOR_XRGB(0,255,0)
#define YELLOW D3DCOLOR_XRGB(180,180,0)

//loading icon colours
#define LOAD_RED 0
#define LOAD_GREEN 127
#define LOAD_BLUE 85
//loading icon dimensions
#define LOADICO_OUTRAD	100
#define LOADICO_INRAD	50

/*calculate an angle. It is not as simple as just calling tan-1( y / x ) for most angles
because of number signing issues*/
double CalculateAngle( double xDis, double yDis, bool angletype = ANGLETYPE_RADIANS );
  

using namespace std;

class Radian
{
public:
	Radian( double a);
	Radian();
	Radian& operator = (const Radian& r );
	~Radian();

private:
	double angle;
};

//Line structure
struct Line
{
	//4 points in a line
	UINT16 x1,x2,y1,y2;
	Line( UINT16 x1_, UINT16 x2_, UINT16 y1_, UINT16 y2_ );
};

class D3DGraphicsClient;

class D3DGraphics
{
	friend D3DGraphicsClient;
	struct Font
	{
		
		int charWidth;
		int charHeight;
		int nCharsPerRow;
	
		D3DCOLOR* surface;
		Font(int nCharsPerRow,  const char * filename,int charHeight, int charWidth );
		//equals operator. important as fonts are copied using = when initialised
		Font& operator= ( const Font& f );
		~Font();
	};
public:
	
	D3DGraphics( HWND hWnd );
	~D3DGraphics();
		
	
	void ClearGraphicsFrame();
	void EndGraphicsFrame(RECT * dest);
	
private:
	D3DCOLOR* pSysBuffer;
	Font	*	fonts;
	IDirect3D9*			pDirect3D;
	IDirect3DDevice9*	pDevice;
	IDirect3DSurface9*	pBackBuffer;
	D3DLOCKED_RECT		backRect;
	
	
};


class D3DGraphicsClient
{
private:
	class Circle
	{
	public:
		Circle(   int outerRadius, int innerRadius, int red, int green, int blue, D3DGraphicsClient& rGfx  );

		~Circle();

		void Draw( int x, int y );
	protected:
		//polar and cartesian point
		struct PCPoint
		{
			short x,y;
			float theta;
		} * pointList;
		int outR,inR; //radii
		int r,g,b;	//colours
		D3DCOLOR c;
		__int64 nValidPoints; //number of valid points
		D3DGraphicsClient& gfx; //reference to d3d client class
	
	};
	//arguments for creating loading icon thread.
	struct LoadIcoArgs
	{
		LoadIcoArgs( D3DGraphicsClient& rGfx, RECT d )
			:gfx(rGfx)
		{
			dest = d;
		}
		D3DGraphicsClient& gfx;
		RECT dest;
	};
	//loading icon class. draws a rotating circle.
	class LoadingIcon : public Circle
	{
	public:
		LoadingIcon(D3DGraphicsClient& gfx,
					int outerRadius = LOADICO_OUTRAD, //default params
					int innerRadius = LOADICO_INRAD );
		//main functions for drawing loadicon
		void Draw(int xCoord, int yCoord);
	private:
		//updates angle of icon
		void Update();
		//current angle
		float currentTheta;
	
	} ;
public:
	
	D3DGraphicsClient( D3DGraphics& ref );
	void PutPixel( int x,int y,int r,int g,int b );			//put pixel using rgb colour
	void PutPixel( int x,int y,D3DCOLOR c );				//put pixel using D3DCOLOR colour
	void PutPixelAlpha( int x, int y, D3DCOLOR c );			//put pixel (calculating alpha with respect to existing pixel)
	D3DCOLOR GetPixel( int x, int y );						//get current pixel
	void DrawVLine( Line line,D3DCOLOR color );				//draw vertical line (can also be horizontal)
	void DrawVLine( int x1, int x2, int y1, int y2,D3DCOLOR color );	//draw vertical line (can also be horizontal)
	void DrawVLine_VP( int x, int y, Line line, D3DCOLOR color );		//draw line variable position, (may be off the screen)
	void DrawRectangle( int x, int y, int width, int height, D3DCOLOR colour);
	void DrawRectangle( int x, int y, int width, int height, int r, int g, int b);
	void DrawBitmap(int x, int y,int width, int height, D3DCOLOR background, D3DCOLOR* pic);
	void DrawChar( char c,int x,int y,int fontType,D3DCOLOR color, int enlargment );				//draw character
	void DrawString( const char* string,int x,int y,int fontType,D3DCOLOR color, int enlargment );	//draw string
	D3DCOLOR FadePixel( int r, int g, int b, float fade );
	static D3DCOLOR GenRandCol( byte maxCol );				//generate random colour, pass number up to 255 for max value
	static D3DCOLOR CalcFadedColour( byte r, byte g, byte b, float fade );	//fade colour 1 > fade var > 0 
	
	
	void Begin_DrawLoadIco( int x, int y );					//start a thread drawing the load icon
	void End_DrawLoadIco();									//end drawing loading icon thread

private:
	
	static void DrawLoadIco_Wrap( void *  loadicoargs);		//static function which draws load icon. paramter should be of format loadicoargs
	LoadingIcon loadIco;									//loading icon object
	Thread_Params t;										//threading object for loading icon
	LoadIcoArgs l;											//loading icon arguments
	D3DGraphics& rGraphics;

	
};

#endif

