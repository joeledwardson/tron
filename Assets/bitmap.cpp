#include "bitmap.h"


void LoadBmp( const char * filename, D3DCOLOR** surface )
{
	
	FILE* pBmpFile = fopen( filename,"rb" );
	
	char signature[2];
	
	fread( signature, sizeof(char),2,pBmpFile );
	assert( signature[0] == 'B' && signature[1] == 'M' );

	BitmapFileHeader fileHeader;
	fread( &fileHeader, sizeof( fileHeader),1,pBmpFile);

	BitMapInfoHeader infoHeader;
	fread( &infoHeader, sizeof(infoHeader),1,pBmpFile);

	*surface = new D3DCOLOR[ infoHeader.width * infoHeader.height ];
	ZeroMemory( *surface,sizeof(D3DCOLOR)*infoHeader.width*infoHeader.height );

	fseek( pBmpFile, fileHeader.offsetToPixelData,SEEK_SET);

	int nPixels = infoHeader.width * infoHeader.height;
	int nPaddingBytesPerRow = 4 - ((infoHeader.width * 3) % 4);
	
	if( nPaddingBytesPerRow == 4 )
		nPaddingBytesPerRow = 0;

	for(int y = infoHeader.height-1; y >= 0; y--)
	{
		for(int x = 0; x < infoHeader.width; x++ )
		{
			Pixel24 pixel;
			fread( &pixel,sizeof(pixel),1,pBmpFile );
			(*surface)[ x + y*infoHeader.width ] = D3DCOLOR_XRGB(pixel.red,pixel.blue,pixel.blue);
		}
	
		fseek(pBmpFile,nPaddingBytesPerRow,SEEK_CUR);
	
	}
	
	fclose(pBmpFile);

	

}