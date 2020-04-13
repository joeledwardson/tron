#ifndef BITMAP_H
#define BITMAP_H

#pragma warning(disable: 4996) //disable fopen warning
#include <d3d9.h>	// D3DCOLOR
#include <stdio.h> //FILE
#include <assert.h>//assertions

struct BitmapFileHeader
{

	UINT32 fileSize;
	UINT16 reserved1, reserved2;
	UINT32 offsetToPixelData;
};

struct BitMapInfoHeader
{
	UINT32 headerSize;
	int width, height;
	UINT16 planes;
	UINT16 bits;
	UINT32 compression;
	UINT32 imageSize;
	int xResolution, yResolution;
	UINT32 nColours;
	UINT32 importantColours;
};

void LoadBmp( const char * filename, D3DCOLOR** surface );

struct Pixel24
{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
};

#endif