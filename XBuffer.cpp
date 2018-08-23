//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the MapSDK Drawing Engine http://www.mapsdk.com
// Copyright (C) Buu Tran Hao buuth@mapsdk.com 
// Contributors:
// Dzung Tran
// Tinh Nguyen
// Phuc Tran
// This source code is distributed under MIT license
// https://opensource.org/licenses/MIT
//*******************************************************************************
#include "stdafx.h"
#include "Defs.h"
#include <utility>
#include <math.h>
#include <algorithm>
#include <vector>
#include "Algorithm.h"
#include <assert.h>


#ifndef _Max
#define _Max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef _Min
#define _Min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
using namespace std;
#define  FT_WIDTH		10
#define  FT_NULL		0	
#define  FT_SOLID		1	
#define  FT_FDIAGONAL	2
#define  FT_CROSS		3
#define  FT_DIAGCROSS	4
#define  FT_BDIAGONAL	5
#define  FT_HORIZONTAL	6
#define  FT_VERTICAL	7
#define  FT_SELECT		9
#define  FT_CHESSBOARD	10
struct edge {
	struct edge *next;
	long yTop, yBot;
	long xNowWhole, xNowNum, xNowDen;
	signed char xNowDir;
	long xNowNumStep;
};
struct HLINE
{
	int nLeft;
	int nRight;
	struct HLINE* next;
};
struct CIRCLEFILL
{
	int nX;
	int cx;
	int cy;
	int yinc;
	int t;
	int w;
	int h;
};


#ifdef USING_FOR_WEB
	#define MAXVERTICAL     2050			// Su dung de export anh lon
#elif (defined(UNDER_CE))
	#define MAXVERTICAL     1000
#elif (defined(OS_ANDROID))
	#define MAXVERTICAL     1000
#else
	#define MAXVERTICAL     2050			// Neu de 2048 se bi loi khi vung ve co kich thuoc 2048x2048
#endif // USING_FOR_WEB
static edge *edgeTable[MAXVERTICAL];
static HLINE *pHLines[MAXVERTICAL];
static CIRCLEFILL* pCirFill[MAXVERTICAL];
static int gAlPha=255;
static int gPattSz=1;

// used for excluding top and bottom horizontal lines when fill polygon
// #ifdef UNDER_CE
// #define		HLINE_NUM_MAX		100
// #else
// #define		HLINE_NUM_MAX		200
// #endif
// static struct 
// {
// 	long nY;
// 	int nLeft;
// 	int nRight;
// } arrHLines[HLINE_NUM_MAX];
// static int nHLineNum;
//

const  Pixel FocusColor = Color(192,192,192);
bool FT_BDIAGONAL_CHECK(int x,int y)
{
	return !((x-y)&7);		
}
bool FT_DIAGCROSS_CHECK(int x,int y)
{
	return (!((x+y)&7)||!((x-y)&7));		
}
bool FT_FDIAGONAL_CHECK(int x,int y)
{
	return !((x+y)&7);
}
bool FT_HORIZONTAL_CHECK(int x,int y)
{
	return !(y&7);
}
bool FT_VERTICAL_CHECK(int x,int y)
{
	return !(x&7);
}
bool FT_CROSS_CHECK(int x,int y)
{
	return (!(x&7)||!(y&7));	
}
bool FT_SELECT_CHECK(int x,int y)
{
	return ((!((x+y)&7)||!((x-y)&7))&&(y%3));				
}
bool FT_CHESSBOARD_CHECK(int x, int y)
{
	return !((x/ gPattSz + y/ gPattSz)&1);
}
bool FT_SOLID_CHECK(int x,int y)
{
	return true;
}

static bool ClipRect(PXSurface xsurface,int &x0,int &y0,int &x1,int &y1)
{
	if(xsurface)
	{
		if(x0<0)
			x0=0;
		if(y0<0)
			y0=0;
		if(x1>xsurface->width)
			x1=xsurface->width;
		if(y1>xsurface->height)
			y1=xsurface->height;		
		return (x0<x1&&y0<y1);		
	}
	return false;
}
void DefaultPixelFunc(const Pixel &src,Pixel &dst)
{
	dst=src;
}
void NOTXORPEN_PixelFunc(const Pixel &src,Pixel &dst)
{
	dst=(dst^src)^0xffff;
}
void NOTPEN_PixelFunc(const Pixel &src,Pixel &dst)
{
	dst^=0xffff;
}
void XORPEN_PixelFunc(const Pixel &src,Pixel &dst)
{
	dst=(dst^src);
}
void AlphaPixelFunc(const Pixel &src,Pixel &dest)
{
	Pixel RB1 = dest & (RED_MASK | BLUE_MASK);
	Pixel G1  = dest & (GREEN_MASK );
	Pixel RB2 = src & (RED_MASK | BLUE_MASK);
	Pixel G2  = src & (GREEN_MASK );

	Pixel RB = RB1 + (((RB2-RB1) * (gAlPha>>3)) >> 5);
	Pixel G  = G1 + (((G2-G1)*(gAlPha>>2))>>6);
	RB &= (RED_MASK | BLUE_MASK);
	G  &= (GREEN_MASK);
	dest=RB | G;
}
static Pixel *GetPixels(PXSurface xsurface,int x,int y)
{
	return (Pixel*)(xsurface->pixel+(x<<1)+y*xsurface->pitch);
}
void _HLine(PXSurface xsurface,int x0,int y0,unsigned l,Pixel color)
{		
	Pixel *pixel=GetPixels(xsurface,x0,y0);
	while(l--)
		xsurface->PixelFunc(color,*pixel++);		
}
static void SetPatternFunc(PXSurface xsurface,int fp)
{
	switch(fp) {
		case FT_BDIAGONAL:
			xsurface->Pattern=FT_BDIAGONAL_CHECK;
			break;			
		case FT_DIAGCROSS:
			xsurface->Pattern=FT_DIAGCROSS_CHECK;
			break;
		case FT_FDIAGONAL:
			xsurface->Pattern=FT_FDIAGONAL_CHECK;
			break;
		case FT_HORIZONTAL:
			xsurface->Pattern=FT_HORIZONTAL_CHECK;
			break;
		case FT_VERTICAL:
			xsurface->Pattern=FT_VERTICAL_CHECK;
			break;
		case FT_CROSS:
			xsurface->Pattern=FT_CROSS_CHECK;
			break;
		case FT_SELECT:
			xsurface->Pattern=FT_SELECT_CHECK;
			break;
		case FT_CHESSBOARD:
			xsurface->Pattern = FT_CHESSBOARD_CHECK;
			break;
		default:
			xsurface->Pattern=FT_SOLID_CHECK;			
			break;
		}	
}
void _HLineEx(PXSurface xsurface,int x0,int y0,int x1,Pixel color)
{		
	Pixel *pixel=GetPixels(xsurface,x0,y0);	
	for(;x0<x1;++x0,++pixel)
	{
		assert(x0 >=0 && y0 >= 0);
		if(xsurface->Pattern(x0,y0))
			xsurface->PixelFunc(color,*pixel);		
	}
}
#ifdef OS_WIN
#include <tchar.h>
#endif // OS_WIN
PXSurface CreateXSurface(uint16_t w,uint16_t h)
{
	PXSurface surface=new XSurface;
	surface->width=w;
	surface->height=h;

#ifdef OS_WIN
	// Create a DIB
    BYTE buffer[sizeof(BITMAPINFOHEADER) + 3 * sizeof(RGBQUAD)];
    
    // Handy POINTers
    BITMAPINFO*       pBMI    = (BITMAPINFO*)buffer;
    BITMAPINFOHEADER* pHeader = &pBMI->bmiHeader;
    DWORD*            pColors = (DWORD*)&pBMI->bmiColors;   
    
    // DIB Header
    pHeader->biSize            = sizeof(BITMAPINFOHEADER);
    pHeader->biWidth           = w;
    pHeader->biHeight          = -h;
    pHeader->biPlanes          = 1;
    pHeader->biBitCount        = 16;
    pHeader->biCompression     = BI_BITFIELDS;
    pHeader->biSizeImage       = w * h * 2;
    pHeader->biXPelsPerMeter   = 0;
    pHeader->biYPelsPerMeter   = 0;
    pHeader->biClrUsed         = 0;
    pHeader->biClrImportant    = 0;
    
    // Color masks
    pColors[0] = RED_MASK;
    pColors[1] = GREEN_MASK;
    pColors[2] = BLUE_MASK;
    
    // Create the DIB
	surface->hbmp = ::CreateDIBSection(NULL, pBMI, DIB_RGB_COLORS, (void**)&surface->pixel, NULL, 0 );	
	if(!surface->hbmp)	{
		DWORD wErr = GetLastError();
		TCHAR str[32];
		_stprintf(str, _T("Error code:%d"), wErr);
		::MessageBox(NULL, str, _T("Create surface error"), MB_ICONERROR);
		delete surface;
		return NULL;
	}
	surface->pitch=(w*2+3) & ~3;
#else
	surface->pitch = (w*2+3) & ~3;
	surface->pixel = new uint8_t[surface->pitch*surface->height];
#endif // OS_WIN
	surface->lastpixel = surface->pixel+surface->pitch*surface->height;	
	surface->PixelFunc = DefaultPixelFunc;
	surface->Pattern = FT_SOLID_CHECK;	
	return surface;
}
#ifdef OS_WIN
HDC CreateDCXSurface(PXSurface xsurface)
{	
	HDC hsurfacedc=::CreateCompatibleDC(0);
	::SelectObject(hsurfacedc,xsurface->hbmp);	
	return hsurfacedc;
}
PXSurface CreateXSurface(HBITMAP hBitmap)
{
	PXSurface xsurface=0;
	if(hBitmap)
	{
		BITMAP bitmap;
		::GetObject( hBitmap, sizeof(BITMAP), &bitmap );
		xsurface=CreateXSurface((uint16_t)bitmap.bmWidth,(uint16_t)bitmap.bmHeight);

		HDC hdc=::GetDC(0);
		HDC hbitmapdc=::CreateCompatibleDC(hdc);
		::SelectObject(hbitmapdc,hBitmap);
		::ReleaseDC(0,hdc);
		HDC hsurfacedc=CreateDCXSurface(xsurface);		
		::BitBlt(hsurfacedc,0,0,bitmap.bmWidth,bitmap.bmHeight,hbitmapdc,0,0,SRCCOPY);
		::DeleteDC(hbitmapdc);
		::DeleteDC(hsurfacedc);
		
	}
	return xsurface;
}
#endif // OS_WIN
inline void PixelAlphaBlend(Pixel src, Pixel &dest,int alpha)
{
	Pixel RB1 = dest & (RED_MASK | BLUE_MASK);
	Pixel G1  = dest & (GREEN_MASK );
	Pixel RB2 = src & (RED_MASK | BLUE_MASK);
	Pixel G2  = src & (GREEN_MASK );

	Pixel RB = RB1 + (((RB2-RB1) * (alpha>>3)) >> 5);
	Pixel G  = G1 + (((G2-G1)*(alpha>>2))>>6);
	RB &= (RED_MASK | BLUE_MASK);
	G  &= (GREEN_MASK);
	dest=RB | G;
}
static void Transform(Pixel *pixel0,Pixel *pixel1,Pixel color)
{
	while(pixel0<pixel1)
		*pixel0++=color;	
}
static void Transform(Pixel *pixel0,Pixel *pixel1,Pixel color,uint8_t alpha)
{
	while(pixel0<pixel1)
		PixelAlphaBlend(color, *pixel0++, alpha);
}
#define a_AAHLineCallBack AAHLine(Pixel *,const Pixel &,const long &,const long &,const long &,const long &,const long &,const Pixel &)
#define a_AAHLineCallBackEx AAHLineEx(Pixel *,const Pixel &,const long &,const long &,const long &,const long &,const long &,const Pixel &, uint8_t)
#define a_AAVLineCallBack AAVLine(Pixel *,const Pixel &,const long &,const long &,const long &,const long &,const Pixel &)
#define a_AAVLineCallBackEx AAVLineEx(Pixel *,const Pixel &,const long &,const long &,const long &,const long &,const Pixel &, uint8_t)
unsigned julery_isqrt(unsigned long val) 
{
	unsigned long temp, g=0, b = 0x8000, bshft = 15;
	do {
		if (val >= (temp = (((g << 1) + b)<<bshft--))) {
	           g += b;
			   val -= temp;
		}
	} while (b >>= 1);
	return g;
}
static int RoundDiv(int a,int b)
{
	return ((a<<1)/b+1)>>1;	
}

inline void AAPixelColor(Pixel *src,const Pixel &dest,const int &t)
{
	if(*src!=dest)
	{
		if(t<252)
		{
			if(t>0)
			{
				Pixel RB1 = dest & RB_MASK;
				Pixel G1  = dest & (GREEN_MASK );
				Pixel RB2 = (*src) & RB_MASK;
				Pixel G2  = (*src) & (GREEN_MASK );

				Pixel RB = RB1 + (((RB2-RB1) * (t>>3)) >> 5);
				Pixel G  = G1 + (((G2-G1)*(t>>2))>>6);
				RB &= RB_MASK;
				G  &= (GREEN_MASK);
				*src=RB | G;
			}
			else
			{
				*src=dest;
			}
		}
	}
}
inline void AAPixelColorEx(Pixel *src,const Pixel &dest,const Pixel &bck,const int &t)
{
	if(*src!=dest)
	{
		if(t<252)
		{
			if(t>0)
			{
				Pixel RB1 = dest & RB_MASK;
				Pixel G1  = dest & (GREEN_MASK );
				Pixel RB2 = bck & RB_MASK;
				Pixel G2  = bck & (GREEN_MASK );

				Pixel RB = RB1 + (((RB2-RB1) * (t>>3)) >> 5);
				Pixel G  = G1 + (((G2-G1)*(t>>2))>>6);
				RB &= RB_MASK;
				G  &= (GREEN_MASK);

				*src=RB | G;
			}
			else
			{
				*src=dest;
			}
		}
		else
		{
			*src=bck;
		}
	}
}
inline uint8_t GetOutCodeEx( const int &x,const int &y,const int &x_Min,const int &y_Min,const int &x_Max,const int &y_Max )
{
    uint8_t code=0;
	if (x < x_Min)
		code = code|0x01;
    else if (x > x_Max)
		code = code|0x02;
	if (y < y_Min)
		code = code|0x04;
    else if (y > y_Max)
		code = code|0x08;
   return code;
}
void CloseXSurface(PXSurface xsurface)
{
	if (xsurface)
	{
#ifdef OS_WIN
		DeleteObject(xsurface->hbmp);
		xsurface->hbmp = NULL;
#else
		if (xsurface->pixel)
		{
			delete [] xsurface->pixel;
			xsurface->pixel = NULL;
		}
#endif // OS_WIN
		delete xsurface;
	}
}

void AttachToSurface(PXSurface pSurface, void* pPixel, int nWidth, int nHeight)
{
	pSurface->width = nWidth;
	pSurface->height = nHeight;
	pSurface->pitch = (nWidth*2+3) & ~3;
	pSurface->pixel = (uint8_t*)pPixel;
	pSurface->lastpixel = pSurface->pixel + pSurface->pitch*pSurface->height;
	pSurface->PixelFunc = DefaultPixelFunc;
	pSurface->Pattern = FT_SOLID_CHECK;
}

void DetachFromSurface(PXSurface pSurface)
{
	pSurface->width = 0;
	pSurface->height = 0;
	pSurface->pitch = 0;
	pSurface->pixel = NULL;
	pSurface->lastpixel = NULL;
	pSurface->PixelFunc = NULL;
}

uint16_t GetXSurfaceWidth(PXSurface xsurface)
{
	return xsurface?xsurface->width:0;
}
uint16_t GetXSurfaceHeight(PXSurface xsurface)
{
	return xsurface?xsurface->height:0;
}
uint16_t GetXSurfacePitch(PXSurface xsurface)
{
	return xsurface?xsurface->pitch:0;
}
void* GetXSurfacebuffer(PXSurface xsurface)
{
	return xsurface?xsurface->pixel:0;
}
void* GetXSurfaceEndBuffer(PXSurface xsurface)
{
	return xsurface?xsurface->lastpixel:0;
}
void ClearXSurface(PXSurface xsurface,const Pixel &color)
{
	if(xsurface&&xsurface->pixel)
	{		
		unsigned  count    = xsurface->pitch>> 2;
		uint32_t* pixels   = (uint32_t*)xsurface->pixel;
		uint32_t  color32  = (color << 16) | (color);
		while (count--)
			*pixels++ = color32;
		uint8_t *linepixel=xsurface->pixel;
		for(linepixel+=xsurface->pitch;linepixel<xsurface->lastpixel;linepixel+=xsurface->pitch)
		{
			memcpy(linepixel,xsurface->pixel,xsurface->pitch);
		}
	}
}
void _HLine(PXSurface xsurface,int x0,int y0,unsigned l,Pixel color,uint8_t alpha)
{		
	Pixel *pixel= GetPixels(xsurface,x0,y0);		
	Pixel *lastpixel=pixel+l;
	for(;pixel<lastpixel;++pixel )
	{		
		PixelAlphaBlend(color,*pixel,alpha);		
	}		
}
void HLine(PXSurface xsurface,int x0,int y0,int x1,Pixel color)
{
	if(xsurface)
	{
		if(x0>x1)
		{
			std::swap(x0,x1);
			++x0;
			++x1;
		}
		if(y0>=0&&y0<xsurface->height)
		{
			if(x0<0)
				x0=0;
			if(x1>xsurface->width)
				x1=xsurface->width;
			if(x1>x0)
			{
				_HLine(xsurface,x0,y0,x1-x0,color);
			}

		}
	}
}
void HLine(PXSurface xsurface,int x0,int y0,int x1,Pixel color,uint8_t alpha)
{
	if(xsurface)
	{
		if(x0>x1)
		{
			std::swap(x0,x1);
			++x0;
			++x1;
		}
		if(y0>=0&&y0<xsurface->height)
		{
			if(x0<0)
				x0=0;
			if(x1>xsurface->width)
				x1=xsurface->width;
			if(x1>x0)
			{
				_HLine(xsurface,x0,y0,x1-x0,color,alpha);
			}

		}
	}
}
void _VLine(PXSurface xsurface,int x0,int y0,int l,Pixel color)
{
	uint8_t *pixel=(uint8_t *)GetPixels(xsurface,x0,y0);
	uint8_t *lastpixel=pixel+l*xsurface->pitch;	
	for(;pixel<lastpixel;pixel+=xsurface->pitch)
	{
		xsurface->PixelFunc(color,*(Pixel*)pixel);		
	}
}
void _VLine(PXSurface xsurface,int x0,int y0,int l,Pixel color,uint8_t alpha)
{
	uint8_t *pixel=(uint8_t *)GetPixels(xsurface,x0,y0);
	uint8_t *lastpixel=pixel+l*xsurface->pitch;	
	for(;pixel<lastpixel;pixel+=xsurface->pitch)
	{
		PixelAlphaBlend(color,*(Pixel*)pixel,alpha);		
	}
}
void VLine(PXSurface xsurface,int x0,int y0,int y1,Pixel color)
{
	if(xsurface)
	{
		if(y0>y1)
		{
			std::swap(y0,y1);
			++y0;
			++y1;
		}
		if(x0>=0&&x0<xsurface->width)
		{
			if(y0<0)
				y0=0;
			if(y1>xsurface->height)
				y1=xsurface->height;
			if(y1>y0)
			{	
				_VLine(xsurface,x0,y0,y1-y0,color);
				
			}

		}
	}
}
void VLine(PXSurface xsurface,int x0,int y0,int y1,Pixel color,uint8_t alpha)
{
	if(xsurface)
	{
		if(y0>y1)
		{
			std::swap(y0,y1);
			++y0;
			++y1;
		}
		if(x0>=0&&x0<xsurface->width)
		{
			if(y0<0)
				y0=0;
			if(y1>xsurface->height)
				y1=xsurface->height;
			if(y1>y0)
			{	
				_VLine(xsurface,x0,y0,y1-y0,color, alpha);
			}

		}
	}
}
inline uint8_t ClipRectEx(int &x1,int &y1,int &x2,int &y2,const int &x_Min,const int &y_Min,const int &x_Max,const int &y_Max)
{
	uint8_t code1 = GetOutCodeEx( x1,y1,x_Min,y_Min,x_Max,y_Max);
	uint8_t code2 = GetOutCodeEx( x2,y2,x_Min,y_Min,x_Max,y_Max);
	while (code1 || code2)
	{
		// Trivial rejection
		if (code1 & code2)
			return false;
		uint8_t code = (code1) ? code1 : code2;
		int x, y;
		if (code & 1)
		{
			// Left edge
			y = (int)(y1 + (double)(y2 - y1) * (x_Min - x1) / (x2 - x1));
			x = x_Min;
		}
		else if (code & 2)
		{
			// Right edge
			y = (int)(y1 + (double)(y2 - y1) * (x_Max - x1) / (x2 - x1));
			x = x_Max;
		}
		else if (code & 4)
		{
			// Top edge
			x = (int)(x1 + (double)(x2 - x1) * (y_Min - y1) / (y2 - y1));
			y = y_Min;
		}
		else if (code & 8)
		{
			// Bottom edge
			x = (int)(x1 + (double)(x2 - x1) * (y_Max - y1) / (y2 - y1));
			y = y_Max;
		}

		if (code == code1)
		{
			x1=x;
			y1=y;				
			code1 = GetOutCodeEx( x,y,x_Min,y_Min,x_Max,y_Max);
		}
		else
		{
			x2=x;
			y2=y;				
			code2 = GetOutCodeEx( x,y,x_Min,y_Min,x_Max,y_Max);
		}
	}	
	return true;
}
void FillRect(PXSurface xsurface,int x0,int y0,int x1,int y1,Pixel color,uint8_t alpha)
{
	if(ClipRect(xsurface,x0,y0,x1,y1))	
	{
		int l=x1-x0;
		if ( (alpha | 0x03) == 0xFF )
		{					
			_HLine(xsurface,x0,y0,l,color);
			uint8_t *pixel=(uint8_t*)GetPixels(xsurface,x0,y0);
			uint8_t *lastpixel=_Min(pixel+(y1-y0)*xsurface->pitch,xsurface->lastpixel);
			l<<=1;
			for(uint8_t *linepixel=pixel+xsurface->pitch;linepixel<lastpixel;linepixel+=xsurface->pitch)
			{
				memcpy(linepixel,pixel,l);
			}
		}
		else
		{				
			for(;y0<y1;++y0)
			{
				_HLine(xsurface,x0,y0,l,color,alpha);
			}
		}
	}
}
static void _AALine(PXSurface xsurface,int x1,int y1,int x2,int y2,Pixel color)
{
	assert(x1 >= 0 && x1 < xsurface->width && y1 >= 0 && y1 < xsurface->height);
	int yinc=xsurface->pitch>>1;
	if(abs(y1-y2)==abs(x1-x2))
	{
		if(y1>y2)
		{
			long tmp=y1;
			y1=y2;
			y2=tmp;
			tmp=x1;
			x1=x2;
			x2=tmp;
		}
		long xyinc;
		Pixel *pixel=GetPixels(xsurface,x1,y1);						
		if(x2>x1)
		{
			xyinc=yinc+1;			
			
		}
		else
		{
			xyinc=yinc-1;			
		}
		for(;y1<=y2;++y1,pixel+=xyinc)
		{
			*pixel=color;			
		}					
	}
	else
	{
		// Deter_Mine increments	
		long x, y, inc,xyinc,dx, dy;	
		Pixel *nextpixel,*lastpixel=(Pixel*)xsurface->lastpixel;
		dx =x2 - x1;
		dy = y2 - y1;	
		dx =x2 - x1;
		dy = y2 - y1;		
		if(abs(dx)>abs(dy))
		{
			//
			if (dx < 0) 
			{			
				x=x2;
				y=y2<<16;
				x2=x1;
			}		
			else
			{
				x=x1;
				y=y1<<16;
			}					
			inc = (dy <<16) / dx;
			if(inc>0)
			{
				xyinc=yinc+1;
			}
			else
			{
				xyinc=-yinc+1;
			}
			x2++;				
			/* TBB: set the last pixel for consistency (<=) */			
			long cury,prevy=y>>16;			
			Pixel *coloraddr=GetPixels(xsurface,x-1,prevy);			

			for(x;x!=x2;x++,y+=inc)
			{					
				cury=y>>16;			
				if(prevy!=cury)
				{
					coloraddr+=xyinc;
					prevy=cury;
				}
				else
				{
					++coloraddr;
				}
				AAPixelColor(coloraddr,color,(y>>8)&0xff);						
				nextpixel=coloraddr+yinc;
				if(nextpixel<lastpixel)
					AAPixelColor(nextpixel,color,(~y>>8)&0xff);			
			}
		}
		else if(dy)
		{			
			if (dy < 0) 
			{
				x=x2<<16;
				y=y2;
				y2=y1;
			}	
			else
			{
				x=x1<<16;
				y=y1;
			}						
			inc = (dx <<16) / dy;	
			if(inc>0)
			{
				xyinc=yinc+1;
			}
			else
			{
				xyinc=yinc-1;
			}
			/* TBB: set the last pixel for consistency (<=) */
			y2++;
			long curx,prevx=x>>16;
			Pixel *coloraddr=GetPixels(xsurface,prevx,y-1);
			for(y;y!=y2;y++,x+=inc) 
			{			
				curx=x>>16;
				if(curx!=prevx)
				{
					prevx=curx;
					coloraddr+=xyinc;
				}
				else
				{
					coloraddr+=yinc;
				}
				AAPixelColor(coloraddr,color,(x >> 8)&0xff);			
				nextpixel=coloraddr+1;
				if(nextpixel<lastpixel)
					AAPixelColor(nextpixel,color,(~x >> 8)&0xff);			
				
			}
		}
	}
}

static void _AALineEx(PXSurface xsurface,int x1,int y1,int x2,int y2,Pixel color,uint8_t alpha)
{
	assert(x1 >= 0 && x1 < xsurface->width && y1 >= 0 && y1 < xsurface->height);
	int yinc=xsurface->pitch>>1;
	Pixel colorAntilias;
	if(abs(y1-y2)==abs(x1-x2))
	{
		if(y1>y2)
		{
			long tmp=y1;
			y1=y2;
			y2=tmp;
			tmp=x1;
			x1=x2;
			x2=tmp;
		}
		long xyinc;
		Pixel *pixel=GetPixels(xsurface,x1,y1);						
		if(x2>x1)
		{
			xyinc=yinc+1;			
			
		}
		else
		{
			xyinc=yinc-1;			
		}
		for(;y1<=y2;++y1,pixel+=xyinc)
		{
			PixelAlphaBlend(color, *pixel, alpha);
		}					
	}
	else
	{
		// Deter_Mine increments	
		long x, y, inc,xyinc,dx, dy;	
		Pixel *nextpixel,*lastpixel=(Pixel*)xsurface->lastpixel;
		dx =x2 - x1;
		dy = y2 - y1;	
		dx =x2 - x1;
		dy = y2 - y1;		
		if(abs(dx)>abs(dy))
		{
			//
			if (dx < 0) 
			{			
				x=x2;
				y=y2<<16;
				x2=x1;
			}		
			else
			{
				x=x1;
				y=y1<<16;
			}					
			inc = (dy <<16) / dx;
			if(inc>0)
			{
				xyinc=yinc+1;
			}
			else
			{
				xyinc=-yinc+1;
			}
			x2++;				
			/* TBB: set the last pixel for consistency (<=) */			
			long cury,prevy=y>>16;			
			Pixel *coloraddr=GetPixels(xsurface,x-1,prevy);			

			for(x;x!=x2;x++,y+=inc)
			{					
				cury=y>>16;			
				if(prevy!=cury)
				{
					coloraddr+=xyinc;
					prevy=cury;
				}
				else
				{
					++coloraddr;
				}

				colorAntilias = *coloraddr;
				AAPixelColor(&colorAntilias,color,(y>>8)&0xff);
				PixelAlphaBlend(colorAntilias, *coloraddr, alpha);
				nextpixel=coloraddr+yinc;
				if(nextpixel<lastpixel)
				{
					colorAntilias = *nextpixel;
					AAPixelColor(&colorAntilias,color,(~y>>8)&0xff);
					PixelAlphaBlend(colorAntilias, *nextpixel, alpha);
				}
			}
		}
		else if(dy)
		{			
			if (dy < 0) 
			{
				x=x2<<16;
				y=y2;
				y2=y1;
			}	
			else
			{
				x=x1<<16;
				y=y1;
			}						
			inc = (dx <<16) / dy;	
			if(inc>0)
			{
				xyinc=yinc+1;
			}
			else
			{
				xyinc=yinc-1;
			}
			/* TBB: set the last pixel for consistency (<=) */
			y2++;
			long curx,prevx=x>>16;
			Pixel *coloraddr=GetPixels(xsurface,prevx,y-1);
			for(y;y!=y2;y++,x+=inc) 
			{			
				curx=x>>16;
				if(curx!=prevx)
				{
					prevx=curx;
					coloraddr+=xyinc;
				}
				else
				{
					coloraddr+=yinc;
				}
				colorAntilias = *coloraddr;				
				AAPixelColor(&colorAntilias,color,(x >> 8)&0xff);
				PixelAlphaBlend(colorAntilias, *coloraddr, alpha);
				nextpixel=coloraddr+1;
				if(nextpixel<lastpixel)
				{
					colorAntilias = *nextpixel;
					AAPixelColor(&colorAntilias,color,(~x >> 8)&0xff);	
					PixelAlphaBlend(colorAntilias, *nextpixel, alpha);
				}
			}
		}
	}
}

static void _Line(PXSurface xsurface,int x0,int y0,int x1,int y1,Pixel color)
{
	assert(x0 >= 0 && x0 < xsurface->width && y0 >= 0 && y0 < xsurface->height);
	int xincr = 1;
	int dx    = x1 - x0;
	if (dx < 0)
	{
		dx    = -dx;
		xincr = -xincr;
	}

	int yincr = xsurface->pitch>>1;
	int dy    = y1 - y0;
	if (dy < 0)
	{
		dy    = -dy;
		yincr = -yincr;
	}

	int xyincr = xincr + yincr;


	// Find the frame buffer address for each POINT
	Pixel* pA = GetPixels( xsurface,x0, y0 );
	Pixel* pB = GetPixels( xsurface,x1, y1 );


	// Draw the line
	if (dx < dy)
	{
		int dpr  = dx<<1;
		int p    = -dy;
		int dpru = p<<1;
		dx = dy >> 1;
		for ( ; dx > 0; --dx )
		{
			xsurface->PixelFunc(color,*pA);		
			xsurface->PixelFunc(color,*pB);		
			p += dpr;
			if (p > 0)
			{
				p += dpru;
				pA += xyincr;
				pB -= xyincr;
			}
			else
			{
				pA += yincr;
				pB -= yincr;
			}
		}
		xsurface->PixelFunc(color,*pA);		
		if (dy & 1)
		{
			xsurface->PixelFunc(color,*pB);
		}
	}
	else
	{
		int dpr  = dy<<1;
		int p    = -dx;
		int dpru = p<<1;
		dy = dx >> 1;
		for ( ; dy > 0; --dy )
		{
			xsurface->PixelFunc(color,*pA);		
			xsurface->PixelFunc(color,*pB);		

			p += dpr;
			if (p > 0)
			{
				p += dpru;
				pA += xyincr;
				pB -= xyincr;
			}
			else
			{
				pA += xincr;
				pB -= xincr;
			}
		}
		xsurface->PixelFunc(color,*pA);	
		if (dx & 1)
			xsurface->PixelFunc(color,*pB);
	}
}
static void _LineEx(PXSurface xsurface,int x0,int y0,int x1,int y1,Pixel color,uint8_t alpha)
{
	assert(x0 >= 0 && x0 < xsurface->width && y0 >= 0 && y0 < xsurface->height);
	int xincr = 1;
	int dx    = x1 - x0;
	if (dx < 0)
	{
		dx    = -dx;
		xincr = -xincr;
	}

	int yincr = xsurface->pitch>>1;
	int dy    = y1 - y0;
	if (dy < 0)
	{
		dy    = -dy;
		yincr = -yincr;
	}

	int xyincr = xincr + yincr;


	// Find the frame buffer address for each POINT
	Pixel* pA = GetPixels( xsurface,x0, y0 );
	Pixel* pB = GetPixels( xsurface,x1, y1 );


	// Draw the line
	if (dx < dy)
	{
		int dpr  = dx<<1;
		int p    = -dy;
		int dpru = p<<1;
		dx = dy >> 1;
		for ( ; dx > 0; --dx )
		{
			PixelAlphaBlend(color, *pA, alpha);
			PixelAlphaBlend(color, *pB, alpha);
			p += dpr;
			if (p > 0)
			{
				p += dpru;
				pA += xyincr;
				pB -= xyincr;
			}
			else
			{
				pA += yincr;
				pB -= yincr;
			}
		}
		PixelAlphaBlend(color, *pA, alpha);
		if (dy & 1)
			PixelAlphaBlend(color, *pB, alpha);
	}
	else
	{
		int dpr  = dy<<1;
		int p    = -dx;
		int dpru = p<<1;
		dy = dx >> 1;
		for ( ; dy > 0; --dy )
		{
			PixelAlphaBlend(color, *pA, alpha);
			PixelAlphaBlend(color, *pB, alpha);

			p += dpr;
			if (p > 0)
			{
				p += dpru;
				pA += xyincr;
				pB -= xyincr;
			}
			else
			{
				pA += xincr;
				pB -= xincr;
			}
		}
		PixelAlphaBlend(color, *pA, alpha);
		if (dx & 1)
			PixelAlphaBlend(color, *pB, alpha);
	}
}
void Line(PXSurface xsurface,int x0,int y0,int x1,int y1,Pixel color,bool bAntialias)
{
	if(x0==x1)
	{
		VLine(xsurface,x0,y0,y1,color);
	}
	else if(y0==y1)
	{
		HLine(xsurface,x0,y0,x1,color);
	}
	else if(xsurface&&x0!=x1&&y0!=y1)
	{
		//
		int xx0 = x0;
		int xx1 = x1;
		int yy0 = y0;
		int yy1 = y1;
		//
		if(ClipRectEx(x0,y0,x1,y1,0,0,xsurface->width-1,xsurface->height-1))
		{
			if(bAntialias)
			{
				_AALine(xsurface,x0,y0,x1,y1,color);
			}
			else
			{
				_Line(xsurface,x0,y0,x1,y1,color);
			}
		}
	}
}
void LineEx(PXSurface xsurface,int x0,int y0,int x1,int y1,Pixel color,uint8_t alpha, bool bAntialias)
{
	if(x0==x1)
	{
		VLine(xsurface,x0,y0,y1,color,alpha);
	}
	else if(y0==y1)
	{
		HLine(xsurface,x0,y0,x1,color,alpha);
	}
	else if(xsurface&&x0!=x1&&y0!=y1)
	{
		//
		int xx0 = x0;
		int xx1 = x1;
		int yy0 = y0;
		int yy1 = y1;
		//
		if(ClipRectEx(x0,y0,x1,y1,0,0,xsurface->width-1,xsurface->height-1))
		{
			if(bAntialias)
			{
				_AALineEx(xsurface,x0,y0,x1,y1,color,alpha);
			}
			else
			{
				_LineEx(xsurface,x0,y0,x1,y1,color,alpha);
			}
		}
	}
}

inline void AAWide_VLine(Pixel *pixel,const Pixel &color,const long &t,const long &x1,const long &x2,const long &w,const Pixel &colorex)
{
	Pixel *pixel1,*pixel2;
	if(x1>=0)
	{
		pixel1=pixel+x1;
		if(x1<w)
		{
			AAPixelColor(pixel1,color,(t>>8)&0xff);
		}
		++pixel1;
	}
	else
	{
		pixel1=pixel;
	}
	if(x2<w)
	{
		pixel2=pixel+x2;
		if(x2>0)
		{
			AAPixelColor(pixel2,color,(~t>>8)&0xff);		
		}
	}
	else
	{
		pixel2=pixel+w;
	}
	for(;pixel1<pixel2;++pixel1)
	{
		*pixel1=color;
	}
}
inline void AAWide_VLineEx(Pixel *pixel,const Pixel &color,const long &t,const long &x1,const long &x2,const long &w,const Pixel &colorex, uint8_t alpha)
{
	Pixel *pixel1,*pixel2;
	Pixel colorAntilias;
	if(x1>=0)
	{
		pixel1=pixel+x1;
		if(x1<w)
		{
			colorAntilias = *pixel1;
			AAPixelColor(&colorAntilias,color,(t>>8)&0xff);
			PixelAlphaBlend(colorAntilias, *pixel1, alpha);
		}
		++pixel1;
	}
	else
	{
		pixel1=pixel;
	}
	if(x2<w)
	{
		pixel2=pixel+x2;
		if(x2>0)
		{
			colorAntilias = *pixel2;
			AAPixelColor(&colorAntilias,color,(~t>>8)&0xff);
			PixelAlphaBlend(colorAntilias, *pixel2, alpha);
		}
	}
	else
	{
		pixel2=pixel+w;
	}
	for(;pixel1<pixel2;++pixel1)
	{
		PixelAlphaBlend(color, *pixel1, alpha);
	}
}
inline void AAWide_HLine(Pixel *pixel,const Pixel &color,const long &t,const long &y1,const long &y2,const long &h,const long &yinc,const Pixel &colorex)
{
	Pixel *pixel1,*pixel2;
	if(y1>=0)
	{
		pixel1=pixel+y1*yinc;
		if(y1<h)
		{
			AAPixelColor(pixel1,color,(t>>8)&0xff);					
		}
		pixel1+=yinc;
	}
	else
	{
		pixel1=pixel;
	}
	if(y2<h)
	{
		pixel2=pixel+y2*yinc;
		if(y2>0)
		{
			AAPixelColor(pixel2,color,(~t>>8)&0xff);		
		}		
	}
	else
	{
		pixel2=pixel+yinc*h;
	}
	for(;pixel1<pixel2;pixel1+=yinc)
	{
		*pixel1=color;
	}	
}
inline void AAWide_HLineEx(Pixel *pixel,const Pixel &color,const long &t,const long &y1,const long &y2,const long &h,const long &yinc,const Pixel &colorex, uint8_t alpha)
{
	Pixel *pixel1,*pixel2;
	Pixel colorAntilias;
	if(y1>=0)
	{
		pixel1=pixel+y1*yinc;
		if(y1<h)
		{
			colorAntilias = *pixel1;
			AAPixelColor(&colorAntilias,color,(t>>8)&0xff);
			PixelAlphaBlend(colorAntilias, *pixel1, alpha);
		}
		pixel1+=yinc;
	}
	else
	{
		pixel1=pixel;
	}
	if(y2<h)
	{
		pixel2=pixel+y2*yinc;
		if(y2>0)
		{
			colorAntilias = *pixel2;
			AAPixelColor(&colorAntilias,color,(~t>>8)&0xff);
			PixelAlphaBlend(colorAntilias, *pixel2, alpha);
		}		
	}
	else
	{
		pixel2=pixel+yinc*h;
	}
	for(;pixel1<pixel2;pixel1+=yinc)
	{
		PixelAlphaBlend(color, *pixel1, alpha);
	}	
}
inline void AAWide_VInLine(Pixel *pixel,const Pixel &color,const long &t,const long &x1,const long &x2,const long &w,const Pixel &colorex)
{
	Pixel *pixel1,*pixel2;
	if(x1>=0)
	{
		pixel1=pixel+x1;
		if(x1<w)
		{
			AAPixelColorEx(pixel1,color,colorex,(t>>8)&0xff);
		}
		++pixel1;
	}
	else
	{
		pixel1=pixel;
	}
	if(x2<w)
	{
		pixel2=pixel+x2;
		if(x2>0)
		{
			AAPixelColorEx(pixel2,color,colorex,(~t>>8)&0xff);		
		}
	}
	else
	{
		pixel2=pixel+w;
	}
	for(;pixel1<pixel2;++pixel1)
	{
		*pixel1=color;
	}
}
inline void AAWide_VInLineEx(Pixel *pixel,const Pixel &color,const long &t,const long &x1,const long &x2,const long &w,const Pixel &colorex,uint8_t alpha)
{
	Pixel *pixel1,*pixel2;
	Pixel colorAntilias;
	if(x1>=0)
	{
		pixel1=pixel+x1;
		if(x1<w)
		{
			colorAntilias = *pixel1;
			AAPixelColorEx(&colorAntilias,color,colorex,(t>>8)&0xff);
			PixelAlphaBlend(colorAntilias, *pixel1, alpha);
		}
		++pixel1;
	}
	else
	{
		pixel1=pixel;
	}
	if(x2<w)
	{
		pixel2=pixel+x2;
		if(x2>0)
		{
			colorAntilias = *pixel2;
			AAPixelColorEx(&colorAntilias,color,colorex,(~t>>8)&0xff);
			PixelAlphaBlend(colorAntilias, *pixel1, alpha);
		}
	}
	else
	{
		pixel2=pixel+w;
	}
	for(;pixel1<pixel2;++pixel1)
	{
		PixelAlphaBlend(color, *pixel1, alpha);
	}
}
inline void AAWide_HInLine(Pixel *pixel,const Pixel &color,const long &t,const long &y1,const long &y2,const long &h,const long &yinc,const Pixel &colorex)
{
	Pixel *pixel1,*pixel2;

	if(y1>=0)
	{
		pixel1=pixel+y1*yinc;
		if(y1<h)
		{
			AAPixelColorEx(pixel1,color,colorex,(t>>8)&0xff);
		}
		pixel1+=yinc;
	}
	else
	{
		pixel1=pixel;
	}
	if(y2<h)
	{
		pixel2=pixel+y2*yinc;
		if(y2>0)
		{
			AAPixelColorEx(pixel2,color,colorex,(~t>>8)&0xff);		
		}		
	}
	else
	{
		pixel2=pixel+yinc*h;
	}
	for(;pixel1<pixel2;pixel1+=yinc)
	{
		*pixel1=color;
	}	
}
inline void AAWide_HInLineEx(Pixel *pixel,const Pixel &color,const long &t,const long &y1,const long &y2,const long &h,const long &yinc,const Pixel &colorex,uint8_t alpha)
{
	Pixel *pixel1,*pixel2;
	Pixel colorAntilias;
	if(y1>=0)
	{
		pixel1=pixel+y1*yinc;
		if(y1<h)
		{
			colorAntilias = *pixel1;
			AAPixelColorEx(&colorAntilias,color,colorex,(t>>8)&0xff);
			PixelAlphaBlend(colorAntilias, *pixel1, alpha);
		}
		pixel1+=yinc;
	}
	else
	{
		pixel1=pixel;
	}
	if(y2<h)
	{
		pixel2=pixel+y2*yinc;
		if(y2>0)
		{
			colorAntilias = *pixel2;
			AAPixelColorEx(&colorAntilias,color,colorex,(~t>>8)&0xff);
			PixelAlphaBlend(colorAntilias, *pixel2, alpha);
		}		
	}
	else
	{
		pixel2=pixel+yinc*h;
	}
	for(;pixel1<pixel2;pixel1+=yinc)
	{
		PixelAlphaBlend(color, *pixel1, alpha);
	}	
}
inline void AAWide_VOutLine(Pixel *pixel,const Pixel &color,const long &t,const long &x1,const long &x2,const long &w,const Pixel &colorex=0)
{	
	if(x1>=0&&x1<w)
	{
		AAPixelColor(pixel+x1,color,(t>>8)&0xff);
	}	
	if(x2>0&&x2<w)
	{
		AAPixelColor(pixel+x2,color,(~t>>8)&0xff);		
	}	
}
inline void AAWide_VOutLineEx(Pixel *pixel,const Pixel &color,const long &t,const long &x1,const long &x2,const long &w, const Pixel &colorex, uint8_t alpha)
{
	Pixel colorAntilias;
	if(x1>=0&&x1<w)
	{
		colorAntilias = *(pixel+x1);
		AAPixelColor(&colorAntilias,color,(t>>8)&0xff);
		PixelAlphaBlend(colorAntilias, *(pixel+x1), alpha);
	}	
	if(x2>0&&x2<w)
	{
		colorAntilias = *(pixel+x2);
		AAPixelColor(&colorAntilias,color,(~t>>8)&0xff);
		PixelAlphaBlend(colorAntilias, *(pixel+x2), alpha);
	}	
}
inline void AAWide_HOutLine(Pixel *pixel,const Pixel &color,const long &t,const long &y1,const long &y2,const long &h,const long &yinc,const Pixel &colorex)
{	
	if(y1>=0&&y1<h)
	{
		AAPixelColor(pixel+y1*yinc,color,(t>>8)&0xff);					
	}	
	if(y2>0&&y2<h)
	{
		AAPixelColor(pixel+y2*yinc,color,(~t>>8)&0xff);		
	}	
}
inline void AAWide_HOutLineEx(Pixel *pixel,const Pixel &color,const long &t,const long &y1,const long &y2,const long &h,const long &yinc,const Pixel &colorex, uint8_t alpha)
{
	Pixel colorAntilias;
	if(y1>=0&&y1<h)
	{
		colorAntilias = *(pixel+y1*yinc);
		AAPixelColor(&colorAntilias,color,(t>>8)&0xff);
		PixelAlphaBlend(colorAntilias, *(pixel+y1*yinc), alpha);
	}	
	if(y2>0&&y2<h)
	{
		colorAntilias = *(pixel+y2*yinc);
		AAPixelColor(&colorAntilias,color,(~t>>8)&0xff);
		PixelAlphaBlend(colorAntilias, *(pixel+y2*yinc), alpha);
	}	
}
inline void AA_Cap_HLine1(Pixel *pixels,const long &y,const long &x1,const long &x2,const long &w,const long &h,const long &yinc,const int &t,const Pixel &color)
{	
	if(y>=0&&y<h&&x1<w&&x2>=0)
	{
		Pixel *cappixel1,*cappixel2;
		long Yinc=y*yinc;
		if(x1>=0)
		{	
			cappixel1=pixels+(Yinc+x1);
			AAPixelColor(cappixel1,color,t);
			++cappixel1;
		}
		else
		{
			cappixel1=pixels+Yinc;
		}
		cappixel2=pixels+(Yinc+_Min((w-1),x2));
		for(;cappixel1<=cappixel2;++cappixel1)
		{
			*cappixel1=color;
		}
	}
}
inline void AA_Cap_HLine1Ex(Pixel *pixels,const long &y,const long &x1,const long &x2,const long &w,const long &h,const long &yinc,const int &t,const Pixel &color,uint8_t alpha)
{	
	if(y>=0&&y<h&&x1<w&&x2>=0)
	{
		Pixel *cappixel1,*cappixel2;
		Pixel colorAntilias;
		long Yinc=y*yinc;
		if(x1>=0)
		{	
			cappixel1=pixels+(Yinc+x1);
			colorAntilias = *cappixel1;
			AAPixelColor(&colorAntilias,color,t);
			PixelAlphaBlend(colorAntilias, *cappixel1, alpha);
			++cappixel1;
		}
		else
		{
			cappixel1=pixels+Yinc;
		}
		cappixel2=pixels+(Yinc+_Min((w-1),x2));
		for(;cappixel1<=cappixel2;++cappixel1)
		{
			PixelAlphaBlend(color, *cappixel1, alpha);
		}
	}
}
void AA_Cap_HLine2(Pixel *pixels,const long &y,const long &x1,const long &x2,const long &w,const long &h,const long &yinc,const int &t,const Pixel &color)
{	
	if(y>=0&&y<h&&x1<w&&x2>=0)
	{
		Pixel *cappixel1,*cappixel2;
		long Yinc=y*yinc;		
		if(x1>=0)
		{	
			cappixel1=pixels+(Yinc+x1);			
		}
		else
		{
			cappixel1=pixels+Yinc;
		}
		if(x2<w)
		{
			cappixel2=pixels+(Yinc+x2);
			AAPixelColor(cappixel2,color,t);
		}
		else
		{
			cappixel2=pixels+(Yinc+w);
		}		
		Transform(cappixel1,cappixel2,color);		
	}	
}
void AA_Cap_HLine2Ex(Pixel *pixels,const long &y,const long &x1,const long &x2,const long &w,const long &h,const long &yinc,const int &t,const Pixel &color,uint8_t alpha)
{	
	if(y>=0&&y<h&&x1<w&&x2>=0)
	{
		Pixel *cappixel1,*cappixel2;
		Pixel colorAntilias;
		long Yinc=y*yinc;		
		if(x1>=0)
		{	
			cappixel1=pixels+(Yinc+x1);			
		}
		else
		{
			cappixel1=pixels+Yinc;
		}
		if(x2<w)
		{
			cappixel2=pixels+(Yinc+x2);
			colorAntilias = *cappixel2;
			AAPixelColor(&colorAntilias,color,t);
			PixelAlphaBlend(colorAntilias, *cappixel2, alpha);
		}
		else
		{
			cappixel2=pixels+(Yinc+w);
		}		
		Transform(cappixel1,cappixel2,color,alpha);
	}	
}
inline void AA_Cap_VLine1(Pixel *pixels,const long &x,const long &y1,const long &y2,const long &w,const long &h,const long &yinc,const int &t,const Pixel &color)
{	
	if(x>=0&&x<w&&y1<h&&y2>=0)
	{
		Pixel *cappixel1,*cappixel2;
		if(y1>=0)
		{	
			cappixel1=pixels+(y1*yinc+x);
			AAPixelColor(cappixel1,color,t);
			cappixel1+=yinc;
		}
		else
		{
			cappixel1=pixels+x;
		}
		cappixel2=pixels+(_Min(y2,h-1)*yinc+x);
		for(;cappixel1<=cappixel2;cappixel1+=yinc)
		{
			*cappixel1=color;
		}
	}	
}
inline void AA_Cap_VLine1Ex(Pixel *pixels,const long &x,const long &y1,const long &y2,const long &w,const long &h,const long &yinc,const int &t,const Pixel &color, uint8_t alpha)
{	
	if(x>=0&&x<w&&y1<h&&y2>=0)
	{
		Pixel *cappixel1,*cappixel2;
		Pixel colorAntilias;
		if(y1>=0)
		{	
			cappixel1=pixels+(y1*yinc+x);
			colorAntilias = *cappixel1;
			AAPixelColor(&colorAntilias,color,t);
			PixelAlphaBlend(colorAntilias, *cappixel1, alpha);
			cappixel1+=yinc;
		}
		else
		{
			cappixel1=pixels+x;
		}
		cappixel2=pixels+(_Min(y2,h-1)*yinc+x);
		for(;cappixel1<=cappixel2;cappixel1+=yinc)
		{
			PixelAlphaBlend(color, *cappixel1, alpha);
		}
	}	
}
void AA_Cap_VLine2(Pixel *pixels,const long &x,const long &y1,const long &y2,const long &w,const long &h,const long &yinc,const int &t,const Pixel &color)
{	
	if(x>=0&&x<w&&y1<h&&y2>=0)
	{
		Pixel *cappixel1,*cappixel2;
		if(y1>=0)
		{	
			cappixel1=pixels+(y1*yinc+x);			
		}
		else
		{
			cappixel1=pixels+x;
		}
		if(y2<h)
		{
			cappixel2=pixels+(y2*yinc+x);
			AAPixelColor(cappixel2,color,t);
		}
		else
		{
			cappixel2=pixels+(h*yinc+x);
		}		
		for(;cappixel1<cappixel2;cappixel1+=yinc)
		{
			*cappixel1=color;
		}
	}	
}
void AA_Cap_VLine2Ex(Pixel *pixels,const long &x,const long &y1,const long &y2,const long &w,const long &h,const long &yinc,const int &t,const Pixel &color,uint8_t alpha)
{	
	if(x>=0&&x<w&&y1<h&&y2>=0)
	{
		Pixel *cappixel1,*cappixel2;
		Pixel colorAntilias;
		if(y1>=0)
		{	
			cappixel1=pixels+(y1*yinc+x);			
		}
		else
		{
			cappixel1=pixels+x;
		}
		if(y2<h)
		{
			cappixel2=pixels+(y2*yinc+x);
			colorAntilias = *cappixel2;
			AAPixelColor(&colorAntilias,color,t);
			PixelAlphaBlend(colorAntilias, *cappixel2, alpha);
		}
		else
		{
			cappixel2=pixels+(h*yinc+x);
		}		
		for(;cappixel1<cappixel2;cappixel1+=yinc)
		{
			PixelAlphaBlend(color, *cappixel1, alpha);
		}
	}	
}
#ifdef OS_WIN
void _WideLine(PXSurface xsurface,int x1,int y1,int x2,int y2,const Pixel &color,int thick,int thick_extend,void __cdecl a_AAHLineCallBack,void _cdecl a_AAVLineCallBack,const Pixel &colorex)
#else
void _WideLine(PXSurface xsurface,int x1,int y1,int x2,int y2,const Pixel &color,int thick,int thick_extend,void a_AAHLineCallBack,void a_AAVLineCallBack,const Pixel &colorex)
#endif // OS_WIN
{	
	if(x1!=x2||y1!=y2)
	{		
		long w=xsurface->width;
		long h=xsurface->height;
		long subthick = thick + thick_extend;
		long halfth = subthick>>1;
		long iadjust = (subthick% 2 == 0) ? 1 : 0;
		//vertical
		if(x1==x2)
		{
			FillRect(xsurface, x1 - halfth + iadjust, _Min(y1,y2), x1 - halfth + subthick + iadjust, _Max(y1,y2)+1, color, 255);//the best			
			return;
		}
		//horizontal
		else if(y1==y2)
		{
			FillRect(xsurface, _Min(x1,x2), y1 - halfth + iadjust, _Max(x1,x2)+1, y1 - halfth + subthick + iadjust, color, 255);
			return;
		}

		///if(!ClipRectEx(x1,y1,x2,y2,-thick,-thick,xsurface->width+thick-1,xsurface->height+thick-1))
		///{
		///	return;
		///}

		long dx=x2-x1;
		long dy=y2-y1;
		if (dx==0 && dy==0)
			return;

		long yinc=xsurface->pitch>>1;
		Pixel *pixels=(Pixel*)xsurface->pixel;	
		long l;//=(wid<<1)-1;//julery_isqrt((wid*wid)<<1);			
		long D2=dx*dx+dy*dy;
		long capstep=abs(int(double(D2)/ double(dx*dy)* double(1<<16)));
		if(abs(dx)>abs(dy))
		{			
			l=julery_isqrt(thick*thick*D2/(dx*dx));	  
			l+=thick_extend;
			if(l == 0)
				return;
			long wid=(l-1)>>1;			
			long x,y;
			long Dy=RoundDiv(thick*thick*dy,2*l*dx);				
			Dy=abs(Dy);
			if (dx < 0) 
			{			
				x=x2;
				y=y2<<16;
				x2=x1;	
				long tmp=y1;
				y1=y2;
				y2=tmp;
			}		
			else
			{
				x=x1;
				y=y1<<16;
			}					
			long inc = (dy <<16) / dx;			
			if(x<0)
			{
				y-=x*inc;
				x=0;
			}
			else//Modify x for cap
			{	
				y+=Dy*inc;
				x+=Dy;				
			}		
			if(x2<w)
			{
				x2-=Dy;								
			}		
			x2++;
			x2=_Min(w,x2);						
			if(x>=x2)
			{
				return;
			}				
			long w_left=wid;
			long w_right=l-wid;						
			bool bStartCap=x>0;
			bool bEndCap=x2<w;			
			if(bStartCap||bEndCap)
			{	
				long lengthcap=(l<<16)-capstep;					
				long xcap0,ycap0,xcap1,ycap1;
				if(bStartCap)
				{
					xcap0=x-1;
					ycap0=y-inc;												
				}	
				if(bEndCap)
				{
					xcap1=x2;
					ycap1=y+(x2-x)*inc;						
				}					
				for(;(bStartCap||bEndCap)&&lengthcap>=0;lengthcap-=capstep)
				{
					if(bStartCap)
					{
						if(xcap0>=0)
						{	
							if(inc>0)
							{
								AA_Cap_VLine2(pixels,xcap0,((ycap0-lengthcap)>>16)+w_right,(ycap0>>16)+w_right,w,h,yinc,(~ycap0>>8)&0xff,color);
							}
							else
							{
								AA_Cap_VLine1(pixels,xcap0,(ycap0>>16)-wid,((ycap0+lengthcap)>>16)-wid,w,h,yinc,(ycap0>>8)&0xff,color);
							}
							--xcap0;
							ycap0-=inc;
						}
						else
						{
							bStartCap=false;
						}

					}	
					if(bEndCap)
					{
						if(xcap1<w)
						{			
							if(inc>0)
							{
								AA_Cap_VLine1(pixels,xcap1,(ycap1>>16)-wid,((ycap1+lengthcap)>>16)-wid,w,h,yinc,(ycap1>>8)&0xff,color);
							}
							else
							{
								AA_Cap_VLine2(pixels,xcap1,((ycap1-lengthcap)>>16)+w_right,(ycap1>>16)+w_right,w,h,yinc,(~ycap1>>8)&0xff,color);
							}
							++xcap1;
							ycap1+=inc;
						}
						else
						{
							bEndCap=false;
						}
					}
				}						
			}				
			Pixel *pixel=pixels+x;
			for(;x!=x2;++x,y+=inc,++pixel)
			{
				y1=y>>16;
				AAHLine(pixel,color,y,y1-w_left,y1+w_right,h,yinc,colorex);
			}
		}
		else
		{			
			l=julery_isqrt(thick*thick*(D2)/(dy*dy));	  
			l+=thick_extend;
			long Dx=long(thick*abs(dx))/(long)julery_isqrt(D2<<2);				
			long wid=(l-1)>>1;			
			long x,y;
			if (dy < 0) 
			{			
				y=y2;
				x=x2<<16;
				y2=y1;					
			}		
			else
			{
				y=y1;
				x=x1<<16;
			}					
			long inc = (dx <<16) / dy;
			if(y<0)
			{
				x-=y*inc;
				y=0;
			}
			else
			{				
				x+=Dx*inc;
				y+=Dx;
			}			
			if(y2<h)
			{				
				y2-=(Dx);
			}			
			y2++;
			y2=_Min(h,y2);
			if(y>y2)
			{
				return;
			}
			long w_left=wid;
			long w_right=(l-wid);						
			bool bStartCap=y>0;
			bool bEndCap=(y2<=h);
			if((bStartCap||bEndCap))
			{
				long lengthcap=(l<<16)-capstep;					
				long xcap0,ycap0,xcap1,ycap1;					
				if(bStartCap)
				{
					ycap0=y-1;
					xcap0=x-inc;												
				}	
				if(bEndCap)
				{
					ycap1=y2;
					xcap1=x+(y2-y)*inc;						
				}	
				for(;(bStartCap||bEndCap)&&lengthcap>=0;lengthcap-=capstep)
				{
					if(bStartCap)
					{
						if(ycap0>=0)
						{									
							if(inc>0)
							{
								AA_Cap_HLine2(pixels,ycap0,((xcap0-lengthcap)>>16)+w_right,(xcap0>>16)+w_right,w,h,yinc,(~xcap0>>8)&0xff,color);
							}
							else
							{
								AA_Cap_HLine1(pixels,ycap0,(xcap0>>16)-wid,((xcap0+lengthcap)>>16)-wid,w,h,yinc,(xcap0>>8)&0xff,color);
							}
							--ycap0;
							xcap0-=inc;
						}
						else
						{
							bStartCap=false;
						}

					}	
					if(bEndCap)
					{
						if(ycap1<h)
						{	
							if(inc>0)
							{
								AA_Cap_HLine1(pixels,ycap1,(xcap1>>16)-wid,((xcap1+lengthcap)>>16)-wid,w,h,yinc,(xcap1>>8)&0xff,color);
							}
							else
							{
								AA_Cap_HLine2(pixels,ycap1,((xcap1-lengthcap)>>16)+w_right,(xcap1>>16)+w_right,w,h,yinc,(~xcap1>>8)&0xff,color);
							}
							++ycap1;
							xcap1+=inc;
						}
						else
						{
							bEndCap=false;
						}					
					}
				}
			}			
			Pixel *pixel=pixels+(y*yinc);
			for(;y!=y2;++y,x+=inc,pixel+=yinc)
			{			
				x1=x>>16;
				AAVLine(pixel,color,x,x1-w_left,x1+w_right,w,colorex);

			}
		}		
	}
}
#ifdef OS_WIN
void _WideLineEx(PXSurface xsurface,int x1,int y1,int x2,int y2,const Pixel &color,int thick,int thick_extend,void __cdecl a_AAHLineCallBackEx,void _cdecl a_AAVLineCallBackEx,const Pixel &colorex, uint8_t alpha)
#else
void _WideLineEx(PXSurface xsurface,int x1,int y1,int x2,int y2,const Pixel &color,int thick,int thick_extend,void a_AAHLineCallBackEx,void a_AAVLineCallBackEx,const Pixel &colorex, uint8_t alpha)
#endif // OS_WIN
{	
	if(x1!=x2||y1!=y2)
	{		
		long w=xsurface->width;
		long h=xsurface->height;
		long subthick = thick + thick_extend;
		long halfth = subthick>>1;
		long iadjust = (subthick% 2 == 0) ? 1 : 0;
		//vertical
		if(x1==x2)
		{
			FillRect(xsurface, x1 - halfth + iadjust, _Min(y1,y2), x1 - halfth + subthick + iadjust, _Max(y1,y2)+1, color, alpha);//the best			
			return;
		}
		//horizontal
		else if(y1==y2)
		{
			FillRect(xsurface, _Min(x1,x2), y1 - halfth + iadjust, _Max(x1,x2)+1, y1 - halfth + subthick + iadjust, color, alpha);
			return;
		}

		if(!ClipRectEx(x1,y1,x2,y2,-thick,-thick,xsurface->width+thick-1,xsurface->height+thick-1))
		{
			return;
		}
		long dx=x2-x1;
		long dy=y2-y1;
		if (dx==0 && dy==0)
			return;

		long yinc=xsurface->pitch>>1;
		Pixel *pixels=(Pixel*)xsurface->pixel;	
		long l;//=(wid<<1)-1;//julery_isqrt((wid*wid)<<1);			
		long D2=dx*dx+dy*dy;
		long capstep=abs(int(double(D2)*double(1<<16)/double(dx*dy)));
		if(abs(dx)>abs(dy))
		{
			l=julery_isqrt(thick*thick*D2/(dx*dx));	  
			l+=thick_extend;
			if (l==0)
				return;
			long wid=(l-1)>>1;			
			long x,y;
			long Dy=RoundDiv(thick*thick*dy,2*l*dx);				
			Dy=abs(Dy);
			if (dx < 0) 
			{
				x=x2;
				y=y2<<16;
				x2=x1;	
				long tmp=y1;
				y1=y2;
				y2=tmp;
			}		
			else
			{
				x=x1;
				y=y1<<16;
			}
			long inc = (dy <<16) / dx;			
			if(x<0)
			{
				y-=x*inc;
				x=0;
			}
			else//Modify x for cap
			{	
				y+=Dy*inc;
				x+=Dy;				
			}		
			if(x2<w)
			{
				x2-=Dy;								
			}		
			x2++;
			x2=_Min(w,x2);						
			if(x>=x2)
			{
				return;
			}				
			long w_left=wid;
			long w_right=l-wid;						
			bool bStartCap=x>0;
			bool bEndCap=x2<w;			
			if(bStartCap||bEndCap)
			{	
				long lengthcap=(l<<16)-capstep;					
				long xcap0,ycap0,xcap1,ycap1;
				if(bStartCap)
				{
					xcap0=x-1;
					ycap0=y-inc;												
				}	
				if(bEndCap)
				{
					xcap1=x2;
					ycap1=y+(x2-x)*inc;						
				}					
				for(;(bStartCap||bEndCap)&&lengthcap>=0;lengthcap-=capstep)
				{
					if(bStartCap)
					{
						if(xcap0>=0)
						{	
							if(inc>0)
							{
								AA_Cap_VLine2Ex(pixels,xcap0,((ycap0-lengthcap)>>16)+w_right,(ycap0>>16)+w_right,w,h,yinc,(~ycap0>>8)&0xff,color,alpha);
							}
							else
							{
								AA_Cap_VLine1Ex(pixels,xcap0,(ycap0>>16)-wid,((ycap0+lengthcap)>>16)-wid,w,h,yinc,(ycap0>>8)&0xff,color,alpha);
							}
							--xcap0;
							ycap0-=inc;
						}
						else
						{
							bStartCap=false;
						}

					}	
					if(bEndCap)
					{
						if(xcap1<w)
						{			
							if(inc>0)
							{
								AA_Cap_VLine1Ex(pixels,xcap1,(ycap1>>16)-wid,((ycap1+lengthcap)>>16)-wid,w,h,yinc,(ycap1>>8)&0xff,color,alpha);
							}
							else
							{
								AA_Cap_VLine2Ex(pixels,xcap1,((ycap1-lengthcap)>>16)+w_right,(ycap1>>16)+w_right,w,h,yinc,(~ycap1>>8)&0xff,color,alpha);
							}
							++xcap1;
							ycap1+=inc;
						}
						else
						{
							bEndCap=false;
						}
					}
				}						
			}				
			Pixel *pixel=pixels+x;
			for(;x!=x2;++x,y+=inc,++pixel)
			{
				y1=y>>16;
				AAHLineEx(pixel,color,y,y1-w_left,y1+w_right,h,yinc,colorex,alpha);
			}
		}
		else
		{
			l=julery_isqrt(thick*thick*(D2)/(dy*dy));	  
			l+=thick_extend;
			long Dx=long(thick*abs(dx))/(long)julery_isqrt(D2<<2);				
			long wid=(l-1)>>1;			
			long x,y;
			if (dy < 0) 
			{			
				y=y2;
				x=x2<<16;
				y2=y1;					
			}		
			else
			{
				y=y1;
				x=x1<<16;
			}					
			long inc = (dx <<16) / dy;
			if(y<0)
			{
				x-=y*inc;
				y=0;
			}
			else
			{				
				x+=Dx*inc;
				y+=Dx;
			}			
			if(y2<h)
			{				
				y2-=(Dx);
			}			
			y2++;
			y2=_Min(h,y2);
			if(y>y2)
			{
				return;
			}
			long w_left=wid;
			long w_right=(l-wid);						
			bool bStartCap=y>0;
			bool bEndCap=(y2<=h);
			if((bStartCap||bEndCap))
			{
				long lengthcap=(l<<16)-capstep;					
				long xcap0,ycap0,xcap1,ycap1;					
				if(bStartCap)
				{
					ycap0=y-1;
					xcap0=x-inc;												
				}	
				if(bEndCap)
				{
					ycap1=y2;
					xcap1=x+(y2-y)*inc;						
				}	
				for(;(bStartCap||bEndCap)&&lengthcap>=0;lengthcap-=capstep)
				{
					if(bStartCap)
					{
						if(ycap0>=0)
						{									
							if(inc>0)
							{
								AA_Cap_HLine2Ex(pixels,ycap0,((xcap0-lengthcap)>>16)+w_right,(xcap0>>16)+w_right,w,h,yinc,(~xcap0>>8)&0xff,color,alpha);
							}
							else
							{
								AA_Cap_HLine1Ex(pixels,ycap0,(xcap0>>16)-wid,((xcap0+lengthcap)>>16)-wid,w,h,yinc,(xcap0>>8)&0xff,color,alpha);
							}
							--ycap0;
							xcap0-=inc;
						}
						else
						{
							bStartCap=false;
						}

					}	
					if(bEndCap)
					{
						if(ycap1<h)
						{	
							if(inc>0)
							{
								AA_Cap_HLine1Ex(pixels,ycap1,(xcap1>>16)-wid,((xcap1+lengthcap)>>16)-wid,w,h,yinc,(xcap1>>8)&0xff,color,alpha);
							}
							else
							{
								AA_Cap_HLine2Ex(pixels,ycap1,((xcap1-lengthcap)>>16)+w_right,(xcap1>>16)+w_right,w,h,yinc,(~xcap1>>8)&0xff,color,alpha);
							}
							++ycap1;
							xcap1+=inc;
						}
						else
						{
							bEndCap=false;
						}					
					}
				}
			}			
			Pixel *pixel=pixels+(y*yinc);
			for(;y!=y2;++y,x+=inc,pixel+=yinc)
			{			
				x1=x>>16;
				AAVLineEx(pixel,color,x,x1-w_left,x1+w_right,w,colorex,alpha);

			}
		}		
	}
}
void WideLine(PXSurface xsurface,int x0,int y0,int x1,int y1,Pixel color,int wide,bool bAntialias)
{
	if(wide == 0)	return;
	if(wide<2)
	{
		Line(xsurface,x0,y0,x1,y1,color,bAntialias);
	}
	else
	{
		_WideLine(xsurface,x0,y0,x1,y1,color,wide,0,AAWide_HLine,AAWide_VLine,0);			
	}
}
void WideLineEx(PXSurface xsurface,int x0,int y0,int x1,int y1,Pixel color,int wide,uint8_t alpha, bool bAntialias)
{
	if(wide == 0)	return;
	if(wide<2)
	{
		LineEx(xsurface,x0,y0,x1,y1,color,alpha, bAntialias);
	}
	else
	{
		_WideLineEx(xsurface,x0,y0,x1,y1,color,wide,0,AAWide_HLineEx,AAWide_VLineEx,0, alpha);
	}
}
void WideOutline(PXSurface xsurface,int x0,int y0,int x1,int y1,Pixel color,int wide)
{
	if(xsurface)
	{
		_WideLine(xsurface,x0,y0,x1,y1,color,wide,2,AAWide_HOutLine,AAWide_VOutLine,0);
	}
}
void WideOutlineEx(PXSurface xsurface,int x0,int y0,int x1,int y1,Pixel color,int wide,uint8_t alpha)
{
	if(xsurface)
	{
		_WideLineEx(xsurface,x0,y0,x1,y1,color,wide,2,AAWide_HOutLineEx,AAWide_VOutLineEx,0, alpha);
	}
}
void WideInline(PXSurface xsurface,int x0,int y0,int x1,int y1,Pixel color,int wide,Pixel outlinecolor)
{
	if(xsurface)
		_WideLine(xsurface,x0,y0,x1,y1,color,wide,0,AAWide_HInLine,AAWide_VInLine,outlinecolor);	
}
void WideInlineEx(PXSurface xsurface,int x0,int y0,int x1,int y1,Pixel color,int wide,Pixel outlinecolor,uint8_t alpha)
{
	if(xsurface)
		_WideLineEx(xsurface,x0,y0,x1,y1,color,wide,0,AAWide_HInLineEx,AAWide_VInLineEx,outlinecolor,alpha);
}

static void  FillEdges( POINT p[], int vertex_cnt)
{
    POINT *p1, *p2, *p3,*pend;		
	struct edge *e;
	for (p1=pend=p+(vertex_cnt-1),p2=p; p2<=pend;p1=p2++)
    {        
		if (p1->y == p2->y)			
            continue;   /* Skip horiz. edges */
		for(p3=p2+1;;p3++)
		{			
			if(p3>pend)
				p3=p;
			if(p3->y!=p2->y)
			{
				break;
			}
		}				
        e = new edge;     		
		if (p2->x > p1->x)
		{
			e->xNowDir = -1;
			e->xNowNumStep=p2->x-p1->x;
		}
		else if (p2->x < p1->x)
		{
			e->xNowDir = 1;
			e->xNowNumStep=p1->x-p2->x;
		}
		else
		{
			e->xNowDir = 0;
			e->xNowNumStep=0;
		}
        if (p2->y > p1->y)
        {
			e->xNowDir=-e->xNowDir;			
            e->yTop = p1->y;
            e->yBot = p2->y;
            e->xNowWhole = p1->x;            
            e->xNowDen = e->yBot - e->yTop;
            e->xNowNum = (e->xNowDen >> 1);
            if (p3->y > p2->y)
                --e->yBot;
        }
        else
        {
            e->yTop = p2->y;
            e->yBot = p1->y;
            e->xNowWhole = p2->x;			
            e->xNowDen = e->yBot - e->yTop;
            e->xNowNum = (e->xNowDen >> 1);
            if (p3->y < p2->y)
            {
                ++e->yTop;
                e->xNowNum += e->xNowNumStep;
                while (e->xNowNum >= e->xNowDen)
                {
                    e->xNowWhole += e->xNowDir;
                    e->xNowNum -= e->xNowDen;
                }
            }
        }
        e->next = edgeTable[e->yTop];
        edgeTable[e->yTop] = e;
    }
}

static void FillEdgesEx( POINT p[], int vertex_cnt)
{
    POINT *p1, *p2, *p3,*pend;		
	struct edge *e;
	for (p1=pend=p+(vertex_cnt-1),p2=p; p2<=pend;p1=p2++)
    {        
		if (p1->y == p2->y)	{
			HLINE* hl = new (struct HLINE);
			if (p1->x<p2->x)
			{
				hl->nLeft = p1->x;
				hl->nRight = p2->x;
			}
			else
			{
				hl->nLeft = p2->x;
				hl->nRight = p1->x;
			}
			hl->next = pHLines[p1->y];
			pHLines[p1->y] = hl;
            continue;   /* Skip horiz. edges */
		}
		for(p3=p2+1;;p3++)
		{			
			if(p3>pend)
				p3=p;
			if(p3->y!=p2->y)
			{
				break;
			}
		}
        e = new edge;
		if (p2->x > p1->x)
		{
			e->xNowDir = -1;
			e->xNowNumStep=p2->x-p1->x;
		}
		else if (p2->x < p1->x)
		{
			e->xNowDir = 1;
			e->xNowNumStep=p1->x-p2->x;
		}
		else
		{
			e->xNowDir = 0;
			e->xNowNumStep=0;
		}
        if (p2->y > p1->y)
        {
			e->xNowDir=-e->xNowDir;			
            e->yTop = p1->y;
            e->yBot = p2->y;
            e->xNowWhole = p1->x;            
            e->xNowDen = e->yBot - e->yTop;
            e->xNowNum = (e->xNowDen >> 1);
            if (p3->y > p2->y)
                --e->yBot;
        }
        else
        {
            e->yTop = p2->y;
            e->yBot = p1->y;
            e->xNowWhole = p2->x;			
            e->xNowDen = e->yBot - e->yTop;
            e->xNowNum = (e->xNowDen >> 1);
            if (p3->y < p2->y)
            {
                ++e->yTop;
                e->xNowNum += e->xNowNumStep;
                while (e->xNowNum >= e->xNowDen)
                {
                    e->xNowWhole += e->xNowDir;
                    e->xNowNum -= e->xNowDen;
                }
            }
        }
        e->next = edgeTable[e->yTop];
        edgeTable[e->yTop] = e;
    }
}

static vector<DWORD> vt_New;
struct edge *UpdateActive(struct edge *active, long curY)
{
    struct edge *e, **ep;
    for (ep = &active, e = *ep; e != NULL; e = *ep)
	{
        if (e->yBot < curY)
        {
            *ep = e->next;
            delete(e);
			//////////////////////////////////////////////////////////////////////////
			vector<DWORD>::iterator it = find(vt_New.begin(), vt_New.end(), (DWORD)e);
			if(it != vt_New.end())
				vt_New.erase(it);
			//////////////////////////////////////////////////////////////////////////
        }
        else
			ep = &e->next;
	}
    *ep = edgeTable[curY];

    return active;
}

void FreeHLine(HLINE*& pHLine)
{
	HLINE* pTemp = pHLine;
	while (pTemp)
	{
		pHLine = pTemp->next;
		delete pTemp;
		pTemp = pHLine;
	}
	pHLine = NULL;
}

void FreeEdges()
{
	vector<DWORD>::iterator it = vt_New.begin();
	for(;it!=vt_New.end();it++)	{
		edge* pEdge = (edge*)(*it);
		delete pEdge;
	}
	vt_New.clear();
}

//b/#ifdef UNDER_CE
//b/static long xCoords[150];
//b/static long kFactors[150];
//b/#else
static long xCoords[10000];
static long kFactors[10000];
//b/#endif
static void HFillLineEx(PXSurface surface,int x0,int y0,int x1,Pixel color)
{
}

static void DrawRuns(PXSurface xsurface, edge *active, long curY, Pixel pat)
{
    struct edge *e;

    long numCoords = 0;
    long i;	
	long *tmp;
    for (e = active; e != NULL; e = e->next)
    {
		for(i=numCoords,tmp=xCoords+numCoords;i&&*(tmp-1)>e->xNowWhole;i--,tmp--)
			*tmp=*(tmp-1);        
		*(tmp)=e->xNowWhole;
        numCoords++;
        e->xNowNum += e->xNowNumStep;
        while (e->xNowNum >= e->xNowDen)
        {
            e->xNowWhole += e->xNowDir;
            e->xNowNum -= e->xNowDen;
        }
    }
	if(curY>=xsurface->height)
		return;
 
	if (numCoords &1)  /* Protect from degenerate polygons */
        xCoords[numCoords] = xCoords[numCoords - 1],
        numCoords++;	

	if(curY<xsurface->height)
	{
		for (i = 0; i < numCoords; i += 2)
		{   			
			//code added to avoid top and bottom horizontal lines
			//end
			//+1 to allow fill polygon before draw line
			//_HLineEx(xsurface, xCoords[i] + 1,curY,xCoords[i+1],pat);			
			if (xCoords[i + 1])
				_HLineEx(xsurface, xCoords[i], curY, xCoords[i + 1], pat);
		}
	}
}

static void DrawRunsEx(PXSurface xsurface, edge *active, long curY, Pixel pat)
{
    struct edge *e;
    
    long numCoords = 0;
    long i, dx, dy;	
	long *tmp;
    for (e = active; e != NULL; e = e->next)
    {		
		for(i=numCoords,tmp=xCoords+numCoords; i&&*(tmp-1)>e->xNowWhole; i--,tmp--)
		{
			*tmp = *(tmp-1);
			kFactors[i] = kFactors[i-1];
		}
		*(tmp)=e->xNowWhole;
		kFactors[i] = 0;
		dx = e->xNowNumStep;
		dy = e->yBot-e->yTop;
		if (dy)
		{
			if (dx>dy)
			{
				kFactors[i] = dx/dy;
				if (kFactors[i]>dx)
					kFactors[i] = dx;
			}
			else
				kFactors[i] = 1;
		}
        numCoords++;
        e->xNowNum += e->xNowNumStep;
        while (e->xNowNum >= e->xNowDen)
        {
            e->xNowWhole += e->xNowDir;
            e->xNowNum -= e->xNowDen;
        }
    }

    if (numCoords &1)  /* Protect from degenerate polygons */
        xCoords[numCoords] = xCoords[numCoords - 1],
        numCoords++;
	int nLeft = 0;
	int nRight = 0;
	bool bNotDraw;
	if(curY<xsurface->height)
	{
		for (i = 0; i < numCoords; i += 2)
		{   			
			//code added to avoid top and bottom horizontal lines
			//end
			//+1 to allow fill polygon before draw line
			//_HLineEx(xsurface, xCoords[i] + 1,curY,xCoords[i+1],pat);	
			bNotDraw = false;
			if (pHLines[curY])
			{
				HLINE* pTemp = pHLines[curY];
				while (pTemp)
				{
					if (pTemp->nLeft==xCoords[i] && pTemp->nRight==xCoords[i+1])
					{
						bNotDraw = true;
						break;
					}
					pTemp = pTemp->next;
				}
			}
			if (bNotDraw)
				continue;

			nLeft = xCoords[i] + kFactors[i];
			nRight = xCoords[i + 1] - kFactors[i+1];
			if(nRight>nLeft)
				_HLineEx(xsurface, nLeft, curY, nRight, pat);
		}
	}
	if (pHLines[curY])
		FreeHLine(pHLines[curY]);
}

static void _FillPoly(PXSurface xsurface, POINT *vertex_vector, int vertex_cnt, Pixel color )
{   
    struct edge *active;
    long curY;
	const long _Maxy=xsurface->height;	
	memset(edgeTable, 0, sizeof(edgeTable));
    FillEdges(vertex_vector, vertex_cnt);

    for (curY = 0; edgeTable[curY] == NULL; ++curY)
        if (curY == _Maxy)
            return;     // No edges in polygon 

    for (active = NULL; (active = UpdateActive(active, curY)) != NULL; curY++)
	{
		DrawRuns(xsurface, active, curY, color);
	}
}

static void _FillPolyEx(PXSurface xsurface, POINT *vertex_vector, int vertex_cnt, Pixel color )
{    
    struct edge *active;
    long curY;
	const long _Maxy = xsurface->height;	
	memset(edgeTable, 0, sizeof(edgeTable));
	memset(pHLines, 0, sizeof(pHLines));
    FillEdgesEx(vertex_vector, vertex_cnt);

    for (curY = 0; edgeTable[curY] == NULL; ++curY)
        if (curY == _Maxy)
		{
			// Giai phong bo nho cap phat cho HLINE neu co
			for (int i=0; i<MAXVERTICAL; i++)
				if (pHLines[i])
					FreeHLine(pHLines[i]);
			
            return;		/* No edges in polygon */
		}

    for (active = NULL; (active = UpdateActive(active, curY)) != NULL; curY++)
	{		
			DrawRunsEx(xsurface,active, curY, color);	
	}
}

#ifdef OS_WIN
#ifndef INFINITY
const float INFINITY = (float)(1.0e+30);
#endif//#ifndef INFINITY
#endif // OS_WIN
const float INV_INFINITY = -INFINITY;
static vector<int> gPoint;
#define add(a,b) {vpt.push_back(int(a));vpt.push_back(int(b));}

size_t
fclip(POINT *pts, int npoly, float wx1, float wy1, float  wx2, float wy2, vector<int>& vpt, bool bPolygon)
{	//incorrect if bPolygon = false; 
	vpt.clear();
	if(npoly==2)
	{
		if(!bPolygon)
		{
			vpt.push_back(pts[0].x);
			vpt.push_back(pts[0].y);
			vpt.push_back(pts[1].x);
			vpt.push_back(pts[1].y);
			if(ClipRectEx(vpt[0],vpt[1],vpt[2],vpt[3],(int)wx1,(int) wy1,(int)wx2,(int) wy2))
			{
				return vpt.size()>>1;
			}
		}
		return 0;
	}
   float deltax, deltay, xin,xout,  yin,yout;
   float tinx,tiny,  toutx,touty,  tin1, tin2,  tout1;      
   
   const POINT *nextpts=pts;
   const POINT *lastpts=pts+(npoly-1);         
   const POINT *tmppts=lastpts;
   if(!bPolygon)
   {
	   ++nextpts;
	   tmppts=pts;
	   if (tmppts->x>=wx1 && tmppts->x<=wx2 && tmppts->y>=wy1 && tmppts->y<=wy2)
		   add (tmppts->x, tmppts->y);
   }

   for (; nextpts<=lastpts;++nextpts)
   {	   
      deltax = float(nextpts->x-tmppts->x);
      deltay = float(nextpts->y-tmppts->y);

      if (deltax > 0 || (deltax == 0 && (float)tmppts->x>wx2)) 
	  { /*  points to right */
         xin = wx1;
         xout = wx2;
	  }
      else 
	  {
         xin = wx2;
         xout = wx1;
	  }
      if (deltay > 0 || (deltay == 0 && (float)tmppts->y>wy2)) 
	  { /*  points up */
         yin = wy1;
         yout = wy2;
	  }
      else 
	  {
         yin = wy2;
         yout = wy1;
	  }
      tinx = (deltax != 0) ? ((xin - (float)tmppts->x)/deltax) : INV_INFINITY ;
      tiny = (deltay != 0) ? ((yin - (float)tmppts->y)/deltay) : INV_INFINITY ;
   
      if (tinx < tiny) 
	  {	/* hits x first */
         tin1 = tinx;
         tin2 = tiny;
	  }
      else			/* hits y first */
	  {
         tin1 = tiny;
         tin2 = tinx;
	  }
      if (1 >= tin1) {
         if (0 < tin1) {
            add(xin,yin);
            }
         if (1 >= tin2) {
	    if (deltax != 0) toutx = (xout-(float)tmppts->x)/deltax;
	    else {
               if (wx1 <= (float)tmppts->x && (float)tmppts->x <= wx2) toutx = INFINITY;
               else                        toutx = INV_INFINITY;
	       }
	    if (deltay != 0) touty = (yout-(float)tmppts->y)/deltay;
	    else {
               if (wy1 <= (float)tmppts->y && (float)tmppts->y <= wy2) touty = INFINITY;
               else                        touty = INV_INFINITY;
	       }

	    tout1 = (toutx < touty) ? toutx : touty ;
   
            if (0 < tin2 || 0 < tout1) {
               if (tin2 <= tout1) {
                  if (0 < tin2) {
                     if (tinx > tiny) {
                        add (xin, (float)tmppts->y+tinx*deltay);
                        }
                     else {
                        add ((float)tmppts->x + tiny*deltax, yin);
                        }
                     }
                  if (1 > tout1) {
                     if (toutx < touty) {
                        add (xout, (float)tmppts->y+toutx*deltay);
                        }
                     else {
                        add ((float)tmppts->x + touty*deltax, yout);
                        }
                     }
                  else {
                     add ((float)nextpts->x,(float)nextpts->y);
                     }
                  }
               else {
                  if (tinx > tiny) {
                     add (xin, yout);
                     }
                  else {
                     add (xout, yin);
                     }
                  }
               }
            }
         }
	  tmppts=nextpts;
   }
   return vpt.size()>>1;   
}
void FillPoly(PXSurface xsurface, POINT *vertex_vector, int vertex_cnt, Pixel color,int fp,bool bclip,uint8_t alpha )
{
	if(bclip)//Need Clip
	{
		vertex_cnt=(int)fclip(vertex_vector,vertex_cnt,0,0,xsurface->width,xsurface->height,gPoint, true);
		if(vertex_cnt>2)
		{
			vertex_vector=(POINT*)&gPoint[0];
		}
		else
		{
			return;
		}
	}
	SetPatternFunc(xsurface,fp);
	memset(edgeTable, 0, sizeof(edgeTable));
	if ( (alpha | 0x03) != 0xFF )
	{
		gAlPha=alpha;
		xsurface->PixelFunc=AlphaPixelFunc;
		_FillPoly(xsurface,vertex_vector,vertex_cnt,color);
		xsurface->PixelFunc=DefaultPixelFunc;		
	}
	else
	{
		_FillPoly(xsurface,vertex_vector,vertex_cnt,color);
	}
	xsurface->Pattern=FT_SOLID_CHECK;
}

void FillPolyEx(PXSurface xsurface, POINT *vertex_vector, int vertex_cnt, Pixel color)
{
	vertex_cnt=(int)fclip(vertex_vector,vertex_cnt,0,0,xsurface->width,xsurface->height,gPoint, true);
	if(vertex_cnt>2)
	{
		vertex_vector=(POINT*)&gPoint[0];
	}
	else
	{
		return;
	}
	_FillPolyEx(xsurface, vertex_vector, vertex_cnt, color);
}

void DrawRect(PXSurface xsurface,const RECT &rect,Pixel color, int width, uint8_t alpha)
{
	if (width==0)
		return;

	if (width<=1)
	{
		::HLine(xsurface,rect.left,rect.top,rect.right,color, alpha);
		::HLine(xsurface,rect.left,rect.bottom,rect.right,color,alpha);
		::VLine(xsurface,rect.left,rect.top,rect.bottom,color,alpha);
		::VLine(xsurface,rect.right,rect.top,rect.bottom+1,color,alpha);
	}
	else
	{
		if (alpha==255)
		{
			WideLine(xsurface, rect.left, rect.top, rect.right, rect.top, color, width, true);
			WideLine(xsurface, rect.right, rect.top, rect.right, rect.bottom, color, width, true);
			WideLine(xsurface, rect.right, rect.bottom, rect.left, rect.bottom, color, width, true);
			WideLine(xsurface, rect.left, rect.bottom, rect.left, rect.top, color, width, true);
		}
		else
		{
			WideLineEx(xsurface, rect.left, rect.top, rect.right, rect.top, color, width, alpha, true);
			WideLineEx(xsurface, rect.right, rect.top, rect.right, rect.bottom, color, width, alpha, true);
			WideLineEx(xsurface, rect.right, rect.bottom, rect.left, rect.bottom, color, width, alpha, true);
			WideLineEx(xsurface, rect.left, rect.bottom, rect.left, rect.top, color, width, alpha, true);
		}
	}
}

void SetPixel(PXSurface xsurface,int x,int y,Pixel color)
{
	if(xsurface&&x>=0&&x<xsurface->width&&y>=0&&y<xsurface->height)
	{
		*GetPixels(xsurface,x,y)=color;		
	}
}
void SetPixel(PXSurface xsurface,int x,int y,Pixel color,uint8_t alpha)
{
	if(xsurface&&x>=0&&x<xsurface->width&&y>=0&&y<xsurface->height)
	{		
		if ( (alpha | 0x03) == 0xFF )
			*GetPixels(xsurface,x,y)=color;
		else
			PixelAlphaBlend(color,*GetPixels(xsurface,x,y),alpha);
	}
}
inline int GetRectWidth(const RECT &rect)
{
	return rect.right-rect.left;
}
inline int GetRectHeight(const RECT &rect)
{
	return rect.bottom-rect.top;
}
static void PixelAlphaBlendMark0(Pixel src, Pixel &dest,int alpha,Pixel markcolor)
{
	dest=src;
}
static void PixelAlphaBlendMark1(Pixel src, Pixel &dest,int alpha,Pixel markcolor)
{
	PixelAlphaBlend(src,dest,alpha);
}
static void PixelAlphaBlendMark2(Pixel src, Pixel &dest,int alpha,Pixel markcolor)
{
	if(src!=markcolor)
		PixelAlphaBlend(src,dest,alpha);
}
static void PixelAlphaBlendMark3(Pixel src, Pixel &dest,int alpha,Pixel markcolor)
{	
	if(src!=markcolor)
	{
		dest=src;
	}	
	
}
typedef void (*PIXEL_ALPHA_BLEND_MARK)(Pixel , Pixel &,int ,Pixel);
void TransformEx(uint8_t *pixel_src,uint8_t *lastpixel_src,uint8_t *pixel_dest,int w,int src_pitch,int dest_pitch,int alpha,Pixel colormark,PIXEL_ALPHA_BLEND_MARK PixelFunc)
{
	for(;pixel_src<lastpixel_src;)
	{
		Pixel *pixelsrc=(Pixel *)pixel_src;
		Pixel *pixeldst=(Pixel *)pixel_dest;
		Pixel *lastpixel=pixelsrc+w;
		for(;pixelsrc<lastpixel;)
		{
			PixelFunc(*pixelsrc,*pixeldst,alpha,colormark);
			++pixelsrc;
			++pixeldst;
		}											
		pixel_src+=src_pitch;
		pixel_dest+=dest_pitch;
	}	
}
void BlitFast(PXSurface dest,const PXSurface source)
{
	memcpy(dest->pixel,source->pixel,source->lastpixel-source->pixel);
}
void BlitSurface(PXSurface dest,int destX, int destY, const PXSurface source, const RECT* srcRect,uint8_t alpha, unsigned long colormark)
{
	if(dest&&source)
	{
		RECT src_rect;
		if (srcRect)
		{
			src_rect = *srcRect;
			if (src_rect.left<0) src_rect.left = 0;
			if (src_rect.top<0) src_rect.top= 0;
			if (src_rect.left>=source->GetWidth() || src_rect.right<=0 || src_rect.top>=source->GetHeight() || src_rect.bottom<=0)
				return;
			if (src_rect.bottom>source->GetHeight())
				src_rect.bottom = source->GetHeight();
			if (src_rect.right>source->GetWidth())
				src_rect.right = source->GetWidth();
		}
		else
		{
			src_rect.left=src_rect.top=0;
			src_rect.right=source->width;
			src_rect.bottom=source->height;
		}
		if(src_rect.left<src_rect.right&&src_rect.top<src_rect.bottom)
		{
			RECT dst_rect;
			dst_rect.right=_Min(dest->width,destX+GetRectWidth(src_rect));
			dst_rect.bottom=_Min(dest->height,destY+GetRectHeight(src_rect));
			dst_rect.left=_Max(0,destX);
			dst_rect.top=_Max(0,destY);
			if(dst_rect.left<dst_rect.right&&dst_rect.top<dst_rect.bottom)
			{
				int xoffset = dst_rect.left - destX;
				int yoffset = dst_rect.top - destY;
				src_rect.left += xoffset;
				src_rect.top += yoffset;
				// Adjust width and height of source rectangle
				src_rect.right = src_rect.left + GetRectWidth(dst_rect);
				src_rect.bottom = src_rect.top + GetRectHeight(dst_rect);

				int w=_Min(GetRectWidth(dst_rect),GetRectWidth(src_rect));
				int h=_Min(GetRectHeight(dst_rect),GetRectHeight(src_rect));				
				uint8_t *pixel_src=(uint8_t*)GetPixels(source,src_rect.left,src_rect.top);
				uint8_t *pixel_dest=(uint8_t*)GetPixels(dest,dst_rect.left,dst_rect.top);
				if(!pixel_src || !pixel_dest)	return;
				uint8_t *lastpixel_src=pixel_src+h*source->pitch;
				if(colormark == ULONG_MAX)
				{
					if ( (alpha | 0x03) == 0xFF)
					{					
						w<<=1;
						for(;pixel_src<lastpixel_src;)
						{
							memcpy(pixel_dest,pixel_src,w);
							pixel_src+=source->pitch;
							pixel_dest+=dest->pitch;
						}					
					}
					else
					{						
						TransformEx(pixel_src,lastpixel_src,pixel_dest,w,source->pitch,dest->pitch,alpha,Color(0,0,0),PixelAlphaBlendMark1);
					}
				}
				else//mark
				{
					Pixel _colormark=Color((COLORREF)colormark);
					if ( (alpha | 0x03) == 0xFF)
					{
						TransformEx(pixel_src,lastpixel_src,pixel_dest,w,source->pitch,dest->pitch,alpha,_colormark,PixelAlphaBlendMark3);
					}
					else
					{
						TransformEx(pixel_src,lastpixel_src,pixel_dest,w,source->pitch,dest->pitch,alpha,_colormark,PixelAlphaBlendMark2);
					}
				}
			}
		}
	}
}

void BlitSurfaceMask(PXSurface pDstSurf, int nDestX, int nDestY, const PXSurface pSrcSurf, uint8_t* pAlpha, const RECT* pSrcRect)
{
	// Check
	if (!pDstSurf || !pSrcSurf)
		return;
	if (pDstSurf->width<=0 || pDstSurf->height<=0 || pSrcSurf->width<=0 || pSrcSurf->height<=0)
		return;

	// Check alpha
	if (!pAlpha)
	{
		::BlitSurface(pDstSurf, nDestX, nDestY, pSrcSurf, pSrcRect, (uint8_t)255, ULONG_MAX);
		return;
	}

	// Calculate and modify source rect
	RECT srcRect = {0, 0, pSrcSurf->width, pSrcSurf->height};
	if (pSrcRect)
	{
		srcRect = *pSrcRect;
		if (srcRect.left>=pSrcSurf->width || srcRect.right<=0 || srcRect.top>=pSrcSurf->height || srcRect.bottom<=0)
			return;
		if (srcRect.bottom>pSrcSurf->height)
			srcRect.bottom = pSrcSurf->height;
		if (srcRect.right>pSrcSurf->width)
			srcRect.right = pSrcSurf->width;
	}
	if (srcRect.left>=srcRect.right || srcRect.top>=srcRect.bottom)
		return;

	// Calculate and modify destination rect
	RECT dstRect;
	dstRect.left = _Max(0, nDestX);
	if (nDestX<0)
		srcRect.left += -nDestX;
	dstRect.top = _Max(0, nDestY);
	if (nDestY<0)
		srcRect.top += -nDestY;
	dstRect.right = _Min(pDstSurf->width, dstRect.left + GetRectWidth(srcRect));
	dstRect.bottom = _Min(pDstSurf->height, dstRect.top + GetRectHeight(srcRect));
	if (dstRect.left>=dstRect.right || dstRect.top>=dstRect.bottom)
		return;

	// Blit source to destination with alpha
	int nWid = _Min(GetRectWidth(dstRect), GetRectWidth(srcRect));
	int nHei = _Min(GetRectHeight(dstRect), GetRectHeight(srcRect));
	uint8_t *pSrcData = (uint8_t*)GetPixels(pSrcSurf, srcRect.left, srcRect.top);
	uint8_t *pDesData = (uint8_t*)GetPixels(pDstSurf, dstRect.left, dstRect.top);
	uint8_t *pSrcAlpha = pAlpha + srcRect.top*pSrcSurf->width + srcRect.left;
	Pixel *pSrcTmp, *pDesTmp;
	uint8_t* pAlphaTmp;
	for (int nRow=0; nRow<nHei; nRow++)
	{
		pSrcTmp = (Pixel*)pSrcData;
		pSrcData += pSrcSurf->pitch;
		pAlphaTmp = pSrcAlpha;
		pSrcAlpha += pSrcSurf->width;
		pDesTmp = (Pixel*)pDesData;
		pDesData += pDstSurf->pitch;

		for (int nCol=0; nCol<nWid; nCol++)
		{
			PixelAlphaBlend(*pSrcTmp, *pDesTmp, *pAlphaTmp);
			pSrcTmp++;
			pDesTmp++;
			pAlphaTmp++;
		}
	}
}

void GradientSurfaceH(PXSurface dest,int destX, int destY, const PXSurface source, uint8_t alpha1, uint8_t alpha2, const RECT* srcRect)
{
	if(dest&&source)
	{
		RECT src_rect;
		if (srcRect)
		{
			src_rect = *srcRect;
			if (src_rect.left>=source->GetWidth() || src_rect.right<=0 || src_rect.top>=source->GetWidth() || src_rect.bottom<=0)
				return;
			if (src_rect.bottom>source->GetHeight())
				src_rect.bottom = source->GetHeight();
			if (src_rect.right>source->GetWidth())
				src_rect.right = source->GetWidth();
		}
		else
		{
			src_rect.left=src_rect.top=0;
			src_rect.right=source->width;
			src_rect.bottom=source->height;
		}		
		if(src_rect.left<src_rect.right&&src_rect.top<src_rect.bottom)
		{
			RECT dst_rect;
			dst_rect.right=_Min(dest->width,destX+GetRectWidth(src_rect));
			dst_rect.bottom=_Min(dest->height,destY+GetRectHeight(src_rect));
			dst_rect.left=_Max(0,destX);
			dst_rect.top=_Max(0,destY);
			if(dst_rect.left<dst_rect.right&&dst_rect.top<dst_rect.bottom)
			{
				int xoffset = dst_rect.left - destX;
				int yoffset = dst_rect.top - destY;
				src_rect.left += xoffset;
				src_rect.top += yoffset;
				// Adjust width and height of source rectangle
				src_rect.right = src_rect.left + GetRectWidth(dst_rect);
				src_rect.bottom = src_rect.top + GetRectHeight(dst_rect);

				int w=_Min(GetRectWidth(dst_rect),GetRectWidth(src_rect));
				int h=_Min(GetRectHeight(dst_rect),GetRectHeight(src_rect));				
				uint8_t *pixel_src=(uint8_t*)GetPixels(source,src_rect.left,src_rect.top);
				uint8_t *pixel_dest=(uint8_t*)GetPixels(dest,dst_rect.left,dst_rect.top);
				if(!pixel_src || !pixel_dest)	return;
				uint8_t *lastpixel_src=pixel_src+h*source->pitch;
				uint8_t alpha = alpha1;
				double dalpha = (((double)alpha2-(double)alpha1))/(src_rect.bottom - src_rect.top);
				int nCount = 0;
				for(;pixel_src<lastpixel_src;)
				{
					Pixel *pixelsrc=(Pixel *)pixel_src;
					Pixel *pixeldst=(Pixel *)pixel_dest;
					Pixel *lastpixel=pixelsrc+w;
					alpha = (uint8_t)(alpha1 + (nCount*dalpha));
					for(;pixelsrc<lastpixel;)
					{
						PixelAlphaBlendMark1(*pixelsrc,*pixeldst,alpha,ULONG_MAX);
						++pixelsrc;
						++pixeldst;
					}
					nCount++;
					pixel_src += source->pitch;
					pixel_dest+= dest->pitch;
				}
			}
		}
	}
}

inline uint8_t GetCode(long &x1,long &x2,const long &v1,const long &v2)
{
	uint8_t code=0;
	if(x1<v1)
	{
		x1=v1;
	}
	else if(x1<v2)
	{
		code|=0x01;//10
	}
	if(x2>v2)
	{
		x2=v2;
	}
	else if(x2>v1)
	{
		code|=0x02;
	}
	return code; 
}
static void Pixel4Sym(Pixel *pixel,const long &cx,const long &cy,const long &x,const long &y,const long &t,const Pixel &color,const long &yinc,const long &w,const long &h)
{	
	long x1=cx-x;
	long x2=cx+x;
	long y1=cy-y;
	long y2=cy+y;
	uint8_t code1=GetCode(x1,x2,0,w);
	if(x1<=x2)
	{
		uint8_t code2=GetCode(y1,y2,0,h);
		if(y1<=y2)
		{
			long y1_inc=y1*yinc;
			long y2_inc=y2*yinc;			
			if(code1&0x01)
			{
				if(code2&0x01)
				{
					AAPixelColor(pixel+(x1+y1_inc),color,t);
				}
				if(code2&0x02)
				{
					AAPixelColor(pixel+(x1+y2_inc),color,t);
				}
			}
			if(code1&0x02)
			{
				if(code2&0x01)
				{
					AAPixelColor(pixel+(x2+y1_inc),color,t);
				}
				if(code2&0x02)
				{
					AAPixelColor(pixel+(x2+y2_inc),color,t);
				}
			}			
		}
	}		
}
static void Pixel4SymEx(Pixel *pixel,const long &cx,const long &cy,const long &x,const long &y,const long &t,const Pixel &color,const long &yinc,const long &w,const long &h, uint8_t alpha)
{	
	long x1=cx-x;
	long x2=cx+x;
	long y1=cy-y;
	long y2=cy+y;
	uint8_t code1=GetCode(x1,x2,0,w);
	if(x1<=x2)
	{
		uint8_t code2=GetCode(y1,y2,0,h);
		Pixel colorAntilias;
		Pixel *colorPixel;
		if(y1<=y2)
		{
			long y1_inc=y1*yinc;
			long y2_inc=y2*yinc;			
			if(code1&0x01)
			{
				if(code2&0x01)
				{
					colorPixel = pixel+(x1+y1_inc);
					colorAntilias = *colorPixel;
					AAPixelColor(&colorAntilias,color,t);
					PixelAlphaBlend(colorAntilias, *colorPixel, alpha);
				}
				if(code2&0x02)
				{
					colorPixel = pixel+(x1+y2_inc);
					colorAntilias = *colorPixel;
					AAPixelColor(&colorAntilias,color,t);
					PixelAlphaBlend(colorAntilias, *colorPixel, alpha);
				}
			}
			if(code1&0x02)
			{
				if(code2&0x01)
				{
					colorPixel = pixel+(x2+y1_inc);
					colorAntilias = *colorPixel;
					AAPixelColor(&colorAntilias,color,t);
					PixelAlphaBlend(colorAntilias, *colorPixel, alpha);
				}
				if(code2&0x02)
				{
					colorPixel = pixel+(x2+y2_inc);
					colorAntilias = *colorPixel;
					AAPixelColor(&colorAntilias,color,t);
					PixelAlphaBlend(colorAntilias, *colorPixel, alpha);
				}
			}			
		}
	}		
}
static void Pixel8Sym(Pixel *pixels,const long &cx,const long &cy,const long &x,const long &y,const long &t,const Pixel &color,const long &yinc,const long &w,const long &h)
{	
	Pixel4Sym(pixels,cx,cy,x,y,t,color,yinc,w,h);
	if(x!=y)
		Pixel4Sym(pixels,cx,cy,y,x,t,color,yinc,w,h);					
	
}
static void Pixel8SymEx(Pixel *pixels,const long &cx,const long &cy,const long &x,const long &y,const long &t,const Pixel &color,const long &yinc,const long &w,const long &h, uint8_t alpha)
{	
	Pixel4SymEx(pixels,cx,cy,x,y,t,color,yinc,w,h,alpha);
	if(x!=y)
		Pixel4SymEx(pixels,cx,cy,y,x,t,color,yinc,w,h,alpha);
	
}
void AACircle(PXSurface xsurface,int x0,int y0,int radius,Pixel color)
{
	if(!xsurface)
		return;
	long w=xsurface->width-1;
	long h=xsurface->height-1;		
	long r=radius>>1;
	if(r==0)
	{		
		if(radius)
		{
			SetPixel(xsurface,x0,y0,color);
			SetPixel(xsurface,x0+1,y0,color);
			SetPixel(xsurface,x0,y0+1,color);
			SetPixel(xsurface,x0+1,y0+1,color);
		}
		else
		{
			SetPixel(xsurface,x0,y0,color);
		}
		return;
	}
	if((x0+r)>=0&&(x0-r)<=w&&(y0+r)>=0&&(y0-r)<=h)
	{
		long yinc=xsurface->pitch>>1;
		Pixel *pixels=(Pixel *)xsurface->pixel;
		int x = r;
		int y = 0;	
		long T = 0;
		const long I = long(0.93*double(1<<8)); 		
		SetPixel(xsurface,x0-r,y0,color);
		SetPixel(xsurface,x0+(radius-r),y0,color);
		SetPixel(xsurface,x0,y0-r,color);
		SetPixel(xsurface,x0,y0+(radius-r),color);		
		long R2=radius*radius;      
		int iShift = (r > 255) ? 12 : 14;
		R2<<=iShift;
		uint8_t dr;
		for(;x>y;)
		{	  
			R2-=((y<<(iShift+3))+(1<<(iShift+2)));
			++y;	  	   
			x=julery_isqrt(R2);
			dr=(~x)&0xff;	   
			x>>=((iShift>>1) + 1);
			if(x<r)
			{
				++x;	
			}			
			Pixel8Sym(pixels,x0,y0, x,y,(I*((dr)&0xff))>>8, color,yinc,w,h);		
			Pixel8Sym(pixels,x0,y0, x-1,y,(I*((~dr)&0xff))>>8, color,yinc,w,h);			
			T = dr;
		}
	}
}
void AACircleEx(PXSurface xsurface,int x0,int y0,int radius,Pixel color, uint8_t alpha)
{	
	if(!xsurface)
		return;
	long w=xsurface->width-1;
	long h=xsurface->height-1;		
	long r=radius>>1;
	if(r==0)
	{		
		if(radius)
		{
			SetPixel(xsurface,x0,y0,color, alpha);
			SetPixel(xsurface,x0+1,y0,color, alpha);
			SetPixel(xsurface,x0,y0+1,color, alpha);
			SetPixel(xsurface,x0+1,y0+1,color, alpha);
		}
		else
		{
			SetPixel(xsurface,x0,y0,color, alpha);
		}
		return;
	}
	if((x0+r)>=0&&(x0-r)<=w&&(y0+r)>=0&&(y0-r)<=h)
	{
		long yinc=xsurface->pitch>>1;
		Pixel *pixels=(Pixel *)xsurface->pixel;
		int x = r;
		int y = 0;	
		long T = 0;
		const long I = long(0.93*double(1<<8)); 		
		SetPixel(xsurface,x0-r,y0,color, alpha);
		SetPixel(xsurface,x0+(radius-r),y0,color, alpha);
		SetPixel(xsurface,x0,y0-r,color, alpha);
		SetPixel(xsurface,x0,y0+(radius-r),color, alpha);
		long R2=radius*radius;      
		int iShift = (r > 255) ? 12 : 14;
		R2<<=iShift;
		uint8_t dr;
		for(;x>y;)
		{	  
			R2-=((y<<(iShift+3))+(1<<(iShift+2)));
			++y;	  	   
			x=julery_isqrt(R2);
			dr=(~x)&0xff;	   
			x>>=((iShift>>1) + 1);
			if(x<r)
			{
				++x;	
			}			
			Pixel8SymEx(pixels,x0,y0, x,y,(I*((dr)&0xff))>>8, color,yinc,w,h,alpha);
 			if (x!=y-1 && x!=y)
				Pixel8SymEx(pixels,x0,y0, x-1,y,(I*((~dr)&0xff))>>8, color,yinc,w,h,alpha);
			T = dr;
		}
	}
}
static void Pixel4SymFill(Pixel *pixel,const long &cx,const long &cy,const long &x,const long &y,const long &t,const Pixel &color,const long &yinc,const long &w,const long &h)
{	
	long x1=cx-x;
	long x2=cx+x;
	long y1=cy-y;
	long y2=cy+y;	
	uint8_t code1=GetCode(y1,y2,0,h);
	Pixel* pixel0,*pixel1;
	if(y1<=y2)
	{
		uint8_t code2=GetCode(x1,x2,0,w);
		if(x1<=x2)
		{
			long y1_inc=y1*yinc;
			long y2_inc=y2*yinc;			
			if(code1&0x01)
			{	
				pixel0=pixel+(x1+y1_inc);
				pixel1=pixel+(x2+y1_inc);
				if(code2&0x01)
				{
					AAPixelColor(pixel0,color,t);
					++pixel0;
				}
				if(code2&0x02)
				{
					AAPixelColor(pixel1,color,t);
				}
				for(;pixel0<pixel1;++pixel0)
				{
					*pixel0=color;
				}
			}
			if(code1&0x02)
			{
				pixel0=pixel+(x1+y2_inc);
				pixel1=pixel+(x2+y2_inc);
				if(code2&0x01)
				{
					AAPixelColor(pixel0,color,t);
					++pixel0;
				}
				if(code2&0x02)
				{
					AAPixelColor(pixel1,color,t);
				}
				for(;pixel0<pixel1;++pixel0)
				{
					*pixel0=color;
				}
			}			
		}
	}		
}
static void Pixel4SymFillEx(Pixel *pixel,const long &cx,const long &cy,const long &x,const long &y,const long &t,const Pixel &color,const long &yinc,const long &w,const long &h,uint8_t alpha)
{	
	long x1=cx-x;
	long x2=cx+x;
	long y1=cy-y;
	long y2=cy+y;	
	uint8_t code1=GetCode(y1,y2,0,h);
	Pixel* pixel0,*pixel1;
	Pixel colorAntilias;
	if(y1<=y2)
	{
		uint8_t code2=GetCode(x1,x2,0,w);
		if(x1<=x2)
		{
			long y1_inc=y1*yinc;
			long y2_inc=y2*yinc;			
			if(code1&0x01)
			{	
				pixel0=pixel+(x1+y1_inc);
				pixel1=pixel+(x2+y1_inc);
				if(code2&0x01)
				{
					colorAntilias = *pixel0;
					AAPixelColor(&colorAntilias,color,t);
					PixelAlphaBlend(colorAntilias, *pixel0, alpha);
					++pixel0;
				}
				if(code2&0x02)
				{
					colorAntilias = *pixel1;
					AAPixelColor(&colorAntilias,color,t);
					PixelAlphaBlend(colorAntilias, *pixel1, alpha);
				}
				for(;pixel0<pixel1;++pixel0)
				{
					PixelAlphaBlend(color, *pixel0, alpha);
				}
			}
			if(code1&0x02)
			{
				pixel0=pixel+(x1+y2_inc);
				pixel1=pixel+(x2+y2_inc);
				if(code2&0x01)
				{
					colorAntilias = *pixel0;
					AAPixelColor(&colorAntilias,color,t);
					PixelAlphaBlend(colorAntilias, *pixel0, alpha);
					++pixel0;
				}
				if(code2&0x02)
				{
					colorAntilias = *pixel1;
					AAPixelColor(&colorAntilias,color,t);
					PixelAlphaBlend(colorAntilias, *pixel1, alpha);
				}
				for(;pixel0<pixel1;++pixel0)
				{
					PixelAlphaBlend(color, *pixel0, alpha);
				}
			}			
		}
	}		
}
void Pixel8SymFill(Pixel *pixels,const long &cx,const long &cy,const long &x,const long &y,const long &t,const Pixel &color,const long &yinc,const long &w,const long &h)
{
	Pixel4SymFill(pixels,cx,cy,x,y,t,color,yinc,w,h);
	if(x!=y)
		Pixel4SymFill(pixels,cx,cy,y,x,t,color,yinc,w,h);			
	
}
void Pixel8SymFillEx(Pixel *pixels,const long &cx,const long &cy,const long &x,const long &y,const long &t,const Pixel &color,const long &yinc,const long &w,const long &h, uint8_t alpha)
{
	Pixel4SymFillEx(pixels,cx,cy,x,y,t,color,yinc,w,h, alpha);
 	if (x!=y && x!=y-1 && y!=x-1)
	{
		if (pCirFill[x])
		{
			if (y>pCirFill[x]->nX)
			{
				pCirFill[x]->nX = y;
				pCirFill[x]->cx = cx;
				pCirFill[x]->cy = cy;
				pCirFill[x]->t = t;
				pCirFill[x]->yinc = yinc;
				pCirFill[x]->w = w;
				pCirFill[x]->h = h;
			}
		}
 		else
 		{
			pCirFill[x] = new CIRCLEFILL;
			pCirFill[x]->nX = y;
			pCirFill[x]->cx = cx;
			pCirFill[x]->cy = cy;
			pCirFill[x]->t = t;
			pCirFill[x]->yinc = yinc;
			pCirFill[x]->w = w;
			pCirFill[x]->h = h;
  		}
		//Pixel4SymFillEx(pixels,cx,cy,y,x,t,color,yinc,w,h,alpha);
	}
}
void FillAACircle(PXSurface xsurface,int x0, int y0, int radius, Pixel color)
{	
	if(!xsurface)
		return;
	long w=xsurface->width-1;
	long h=xsurface->height-1;		
	long r=radius>>1;
	if(r==0)
	{
		if(radius)
		{
			SetPixel(xsurface,x0,y0,color);
			SetPixel(xsurface,x0+1,y0,color);
			SetPixel(xsurface,x0,y0+1,color);
			SetPixel(xsurface,x0+1,y0+1,color);
		}
		else
		{
			SetPixel(xsurface,x0,y0,color);
		}
		return;
	}
	if((x0+r)>=0&&(x0-r)<=w&&(y0+r)>=0&&(y0-r)<=h)
	{		
		int x = radius;
		int y = 0;			
		const long I = long(0.93*double(1<<8));  			
		HLine(xsurface,x0-r,y0,radius+x0-r,color);
		VLine(xsurface,x0,y0-r,radius+y0-r,color);		
		long R2=radius*radius;      
		int iShift = (r > 255) ? 12 : 14;
		R2<<=iShift;
		long yinc=xsurface->pitch>>1;
		Pixel *pixels=(Pixel*)xsurface->pixel;
		uint8_t dr;
		for(;x>y;)
		{	  
			R2-=((y<<(iShift+3))+(1<<(iShift+2)));   
			++y;	  	   
			x=julery_isqrt(R2);
			dr=(~x)&0xff;	   
			x>>=((iShift>>1) + 1);
			if(x<r)
			{
				++x;	
			}
			Pixel8SymFill(pixels,x0,y0, x,y,(I*((dr)&0xff))>>8, color,yinc,w,h);			   			
	   }
	}
}
void FillAACircleEx(PXSurface xsurface,int x0, int y0, int radius, Pixel color,uint8_t alpha)
{	
	if(!xsurface)
		return;
	long w=xsurface->width-1;
	long h=xsurface->height-1;		
	long r=radius>>1;
	if(r==0)
	{
		if(radius)
		{
			SetPixel(xsurface,x0,y0,color,alpha);
			SetPixel(xsurface,x0+1,y0,color,alpha);
			SetPixel(xsurface,x0,y0+1,color,alpha);
			SetPixel(xsurface,x0+1,y0+1,color,alpha);
		}
		else
		{
			SetPixel(xsurface,x0,y0,color,alpha);
		}
		return;
	}
	if((x0+r)>=0&&(x0-r)<=w&&(y0+r)>=0&&(y0-r)<=h)
	{		
		int x = radius;
		int y = 0;			
		const long I = long(0.93*double(1<<8));  			
		HLine(xsurface,x0-r,y0,radius+x0-r+1,color,alpha);
// 		VLine(xsurface,x0,y0-r,radius+y0-r,color,alpha);		
		long R2=radius*radius;      
		int iShift = (r > 255) ? 12 : 14;
		R2<<=iShift;
		long yinc=xsurface->pitch>>1;
		Pixel *pixels=(Pixel*)xsurface->pixel;
		uint8_t dr;
		memset(pCirFill, 0, MAXVERTICAL*sizeof(CIRCLEFILL*));
		for(;x>y;)
		{	  
			R2-=((y<<(iShift+3))+(1<<(iShift+2)));   
			++y;	  	   
			x=julery_isqrt(R2);
			dr=(~x)&0xff;	   
			x>>=((iShift>>1) + 1);
			if(x<r)
			{
				++x;	
			}
			Pixel8SymFillEx(pixels,x0,y0, x,y,(I*((dr)&0xff))>>8, color,yinc,w,h,alpha);
	   }
	   for (int i=0; i<MAXVERTICAL; i++)
		   if (pCirFill[i])
		   {
				Pixel4SymFillEx(pixels,pCirFill[i]->cx,pCirFill[i]->cy,pCirFill[i]->nX,i,pCirFill[i]->t,color,pCirFill[i]->yinc,pCirFill[i]->w,pCirFill[i]->h,alpha);
				delete pCirFill[i];
		   }
	}
}

void AACircle2(PXSurface xsurface,int x0,int y0,int radius,Pixel color);
void AAPolyOutLine( PXSurface xsurface,POINT *points, int cnt, int w,Pixel color)
{
	if(w>1)	
	{
		int i=0;
		int nMofWid = w;
		if (nMofWid%2)
			nMofWid++;
		for(;i<cnt-1;i++)
		{	
			AACircle(xsurface, points[i].x, points[i].y, w, color);//w
			WideOutline(xsurface, points[i].x, points[i].y, points[i+1].x, points[i+1].y, color, w);
		}
		//AACircle(xsurface,points[i].x, points[i].y, nMofWid, color);//w or nMofWid?
		AACircle2(xsurface,points[i].x, points[i].y, nMofWid, color);
	}
}

void AAPolyOutLineEx( PXSurface xsurface,POINT *points, int cnt, int w,Pixel color,uint8_t alpha)
{
	if(w>1)	
	{
		int i=0;
		int nMofWid = w;
		if (nMofWid%2)
			nMofWid++;
		for(;i<cnt-1;i++)
		{	
			AACircleEx(xsurface,points[i].x,points[i].y,nMofWid,color,alpha);
			WideOutlineEx(xsurface,points[i].x,points[i].y,points[i+1].x,points[i+1].y,color,w, alpha);
		}
		AACircleEx(xsurface,points[i].x,points[i].y,nMofWid,color,alpha);
	}
}

void AAPolyInLine( PXSurface xsurface,POINT *points, int pointCount, int w,Pixel color,Pixel coloroutline )
{
	int i;
	int nMofWid = w;
	if (nMofWid%2)
		nMofWid++;
	for(i=0;i<pointCount-1;i++)
	{			
		FillAACircle(xsurface,points[i].x,points[i].y,(w/*nMofWid*/-2),color);
		WideInline(xsurface,points[i].x,points[i].y,points[i+1].x,points[i+1].y,color,w,coloroutline);					
	}
	FillAACircle(xsurface,points[i].x,points[i].y,(nMofWid-2),color);	
}
void AAPolyInLineEx( PXSurface xsurface,POINT *points, int pointCount, int w,Pixel color,Pixel coloroutline ,uint8_t alpha)
{	
	int i;
	int nMofWid = w;
	if (nMofWid%2)
		nMofWid++;
	for(i=0;i<pointCount-1;i++)
	{			
		FillAACircleEx(xsurface,points[i].x,points[i].y,(nMofWid-2),color,alpha);
		WideInlineEx(xsurface,points[i].x,points[i].y,points[i+1].x,points[i+1].y,color,w,coloroutline,alpha);
	}
	FillAACircleEx(xsurface,points[i].x,points[i].y,(nMofWid-2),color,alpha);
}
static void _BlitStretch( PXSurface destsurface,const RECT& dest, const PXSurface source, const RECT* srcRect_,int alpha,Pixel colormark,PIXEL_ALPHA_BLEND_MARK PixelFunc)
{
	// Loai bo truong hop ko can ve
	if (dest.left>=dest.right || dest.top>=dest.bottom ||
		dest.right<=0 || dest.left>=destsurface->width ||
		dest.bottom<=0 || dest.top>=destsurface->height )	
		return;

	RECT srcRect;
	if(srcRect_)
		srcRect = *srcRect_;
	else
	{
		srcRect.left = srcRect.top = 0;
		srcRect.right = source->width;
		srcRect.bottom = source->height;
	}

	RECT destRect = dest;	//{max(0,dest.left), max(0,dest.top), min(destsurface->width,dest.right), min(destsurface->height,dest.bottom)};

	unsigned rx = unsigned(GetRectWidth(srcRect) << 10) / unsigned(GetRectWidth(destRect));
	unsigned ry = unsigned(GetRectHeight(srcRect) << 10) / unsigned(GetRectHeight(destRect));    
	//unsigned ry = rx;

	unsigned T[3][2];

	T[0][0] = rx;
	T[0][1] = 0;
	T[1][0] = 0;
	T[1][1] = ry;
	T[2][0] = -destRect.left * rx + (srcRect.left << 10);
	T[2][1] = -destRect.top * ry + (srcRect.top << 10);

	// Optimization: compensate for fixed point arithmetic in rasterization loops
	T[2][0] <<= 6;
	T[2][1] <<= 6;

	// Hieu chinh de khong ra ngoai man hinh
	destRect.left = _Max(0,dest.left);
	destRect.top = _Max(0,dest.top);
	destRect.right = _Min(destsurface->width,dest.right);
	destRect.bottom = _Min(destsurface->height,dest.bottom);
	unsigned y1 = (destRect.top << 6) + (1<<5);
	unsigned y2 = (destRect.bottom << 6) + (1<<5);
	unsigned x1 = (destRect.left << 6) + (1<<5);
	unsigned x2 = (destRect.right << 6) + (1<<5);

	unsigned u0 = x1 * T[0][0] + y1 * T[1][0] + T[2][0];
	unsigned v0 = x1 * T[0][1] + y1 * T[1][1] + T[2][1];

	unsigned dudx = T[0][0] << 6;
	unsigned dvdx = T[0][1] << 6;
	unsigned dudy = T[1][0] << 6;
	unsigned dvdy = T[1][1] << 6;

	for (unsigned dy = y1; dy != y2; dy += (1<<6) )	{	
		Pixel* destpixel = GetPixels(destsurface, x1 >> 6, dy >> 6 );

		unsigned u = u0;
		unsigned v = v0;

		for (unsigned dx = x1; dx != x2; dx += (1<<6) )
		{
			unsigned ix = u >> 16;
			unsigned iy = v >> 16;

			if (ix < (unsigned)source->width && iy < (unsigned)source->height )
			{
				//*destpixel = *GetPixels(source,ix,iy);
				PixelFunc(*GetPixels(source,ix,iy),*destpixel,alpha,colormark);
			}

			u += dudx;
			v += dvdx;
			++destpixel;
		}

		u0 += dudy;
		v0 += dvdy;
	}
}

void BlitStretch( PXSurface destsurface,const RECT& destRect, const PXSurface source, const RECT* srcRect_,int alpha,int colormark)
{
	if(colormark==-1)
	{
		if ( (alpha | 0x03) == 0xFF)
		{			
			_BlitStretch(destsurface,destRect,source,srcRect_,alpha,colormark,PixelAlphaBlendMark0);
			
		}
		else
		{						
			_BlitStretch(destsurface,destRect,source,srcRect_,alpha,colormark,PixelAlphaBlendMark1);
		}
	}
	else//mark
	{
		Pixel _colormark=Color((COLORREF)colormark);
		if ( (alpha | 0x03) == 0xFF)
		{
			_BlitStretch(destsurface,destRect,source,srcRect_,alpha,_colormark,PixelAlphaBlendMark2);			
		}
		else
		{
			_BlitStretch(destsurface,destRect,source,srcRect_,alpha,_colormark,PixelAlphaBlendMark3);
		}
	}
}
#ifdef OS_WIN
void SetXSurfaceDC(HDC hdc,PXSurface xsurface)
{
	if (xsurface)
	{
		::SelectObject(hdc,xsurface->hbmp);
	}
}
#endif // OS_WIN

PXSurface CloneXSurface(PXSurface xsurface)
{
	PXSurface clone_surface=0;
	if(xsurface)
	{
		clone_surface=::CreateXSurface(xsurface->width,xsurface->height);
		memcpy(clone_surface->pixel,xsurface->pixel,xsurface->pitch*xsurface->height);
	}
	return clone_surface;
}
void DrawPoly(PXSurface xsurface,POINT *pts,int ncnt,Pixel color,bool bPolygon,bool bAntialias)
{
	POINT *curpt,*nextpt,*lastpt;
	lastpt=pts+ncnt;
	if(bPolygon)
	{
		curpt=lastpt-1;
		nextpt=pts;
	}
	else
	{
		curpt=pts;
		nextpt=pts+1;
	}
	for(;nextpt<lastpt;curpt=nextpt++)
	{
		Line(xsurface,curpt->x,curpt->y,nextpt->x,nextpt->y,color,bAntialias);		
	}
}

void LineFocus(PXSurface xsurface,int x0,int y0,int x1,int y1)
{
	if(xsurface)
	{
		xsurface->PixelFunc=&NOTXORPEN_PixelFunc;
		Line(xsurface,x0,y0,x1,y1,FocusColor,false);
		xsurface->PixelFunc=&DefaultPixelFunc;
	}
}

void LineSnap(PXSurface xsurface, const POINT& pt, int nLength)
{
	if (xsurface)
	{
		xsurface->PixelFunc = &NOTXORPEN_PixelFunc;
		Line(xsurface, pt.x-nLength, pt.y, pt.x+nLength, pt.y, Color(0, 0, 255), false);
		Line(xsurface, pt.x, pt.y-nLength, pt.x, pt.y+nLength, Color(0, 0, 255), false);
		xsurface->PixelFunc = &DefaultPixelFunc;
	}
}

void Tracker(PXSurface xsurface,int x0,int y0,int x1,int y1)
{
	if(ClipRect(xsurface,x0,y0,x1,y1))
	{
		xsurface->PixelFunc=&NOTPEN_PixelFunc;
		int l=x1-x0;
		for(;y0<y1;++y0)
		{
			_HLine(xsurface,x0,y0,l,0);
		}	
		xsurface->PixelFunc=&DefaultPixelFunc;
	}
}
#define ELLIPSE_PIXEL()	X0=x0+x;X1=x0-x;Y0=y0+y;Y1=y0-y; \
	if(X0>=0&&X0<xsurface->width){\
	if(Y0>=0&&Y0<xsurface->height) xsurface->PixelFunc(color,*GetPixels(xsurface,X0,Y0));\
	if(Y1>=0&&Y1<xsurface->height&&y) xsurface->PixelFunc(color,*GetPixels(xsurface,X0,Y1));}\
	if(X1>=0&&X1<xsurface->width&&x){\
	if(Y0>=0&&Y0<xsurface->height) xsurface->PixelFunc(color,*GetPixels(xsurface,X1,Y0));\
	if(Y1>=0&&Y1<xsurface->height&&y) xsurface->PixelFunc(color,*GetPixels(xsurface,X1,Y1));}
#define ELLIPSE_PIXELEX()	X0=x0+x;X1=x0-x;Y0=y0+y;Y1=y0-y; \
	if(X0>=0&&X0<xsurface->width){\
	if(Y0>=0&&Y0<xsurface->height) PixelAlphaBlend(color, *GetPixels(xsurface,X0,Y0), alpha);\
	if(Y1>=0&&Y1<xsurface->height&&y) PixelAlphaBlend(color,*GetPixels(xsurface,X0,Y1), alpha);}\
	if(X1>=0&&X1<xsurface->width&&x){\
	if(Y0>=0&&Y0<xsurface->height) PixelAlphaBlend(color,*GetPixels(xsurface,X1,Y0),alpha);\
	if(Y1>=0&&Y1<xsurface->height&&y) PixelAlphaBlend(color,*GetPixels(xsurface,X1,Y1),alpha);}
void EllipseDefaultPixelFunc(PXSurface xsurface,int x0,int y0,int x,int y,Pixel color,int iWidth)
{
	int X0,Y0,X1,Y1;
	ELLIPSE_PIXEL();
}
void EllipseDefaultPixelFuncEx(PXSurface xsurface,int x0,int y0,int x,int y,Pixel color,int iWidth,uint8_t alpha)
{
	int X0,Y0,X1,Y1;
	ELLIPSE_PIXELEX();
}
static void EllipseFillRect(PXSurface xsurface,int x,int y,Pixel color,int iWidth, uint8_t alpha = 255)
{
	x-=iWidth>>1;
	y-=iWidth>>1;
	::FillRect(xsurface,x,y,x+iWidth,y+iWidth,color,alpha);
}
void EllipseWidePixelFunc(PXSurface xsurface,int x0,int y0,int x,int y,Pixel color,int iWidth)
{
	int X0,Y0,X1,Y1;
	X0=x0+x;
	X1=x0-x;
	Y0=y0+y;
	Y1=y0-y;
	EllipseFillRect(xsurface,X0,Y0,color,iWidth);
	EllipseFillRect(xsurface,X0,Y1,color,iWidth);
	EllipseFillRect(xsurface,X1,Y0,color,iWidth);
	EllipseFillRect(xsurface,X1,Y1,color,iWidth);
}
void EllipseWidePixelFuncEx(PXSurface xsurface,int x0,int y0,int x,int y,Pixel color,int iWidth,uint8_t alpha)
{
	int X0,Y0,X1,Y1;
	X0=x0+x;
	X1=x0-x;
	Y0=y0+y;
	Y1=y0-y;
	EllipseFillRect(xsurface,X0,Y0,color,iWidth,alpha);
	EllipseFillRect(xsurface,X0,Y1,color,iWidth,alpha);
	EllipseFillRect(xsurface,X1,Y0,color,iWidth,alpha);
	EllipseFillRect(xsurface,X1,Y1,color,iWidth,alpha);
}
void Ellipse( PXSurface xsurface, int x0,int y0, int rx,int ry, Pixel color,int iWidth )
{
	/*long t1 = rx*rx;
	long t2 = t1<<1;
	long t3 = t2<<1;
	long t4 = ry*ry;
	long t5 = t4<<1;
	long t6 = t5<<1;
	long t7 = rx*t5;
	long t8 = t7<<1;
	long t9 = 0L;
	long d1 = t2 - t7 + (t4>>1);	// error terms 
	long d2 = (t1>>1) - t8 + t5;*/

	if (iWidth==0)
		return;
	
	__int64 t1 = rx*rx;
	__int64 t2 = t1<<1;
	__int64 t3 = t2<<1;
	__int64 t4 = ry*ry;
	//check large ellipse
	POINT pt1, pt2; pt1.x = -1; pt2.x = -1;
	if(rx > 32000 || ry > 32000)	{
		Algorithm Eclipse;
		const POINT *pECLipseR;
		Eclipse.SetEclipse(x0, y0, rx, ry);
		Eclipse.SetRectangle(xsurface->width, xsurface->height);
		pECLipseR = Eclipse.EclipseIntersection();
		if(pECLipseR[1].x != -1)	
			WideLine(xsurface, pECLipseR[0].x, pECLipseR[0].y, pECLipseR[1].x, pECLipseR[1].y, color, iWidth, false);
		return;		
	}
	__int64 t5 = t4<<1;
	__int64 t6 = t5<<1;
	__int64 t7 = rx*t5;
	__int64 t8 = t7<<1;
	__int64 t9 = 0L;
	__int64 d1 = t2 - t7 + (t4>>1);	/* error terms */
	__int64 d2 = (t1>>1) - t8 + t5;

	void (*EllipsePixelFunc)(PXSurface ,int ,int ,int ,int ,Pixel,int )=EllipseDefaultPixelFunc;
	if(iWidth>1)
	{
		EllipsePixelFunc=EllipseWidePixelFunc;
	}

	int x = rx, y = 0;	/* ellipse points */
	

	while (d2 < 0)			/* til slope = -1 */
	{
		EllipsePixelFunc(xsurface,x0,y0,x,y,color,iWidth);
		//ELLIPSE_PIXEL();		
		y++;		/* always move up here */
		t9 += t3;	
		if (d1 < 0)	/* move straight up */
		{
			d1 += t9 + t2;
			d2 += t9;
		}
		else		/* move up and left */
		{
			x--;
			t8 -= t6;
			d1 += t9 + t2 - t8;
			d2 += t9 + t5 - t8;
		}
	}

	do 				/* rest of top right quadrant */
	{
		EllipsePixelFunc(xsurface,x0,y0,x,y,color,iWidth);
		x--;		/* always move left here */
		t8 -= t6;	
		if (d2 < 0)	/* move up and left */
		{
			y++;
			t9 += t3;
			d2 += t9 + t5 - t8;
		}
		else		/* move straight left */
			d2 += t5 - t8;
	} while (x>=0);
}
void EllipseEx( PXSurface xsurface, int x0,int y0, int rx,int ry, Pixel color, uint8_t alpha, int iWidth)
{
	if (iWidth==0)
		return;
	__int64 t1 = rx*rx;
	__int64 t2 = t1<<1;
	__int64 t3 = t2<<1;
	__int64 t4 = ry*ry;
	//check large ellipse
	POINT pt1, pt2; pt1.x = -1; pt2.x = -1;
	if(rx > 32000 || ry > 32000)	{
		Algorithm Eclipse;
		const POINT *pECLipseR;
		Eclipse.SetEclipse(x0, y0, rx, ry);
		Eclipse.SetRectangle(xsurface->width, xsurface->height);
		pECLipseR = Eclipse.EclipseIntersection();
		if(pECLipseR[1].x != -1)	
			WideLineEx(xsurface, pECLipseR[0].x, pECLipseR[0].y, pECLipseR[1].x, pECLipseR[1].y, color, iWidth, alpha, false);
		return;		
	}
	__int64 t5 = t4<<1;
	__int64 t6 = t5<<1;
	__int64 t7 = rx*t5;
	__int64 t8 = t7<<1;
	__int64 t9 = 0L;
	__int64 d1 = t2 - t7 + (t4>>1);	/* error terms */
	__int64 d2 = (t1>>1) - t8 + t5;

	void (*EllipsePixelFuncEx)(PXSurface ,int ,int ,int ,int ,Pixel, int, uint8_t )=EllipseDefaultPixelFuncEx;
	if(iWidth>1)
	{
		EllipsePixelFuncEx = EllipseWidePixelFuncEx;
	}

	int x = rx, y = 0;	/* ellipse points */
	

	while (d2 < 0)			/* til slope = -1 */
	{
		EllipsePixelFuncEx(xsurface,x0,y0,x,y,color,iWidth,alpha);
		//ELLIPSE_PIXEL();		
		y++;		/* always move up here */
		t9 += t3;	
		if (d1 < 0)	/* move straight up */
		{
			d1 += t9 + t2;
			d2 += t9;
		}
		else		/* move up and left */
		{
			x--;
			t8 -= t6;
			d1 += t9 + t2 - t8;
			d2 += t9 + t5 - t8;
		}
	}

	do 				/* rest of top right quadrant */
	{
		EllipsePixelFuncEx(xsurface,x0,y0,x,y,color,iWidth,alpha);
		x--;		/* always move left here */
		t8 -= t6;	
		if (d2 < 0)	/* move up and left */
		{
			y++;
			t9 += t3;
			d2 += t9 + t5 - t8;
		}
		else		/* move straight left */
			d2 += t5 - t8;
	} while (x>=0);
}
void EllipseFocus( PXSurface xsurface, int x0,int y0, int rx,int ry)
{
	if(xsurface)
	{
		xsurface->PixelFunc=&NOTXORPEN_PixelFunc;
		Ellipse(xsurface,x0,y0,rx,ry,FocusColor,1);
		xsurface->PixelFunc=&DefaultPixelFunc;
	}
}

static void HFillLine(PXSurface xsurface,int x0,int y0,int x1,Pixel color)
{
	if(y0>=0&&y0<xsurface->height)
	{
		if(x0<0)
			x0=0;	
		if(x1>xsurface->width)
			x1=xsurface->width;
		Pixel *pixel=GetPixels(xsurface,x0,y0);
		for(;x0<x1;++x0,++pixel)
		{
			if(xsurface->Pattern(x0,y0))
				xsurface->PixelFunc(color,*pixel);
		}
	}
}
static void HFillLineEx(PXSurface xsurface,int x0,int y0,int x1,Pixel color,uint8_t alpha)
{
	if(y0>=0&&y0<xsurface->height)
	{
		if(x0<0)
			x0=0;	
		if(x1>xsurface->width)
			x1=xsurface->width;
		Pixel *pixel=GetPixels(xsurface,x0,y0);
		for(;x0<x1;++x0,++pixel)
		{
			if(xsurface->Pattern(x0,y0))
				PixelAlphaBlend(color, *pixel, alpha);
		}
	}
}
void FillEllipse( PXSurface xsurface,int x0,int y0, int rx, int ry, Pixel color,int fp )
{
	if(xsurface==0)
		return;
	SetPatternFunc(xsurface,fp);
	/*long t1 = rx*rx;
	long t2 = t1<<1;
	long t3 = t2<<1;
	long t4 = ry*ry;
	long t5 = t4<<1;
	long t6 = t5<<1;
	long t7 = rx*t5;
	long t8 = t7<<1;
	long t9 = 0L;
	long d1 = t2 - t7 + (t4>>1);	// error terms 
	long d2 = (t1>>1) - t8 + t5;*/
	
	__int64 t1 = rx*rx;
	__int64 t2 = t1<<1;
	__int64 t3 = t2<<1;
	__int64 t4 = ry*ry;
	__int64 t5 = t4<<1;
	__int64 t6 = t5<<1;
	__int64 t7 = rx*t5;
	__int64 t8 = t7<<1;
	__int64 t9 = 0L;
	__int64 d1 = t2 - t7 + (t4>>1);	// error terms 
	__int64 d2 = (t1>>1) - t8 + t5;

	int x = rx, y = 0;	/* ellipse points */
	while (d2 < 0)			/* til slope = -1 */
	{
		if(x>1){
			HFillLine(xsurface,x0-x,y0+y,x0+x+1,color);			
			if(y)
				HFillLine(xsurface,x0-x,y0-y,x0+x+1,color);				
		}		
		y++;		/* always move up here */
		t9 += t3;	
		if (d1 < 0)	/* move straight up */
		{
			d1 += t9 + t2;
			d2 += t9;
		}
		else		/* move up and left */
		{
			x--;
			t8 -= t6;
			d1 += t9 + t2 - t8;
			d2 += t9 + t5 - t8;
		}
	}

	do 				/* rest of top right quadrant */
	{
		if(x>1){
			HFillLine(xsurface,x0-x,y0+y,x0+x+1,color);			
			if(y)
				HFillLine(xsurface,x0-x,y0-y,x0+x+1,color);				
		}
		x--;		/* always move left here */
		t8 -= t6;	
		if (d2 < 0)	/* move up and left */
		{
			y++;
			t9 += t3;
			d2 += t9 + t5 - t8;
		}
		else		/* move straight left */
			d2 += t5 - t8;
	} while (x>=0);
	xsurface->Pattern=FT_SOLID_CHECK;
}
void FillEllipseEx( PXSurface xsurface,int x0,int y0, int rx, int ry, Pixel color,int fp, uint8_t alpha)
{
	if(xsurface==0)
		return;
	SetPatternFunc(xsurface,fp);
	/*long t1 = rx*rx;
	long t2 = t1<<1;
	long t3 = t2<<1;
	long t4 = ry*ry;
	long t5 = t4<<1;
	long t6 = t5<<1;
	long t7 = rx*t5;
	long t8 = t7<<1;
	long t9 = 0L;
	long d1 = t2 - t7 + (t4>>1);	// error terms 
	long d2 = (t1>>1) - t8 + t5;*/
	
	__int64 t1 = rx*rx;
	__int64 t2 = t1<<1;
	__int64 t3 = t2<<1;
	__int64 t4 = ry*ry;
	__int64 t5 = t4<<1;
	__int64 t6 = t5<<1;
	__int64 t7 = rx*t5;
	__int64 t8 = t7<<1;
	__int64 t9 = 0L;
	__int64 d1 = t2 - t7 + (t4>>1);	// error terms 
	__int64 d2 = (t1>>1) - t8 + t5;

	int x = rx, y = 0;	/* ellipse points */
	memset(pCirFill, 0, MAXVERTICAL*sizeof(CIRCLEFILL*));
	while (d2 < 0)			/* til slope = -1 */
	{
		if(x>1 && y<MAXVERTICAL)
		{
			if (pCirFill[y])
			{
				if (pCirFill[y]->nX>x)
					pCirFill[y]->nX = x;
			}
			else
			{
				pCirFill[y] = new CIRCLEFILL;
				pCirFill[y]->nX = x;
			}
		}		
		y++;		/* always move up here */
		t9 += t3;	
		if (d1 < 0)	/* move straight up */
		{
			d1 += t9 + t2;
			d2 += t9;
		}
		else		/* move up and left */
		{
			x--;
			t8 -= t6;
			d1 += t9 + t2 - t8;
			d2 += t9 + t5 - t8;
		}
	}

	do 				/* rest of top right quadrant */
	{
		if (x>1 && y<MAXVERTICAL)
		{
			if (pCirFill[y])
			{
				if (pCirFill[y]->nX>x)
					pCirFill[y]->nX = x;
			}
			else
			{
				pCirFill[y] = new CIRCLEFILL;
				pCirFill[y]->nX = x;
			}
		}
		x--;		/* always move left here */
		t8 -= t6;	
		if (d2 < 0)	/* move up and left */
		{
			y++;
			t9 += t3;
			d2 += t9 + t5 - t8;
		}
		else		/* move straight left */
			d2 += t5 - t8;
	} while (x>=0);
	for (int i=0; i<MAXVERTICAL; i++)
	   if (pCirFill[i])
	   {
			HFillLineEx(xsurface,x0-pCirFill[i]->nX,y0+i,x0+pCirFill[i]->nX+1,color,alpha);
			if(i)
				HFillLineEx(xsurface,x0-pCirFill[i]->nX,y0-i,x0+pCirFill[i]->nX+1,color,alpha);
			delete pCirFill[i];
	   }
	xsurface->Pattern=FT_SOLID_CHECK;
}
void FillRect(PXSurface xsurface,int x0,int y0,int x1,int y1,Pixel color, int fp)
{
	if(ClipRect(xsurface,x0,y0,x1,y1))
	{
		SetPatternFunc(xsurface,fp);
		for(;y0<y1;++y0)
		{			
			for(int x=x0;x<x1;x++)
			{
				if(xsurface->Pattern(x,y0))
					xsurface->PixelFunc(color,*GetPixels(xsurface,x,y0));
			}
		}	
		xsurface->Pattern=FT_SOLID_CHECK;
	}	
}
void FillRectEx(PXSurface xsurface,int x0,int y0,int x1,int y1,Pixel color, int fp, uint8_t alpha)
{
	if(ClipRect(xsurface,x0,y0,x1,y1))
	{
		SetPatternFunc(xsurface,fp);
		for(;y0<y1;++y0)
		{			
			for(int x=x0;x<x1;x++)
			{
				if(xsurface->Pattern(x,y0))
					PixelAlphaBlend(color, *GetPixels(xsurface,x,y0),alpha);
			}
		}	
		xsurface->Pattern=FT_SOLID_CHECK;
	}	
}

int CalculatePatternLength(const int* fp,int np)
{
	int sz=0;
	for(int i=0;i<np;i++)
	{
		sz+=fp[i];
	}
	return sz;
}
void MakeMarkPattern(char *mark_patten,const int* fp,int np)
{		
	for(int i=0;i<np;i++)
	{	
		//memset(mark_patten,(i&1),fp[i]);//first is space
		memset(mark_patten,1-(i&1),fp[i]);//dash first
		mark_patten+=fp[i];
	}
}
char* linePattern(PXSurface xsurface,int x0, int y0, int x1, int y1, Pixel color, char* pPattern, const int* fp,int np)
{	
	if(!xsurface||!ClipRectEx(x0,y0,x1,y1,0,0,xsurface->width-1,xsurface->height-1))
	{
		return NULL;
	}
	if(np<2)	{
		_Line(xsurface,x0,y0,x1,y1,color);
		return NULL;
	}
	char *pattern = NULL;
	char *ptt = NULL;
	int dy = y1 - y0;
	int dx = x1 - x0;

	if(!pPattern)	{		
		int size=abs(dx)+abs(dy);
		int npp=CalculatePatternLength(fp,np);
		size=(1+size/npp)*npp;
		pattern=new char[size];	
		MakeMarkPattern(pattern,fp,np);	
		char *lastpattern=pattern+size;
		ptt=pattern+npp;
		for(;ptt<lastpattern;ptt+=npp)
		{
			memcpy(ptt,pattern,npp);
		}
		ptt=pattern;
	}
	else
		ptt = pPattern;

	int stepx, stepy;

	if (dy < 0) { dy = -dy;  stepy = -xsurface->pitch; } else { stepy = xsurface->pitch; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;
	dx <<= 1;

	y0 *= xsurface->pitch;
	y1 *= xsurface->pitch;
	uint8_t *pixel=xsurface->pixel;
	//if(*ptt++)
	//{
		//*(Pixel*)(pixel+((x0<<1)+y0))=color;			
	//}
	if (dx > dy) {
		int fraction = dy - (dx >> 1);
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx;
			}
			x0 += stepx;
			fraction += dy;
			if(*ptt++)			
				*(Pixel*)(pixel+((x0<<1)+y0))=color;						
		}
	} else {
		int fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			if(*ptt++)
				*(Pixel*)(pixel+((x0<<1)+y0))=color;			
		}
	}
	if(!pPattern)	{
		delete []pattern;
		return NULL;
	}
	return ptt;
}

char* linePatternEx(PXSurface xsurface,int x0, int y0, int x1, int y1, Pixel color, char* pPattern, const int* fp,int np,uint8_t alpha)
{	
	if(!xsurface||!ClipRectEx(x0,y0,x1,y1,0,0,xsurface->width-1,xsurface->height-1))
	{
		return NULL;
	}
	if(np<2)	{
		_LineEx(xsurface,x0,y0,x1,y1,color,alpha);
		return NULL;
	}
	char *pattern = NULL;
	char *ptt = NULL;
	int dy = y1 - y0;
	int dx = x1 - x0;

	if(!pPattern)	{		
		int size=abs(dx)+abs(dy);
		int npp=CalculatePatternLength(fp,np);
		size=(1+size/npp)*npp;
		pattern=new char[size];	
		MakeMarkPattern(pattern,fp,np);	
		char *lastpattern=pattern+size;
		ptt=pattern+npp;
		for(;ptt<lastpattern;ptt+=npp)
		{
			memcpy(ptt,pattern,npp);
		}
		ptt=pattern;
	}
	else
		ptt = pPattern;

	int stepx, stepy;

	if (dy < 0) { dy = -dy;  stepy = -xsurface->pitch; } else { stepy = xsurface->pitch; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;
	dx <<= 1;

	y0 *= xsurface->pitch;
	y1 *= xsurface->pitch;
	uint8_t *pixel=xsurface->pixel;
	//if(*ptt++)
	//{
		//*(Pixel*)(pixel+((x0<<1)+y0))=color;			
	//}
	if (dx > dy) {
		int fraction = dy - (dx >> 1);
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx;
			}
			x0 += stepx;
			fraction += dy;
			if(*ptt++)			
				//*(Pixel*)(pixel+((x0<<1)+y0))=color;						
				PixelAlphaBlend(color, *(Pixel*)(pixel+((x0<<1)+y0)), alpha);
		}
	} else {
		int fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			if(*ptt++)
				//*(Pixel*)(pixel+((x0<<1)+y0))=color;
				PixelAlphaBlend(color, *(Pixel*)(pixel+((x0<<1)+y0)), alpha);
		}
	}
	if(!pPattern)	{
		delete []pattern;
		return NULL;
	}
	return ptt;
}

char* WideLinePattern (PXSurface xsurface, int x1, int y1, int x2, int y2,int width, Pixel color, char* pPattern, const int *fp,const int np)
{
	if (width==0)
		return NULL;
	if(!xsurface||!ClipRectEx(x1,y1,x2,y2,0,0,xsurface->width,xsurface->height))
	{
		return NULL;
	}
	if(np<2)
	{
		WideLine(xsurface,x1,y1,x2,y2,color,width,true);
		return NULL;
	}
	if(width<2)
		return linePattern(xsurface,x1,y1,x2,y2,color, pPattern, fp,np);
	
	int dx, dy, incr1, incr2, d, x, y, xend, yend, xdirflag, ydirflag;
	int wid;
	dx = abs (x2 - x1);
	dy = abs (y2 - y1);	

	char *pattern = NULL;
	char *ptt = NULL;
	if(!pPattern)	{
		int size=abs(dx)+abs(dy);
		int npp=CalculatePatternLength(fp,np);
		size=(1+size/npp)*npp;
		pattern=new char[size];	
		MakeMarkPattern(pattern,fp,np);	
		char *lastpattern=pattern+size;
		ptt=pattern+npp;
		for(;ptt<lastpattern;ptt+=npp)
		{
			memcpy(ptt,pattern,npp);
		}
		ptt=pattern;
	}
	else
		ptt = pPattern;
	if (dy <= dx)
	{
		/* More-or-less horizontal. use wid for vertical stroke */
		/* Doug Claar: watch out for NaN in atan2 (2.0.5) */
		if ((dx == 0) && (dy == 0))
		{
			wid = 1;
		}
		else
		{
			double ac = cos (atan2 ((double)dy, (double)dx));
			if (ac != 0)
			{
				wid = width / ac;
			}
			else
			{
				wid = 1;
			}
			if (wid == 0)
			{
				wid = 1;
			}
		}
		/* 2.0.12: Michael Schwartz: divide rather than multiply;
		TBB: but watch out for /0! */
		//wid=RoundDevInt(julery_isqrt((thick*thick*(dx*dx+dy*dy))),dx);
		if (wid == 0)
		{
			wid = 1;
		}
		d = (dy<<1) - dx;
		incr1 = dy<<1;
		incr2 = (dy - dx)<<1;
		if (x1 > x2)
		{
			x = x2;
			y = y2;
			ydirflag = (-1);
			xend = x1;
		}
		else
		{
			x = x1;
			y = y1;
			ydirflag = 1;
			xend = x2;
		}

		/* Set up line thickness */
		if(*ptt++)
			::VLine(xsurface,x,y - (wid >>1),y - (wid >>1)+wid,color);

		if (((y2 - y1) * ydirflag) > 0)
		{
			while (x < xend)
			{
				x++;
				if (d < 0)
				{
					d += incr1;
				}
				else
				{
					y++;
					d += incr2;
				}
				if(*ptt++)
					::VLine(xsurface,x,y - (wid >>1),y - (wid >>1)+wid,color);
				//this->DrawVLine(x,y - (wid >>1),wid,color);
			}
		}
		else
		{
			while (x < xend)
			{
				x++;
				if (d < 0)
				{
					d += incr1;
				}
				else
				{
					y--;
					d += incr2;
				}
				if(*ptt++)
					::VLine(xsurface,x,y - (wid >>1),y - (wid >>1)+wid,color);
				//this->DrawVLine(x,y - (wid >>1),wid,color);
			}
		}
	}
	else
	{
		/* More-or-less vertical. use wid for horizontal stroke */
		/* 2.0.12: Michael Schwartz: divide rather than multiply;
		TBB: but watch out for /0! */
		 double as = sin (atan2 ((double)dy, (double)dx));
		 if (as != 0)
		{
		  wid = width / as;
		}
		  else
		{
		  wid = 1;
		}
		//wid=RoundDevInt(julery_isqrt((thick*thick*(dx*dx+dy*dy))),dy);
		if (wid == 0)
			wid = 1;

		d = (dx<<1) - dy;
		incr1 = dx<<1;
		incr2 = (dx - dy)<<1;
		if (y1 > y2)
		{
			y = y2;
			x = x2;
			yend = y1;
			xdirflag = (-1);
		}
		else
		{
			y = y1;
			x = x1;
			yend = y2;
			xdirflag = 1;
		}

		/* Set up line thickness */
		if(*ptt++)
			::HLine(xsurface,x - (wid >>1),y,x - (wid >>1)+wid,color);	

		if (((x2 - x1) * xdirflag) > 0)
		{
			while (y < yend)
			{
				y++;
				if (d < 0)
				{
					d += incr1;
				}
				else
				{
					x++;
					d += incr2;
				}
				if(*ptt++)
					::HLine(xsurface,x - (wid >>1),y,x - (wid >>1)+wid,color);				
			}
		}
		else
		{
			while (y < yend)
			{
				y++;
				if (d < 0)
				{
					d += incr1;
				}
				else
				{
					x--;
					d += incr2;
				}
				if(*ptt++)
					::HLine(xsurface,x - (wid >>1),y,x - (wid >>1)+wid,color);				
			}
		}
	}
	if(!pPattern)	{
		delete []pattern;
		return NULL;
	}
	return ptt;
}

char* WideLinePatternEx (PXSurface xsurface, int x1, int y1, int x2, int y2,int width, Pixel color, char* pPattern, const int *fp,const int np, uint8_t alpha)
{
	if (width==0)
		return NULL;
	if(!xsurface||!ClipRectEx(x1,y1,x2,y2,0,0,xsurface->width,xsurface->height))
	{
		return NULL;
	}
	if(np<2)
	{
		WideLineEx(xsurface,x1,y1,x2,y2,color,width, alpha, true);
		return NULL;
	}
	if(width<2)
		return linePatternEx(xsurface,x1,y1,x2,y2,color, pPattern, fp,np, alpha);
	
	int dx, dy, incr1, incr2, d, x, y, xend, yend, xdirflag, ydirflag;
	int wid;
	dx = abs (x2 - x1);
	dy = abs (y2 - y1);	

	char *pattern = NULL;
	char *ptt = NULL;
	if(!pPattern)	{
		int size=abs(dx)+abs(dy);
		int npp=CalculatePatternLength(fp,np);
		size=(1+size/npp)*npp;
		pattern=new char[size];	
		MakeMarkPattern(pattern,fp,np);	
		char *lastpattern=pattern+size;
		ptt=pattern+npp;
		for(;ptt<lastpattern;ptt+=npp)
		{
			memcpy(ptt,pattern,npp);
		}
		ptt=pattern;
	}
	else
		ptt = pPattern;
	if (dy <= dx)
	{
		/* More-or-less horizontal. use wid for vertical stroke */
		/* Doug Claar: watch out for NaN in atan2 (2.0.5) */
		if ((dx == 0) && (dy == 0))
		{
			wid = 1;
		}
		else
		{
			double ac = cos (atan2 ((double)dy, (double)dx));
			if (ac != 0)
			{
				wid = width / ac;
			}
			else
			{
				wid = 1;
			}
			if (wid == 0)
			{
				wid = 1;
			}
		}
		/* 2.0.12: Michael Schwartz: divide rather than multiply;
		TBB: but watch out for /0! */
		//wid=RoundDevInt(julery_isqrt((thick*thick*(dx*dx+dy*dy))),dx);
		if (wid == 0)
		{
			wid = 1;
		}
		d = (dy<<1) - dx;
		incr1 = dy<<1;
		incr2 = (dy - dx)<<1;
		if (x1 > x2)
		{
			x = x2;
			y = y2;
			ydirflag = (-1);
			xend = x1;
		}
		else
		{
			x = x1;
			y = y1;
			ydirflag = 1;
			xend = x2;
		}

		/* Set up line thickness */
		if(*ptt++)
			::VLine(xsurface,x,y - (wid >>1),y - (wid >>1)+wid,color,alpha);

		if (((y2 - y1) * ydirflag) > 0)
		{
			while (x < xend)
			{
				x++;
				if (d < 0)
				{
					d += incr1;
				}
				else
				{
					y++;
					d += incr2;
				}
				if(*ptt++)
					::VLine(xsurface,x,y - (wid >>1),y - (wid >>1)+wid,color,alpha);
			}
		}
		else
		{
			while (x < xend)
			{
				x++;
				if (d < 0)
				{
					d += incr1;
				}
				else
				{
					y--;
					d += incr2;
				}
				if(*ptt++)
					::VLine(xsurface,x,y - (wid >>1),y - (wid >>1)+wid,color,alpha);
			}
		}
	}
	else
	{
		/* More-or-less vertical. use wid for horizontal stroke */
		/* 2.0.12: Michael Schwartz: divide rather than multiply;
		TBB: but watch out for /0! */
		 double as = sin (atan2 ((double)dy, (double)dx));
		 if (as != 0)
		{
		  wid = width / as;
		}
		  else
		{
		  wid = 1;
		}
		if (wid == 0)
			wid = 1;

		d = (dx<<1) - dy;
		incr1 = dx<<1;
		incr2 = (dx - dy)<<1;
		if (y1 > y2)
		{
			y = y2;
			x = x2;
			yend = y1;
			xdirflag = (-1);
		}
		else
		{
			y = y1;
			x = x1;
			yend = y2;
			xdirflag = 1;
		}

		/* Set up line thickness */
		if(*ptt++)
			::HLine(xsurface,x - (wid >>1),y,x - (wid >>1)+wid,color);	

		if (((x2 - x1) * xdirflag) > 0)
		{
			while (y < yend)
			{
				y++;
				if (d < 0)
				{
					d += incr1;
				}
				else
				{
					x++;
					d += incr2;
				}
				if(*ptt++)
					::HLine(xsurface,x - (wid >>1),y,x - (wid >>1)+wid,color,alpha);				
			}
		}
		else
		{
			while (y < yend)
			{
				y++;
				if (d < 0)
				{
					d += incr1;
				}
				else
				{
					x--;
					d += incr2;
				}
				if(*ptt++)
					::HLine(xsurface,x - (wid >>1),y,x - (wid >>1)+wid,color,alpha);				
			}
		}
	}
	if(!pPattern)	{
		delete []pattern;
		return NULL;
	}
	return ptt;
}

void EllipsePattern( PXSurface xsurface, int x0,int y0, int rx,int ry, Pixel color,int iWidth,const int *fp,int np )
{
	if (iWidth==0)
		return;
	if(!xsurface)
		return;
	if(np<2)
	{
		Ellipse(xsurface,x0,y0,rx,ry,color,1);
		return;
	}
	void (*EllipsePixelFunc)(PXSurface ,int ,int ,int ,int ,Pixel,int )=EllipseDefaultPixelFunc;
	if(iWidth>1)
	{
		EllipsePixelFunc=EllipseWidePixelFunc;
	}
	/*long t1 = rx*rx;
	long t2 = t1<<1;
	long t3 = t2<<1;
	long t4 = ry*ry;
	long t5 = t4<<1;
	long t6 = t5<<1;
	long t7 = rx*t5;
	long t8 = t7<<1;
	long t9 = 0L;
	long d1 = t2 - t7 + (t4>>1);	// error terms 
	long d2 = (t1>>1) - t8 + t5;*/

	__int64 t1 = rx*rx;
	__int64 t2 = t1<<1;
	__int64 t3 = t2<<1;
	__int64 t4 = ry*ry;
	//check large ellipse
	POINT pt1, pt2; pt1.x = -1; pt2.x = -1;
	if(rx > 32000 || ry > 32000)	{
		Algorithm Eclipse;
		const POINT *pECLipseR;
		Eclipse.SetEclipse(x0, y0, rx, ry);
		Eclipse.SetRectangle(xsurface->width, xsurface->height);
		pECLipseR = Eclipse.EclipseIntersection();
		if(pECLipseR[1].x != -1)	
			WideLinePattern(xsurface, pECLipseR[0].x, pECLipseR[0].y, pECLipseR[1].x, pECLipseR[1].y, iWidth, color, NULL, fp, np);
		return;		
	}
	__int64 t5 = t4<<1;
	__int64 t6 = t5<<1;
	__int64 t7 = rx*t5;
	__int64 t8 = t7<<1;
	__int64 t9 = 0L;
	__int64 d1 = t2 - t7 + (t4>>1);	/* error terms */
	__int64 d2 = (t1>>1) - t8 + t5;
	//COLORREF color=RGB(255,0,0);	
	int npp=CalculatePatternLength(fp,np);	
	char *pattern=new char[npp];	
	MakeMarkPattern(pattern,fp,np);	
	char *lastpt=pattern+npp;
	char *ptt=pattern + (fp[0]>>1);

	int x = rx, y = 0;	/* ellipse points */

	while (d2 < 0)			// til slope = -1 
	{		
		if(*ptt)
		{
			EllipsePixelFunc(xsurface,x0,y0,x,y,color,iWidth);
		}
		ptt+=iWidth;
		if(ptt>=lastpt)
			ptt=pattern;
		y++;		// always move up here 
		t9 += t3;	
		if (d1 < 0)	// move straight up 
		{
			d1 += t9 + t2;
			d2 += t9;
		}
		else		// move up and left 
		{
			x--;
			t8 -= t6;
			d1 += t9 + t2 - t8;
			d2 += t9 + t5 - t8;
		}
	}

	//ptt=pattern + (npp>>1);
	do 				// rest of top right quadrant 
	{
		if(*ptt)
		{
			EllipsePixelFunc(xsurface,x0,y0,x,y,color,iWidth);
		}
		ptt+=iWidth;
		if(ptt>=lastpt)
			ptt=pattern;	
		x--;		// always move left here 
		t8 -= t6;	
		if (d2 < 0)	// move up and left 
		{
			y++;
			t9 += t3;
			d2 += t9 + t5 - t8;
		}
		else		// move straight left 
			d2 += t5 - t8;
	} while (x>=0);
	delete []pattern;
}

void EllipsePatternEx( PXSurface xsurface, int x0,int y0, int rx,int ry, Pixel color,int iWidth,const int *fp,int np , uint8_t alpha)
{
	if (iWidth==0)
		return;
	if(!xsurface)
		return;
	if(np<2)
	{
		EllipseEx(xsurface,x0,y0,rx,ry,color,alpha, 1);
		return;
	}
	void (*EllipsePixelFuncEx)(PXSurface ,int ,int ,int ,int ,Pixel,int,uint8_t)=EllipseDefaultPixelFuncEx;
	if(iWidth>1)
	{
		EllipsePixelFuncEx=EllipseWidePixelFuncEx;
	}
	/*long t1 = rx*rx;
	long t2 = t1<<1;
	long t3 = t2<<1;
	long t4 = ry*ry;
	long t5 = t4<<1;
	long t6 = t5<<1;
	long t7 = rx*t5;
	long t8 = t7<<1;
	long t9 = 0L;
	long d1 = t2 - t7 + (t4>>1);	// error terms 
	long d2 = (t1>>1) - t8 + t5;*/

	__int64 t1 = rx*rx;
	__int64 t2 = t1<<1;
	__int64 t3 = t2<<1;
	__int64 t4 = ry*ry;
	//check large ellipse
	POINT pt1, pt2; pt1.x = -1; pt2.x = -1;
	if(rx > 32000 || ry > 32000)	{
		Algorithm Eclipse;
		const POINT *pECLipseR;
		Eclipse.SetEclipse(x0, y0, rx, ry);
		Eclipse.SetRectangle(xsurface->width, xsurface->height);
		pECLipseR = Eclipse.EclipseIntersection();
		if(pECLipseR[1].x != -1)	
			WideLinePatternEx(xsurface, pECLipseR[0].x, pECLipseR[0].y, pECLipseR[1].x, pECLipseR[1].y, iWidth, color, NULL, fp, np,alpha);
		return;		
	}
	__int64 t5 = t4<<1;
	__int64 t6 = t5<<1;
	__int64 t7 = rx*t5;
	__int64 t8 = t7<<1;
	__int64 t9 = 0L;
	__int64 d1 = t2 - t7 + (t4>>1);	/* error terms */
	__int64 d2 = (t1>>1) - t8 + t5;
	//COLORREF color=RGB(255,0,0);	
	int npp=CalculatePatternLength(fp,np);	
	char *pattern=new char[npp];	
	MakeMarkPattern(pattern,fp,np);	
	char *lastpt=pattern+npp;
	char *ptt=pattern + (fp[0]>>1);

	int x = rx, y = 0;	/* ellipse points */

	while (d2 < 0)			// til slope = -1 
	{		
		if(*ptt)
		{
			EllipsePixelFuncEx(xsurface,x0,y0,x,y,color,iWidth,alpha);
		}
		ptt+=iWidth;
		if(ptt>=lastpt)
			ptt=pattern;
		y++;		// always move up here 
		t9 += t3;	
		if (d1 < 0)	// move straight up 
		{
			d1 += t9 + t2;
			d2 += t9;
		}
		else		// move up and left 
		{
			x--;
			t8 -= t6;
			d1 += t9 + t2 - t8;
			d2 += t9 + t5 - t8;
		}
	}

	//ptt=pattern + (npp>>1);
	do 				// rest of top right quadrant 
	{
		if(*ptt)
		{
			EllipsePixelFuncEx(xsurface,x0,y0,x,y,color,iWidth,alpha);
		}
		ptt+=iWidth;
		if(ptt>=lastpt)
			ptt=pattern;	
		x--;		// always move left here 
		t8 -= t6;	
		if (d2 < 0)	// move up and left 
		{
			y++;
			t9 += t3;
			d2 += t9 + t5 - t8;
		}
		else		// move straight left 
			d2 += t5 - t8;
	} while (x>=0);
	delete []pattern;
}

void FillPolyPolyEdges( int width,int height,POINT *P, int vertex_cnt[],int nvertex)
{
    int i, n;
	edge **tmpedge=edgeTable;	
	POINT *pts;
	for(int iv=0;iv<nvertex-1;iv++)
	{
		n=vertex_cnt[iv+1]-vertex_cnt[iv];
		n=(int)fclip(P+vertex_cnt[iv],n,0,0,(float)width,(float)height,gPoint, true);
		if(n>2)
		{
			pts=(POINT*)&gPoint[0];
		}
		else
		{
			continue;
		}		
		POINT *p1, *p2, *p3,*p;
		p=pts;
		POINT *pend=p+n-1;
		p1=p;
		for (i = n; i; p1++,i--)
		{
			//Point *p1, *p2, *p3;
			struct edge *e;
			//p1 = &p[i];
			p2=p1+1;
			if(p2>pend)
			{
				p2=p;
			}
			//p2 = &p[(i + 1) % n];
			if (p1->y == p2->y)
				continue;   /* Skip horiz. edges */
			/* Find next vertex not level with p2 */
			for(p3=p1+2;;p3++)
			{
				if(p3>pend)
					p3=p;
				if(p3->y!=p2->y)
				{
					break;
				}
			}
			e = new edge;
			e->xNowNumStep = abs(p1->x - p2->x);
			if (p2->y > p1->y)
			{
				e->yTop = p1->y;
				e->yBot = p2->y;
				e->xNowWhole = p1->x;
				if (p2->x < p1->x)
					e->xNowDir = -1;
				else if (p2->x > p1->x)
					e->xNowDir = 1;
				else e->xNowDir = 0;
	//          e->xNowDir = SGN(p2->x - p1->x);
				e->xNowDen = e->yBot - e->yTop;
				e->xNowNum = (e->xNowDen >> 1);
				if (p3->y > p2->y)
					e->yBot--;
			}
			else
			{
				e->yTop = p2->y;
				e->yBot = p1->y;
				e->xNowWhole = p2->x;
				if (p2->x > p1->x)
					e->xNowDir = -1;
				else if (p2->x < p1->x)
					e->xNowDir = 1;
				else e->xNowDir = 0;
	//          e->xNowDir = SGN(p1->x - p2->x);
				e->xNowDen = e->yBot - e->yTop;
				e->xNowNum = (e->xNowDen >> 1);
				if (p3->y < p2->y)
				{
					e->yTop++;
					e->xNowNum += e->xNowNumStep;
					while (e->xNowNum >= e->xNowDen)
					{
						e->xNowWhole += e->xNowDir;
						e->xNowNum -= e->xNowDen;
					}
				}
			}
			e->next = edgeTable[e->yTop];
			//////////////////////////////////////////////////////////////////////////
			vt_New.push_back((DWORD)e);
			//////////////////////////////////////////////////////////////////////////
			edgeTable[e->yTop] = e;
		}
	}
}

void FillPolyPoly(PXSurface xsurface, POINT *vertex_vector, int *vertex_cnt,int nvertex, Pixel color,int fp,uint8_t alpha )
{	
	SetPatternFunc(xsurface,fp);
	if ( (alpha | 0x03) != 0xFF )
	{
		gAlPha=alpha;
		xsurface->PixelFunc=AlphaPixelFunc;				
	}	
	struct edge *active;
    long curY;
	const long _Maxy=xsurface->height;	
	memset(edgeTable, 0, sizeof(edgeTable));
	FillPolyPolyEdges(xsurface->width,xsurface->height,vertex_vector, vertex_cnt,nvertex);

	for (curY = 0; edgeTable[curY] == NULL; ++curY)	
        if (curY == _Maxy)
            return;     /* No edges in polygon */

	for (active = NULL;curY<_Maxy; curY++)
	{
		active = UpdateActive(active, curY);
		if(active != NULL)
			DrawRuns(xsurface,active, curY, color);		
	}
	
	xsurface->PixelFunc=DefaultPixelFunc;		
	xsurface->Pattern=FT_SOLID_CHECK;
	FreeEdges();
}

bool SaveXSurface( const PXSurface surface, const wchar_t* filename )
{
    if (!surface || !filename)
        return false;    

    // Lock the surface    

    // Lines must be dword aligned
   // assert( ((surface->GetWidth() * sizeof(Pixel))&3)==0 );
    
    BITMAPINFOHEADER info;
    info.biSize          = sizeof(BITMAPINFOHEADER);
    info.biWidth         = surface->GetWidth();
    info.biHeight        = surface->GetHeight();
    info.biPlanes        = 1;
    info.biBitCount      = 16;
    info.biCompression   = BI_BITFIELDS;
    info.biSizeImage     = surface->pitch/*info.biWidth*/ * info.biHeight;
    info.biXPelsPerMeter = 0;
    info.biYPelsPerMeter = 0;
    info.biClrUsed       = 0;
    info.biClrImportant  = 0; 
    
    uint32_t colors[3] = { RED_MASK, GREEN_MASK, BLUE_MASK };
    
    
    BITMAPFILEHEADER header;
    header.bfType      = 0x4d42;
    header.bfSize      = sizeof(BITMAPFILEHEADER) + info.biSize + sizeof(colors) + info.biSizeImage;
    header.bfReserved1 = 0;
    header.bfReserved2 = 0;
    header.bfOffBits   = sizeof(BITMAPFILEHEADER) + info.biSize + sizeof(colors);
#ifdef OS_WIN
	// Create the file
	HANDLE hFile = ::CreateFileW( filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0 );
	if (hFile == INVALID_HANDLE_VALUE)
	{        
		return false;
	}

	DWORD temp;

	// Write the header + bitmap info
	::WriteFile( hFile, &header, sizeof(header), &temp, 0 );
	::WriteFile( hFile, &info, sizeof(info), &temp, 0 );
	::WriteFile( hFile, colors, sizeof(colors), &temp, 0 );

	// Write the image (must flip image vertically)
	const Pixel* pixels = (Pixel*)surface->pixel;
	pixels = (Pixel*)((uint8_t*)pixels + surface->pitch * (surface->GetHeight()-1));

	for (int h = surface->GetHeight(); h; --h)
	{
		::WriteFile( hFile, pixels, surface->pitch/*surface->GetWidth() * sizeof(Pixel)*/, &temp, 0 );
		pixels = (Pixel*)((uint8_t*)pixels - surface->pitch);
	}   

	::CloseHandle( hFile );
#else
	char szTmpFile[512];
	const wchar_t* pChW = filename;
	char* pChA = szTmpFile;
	while (*pChW)
	{
		*pChA = *pChW;
		pChA++; pChW++;
	}
	*pChA = 0;
	FILE* f = fopen(szTmpFile, "wb");
	if (f==NULL)
		return false;
	
	// Write the header + bitmap info
	fwrite(&header, sizeof(header), 1, f);
	fwrite(&info, sizeof(info), 1, f);
	fwrite(colors, sizeof(colors), 1, f);

	// Write the image (must flip image vertically)
	const Pixel* pixels = (Pixel*)surface->pixel;
	pixels = (Pixel*)((uint8_t*)pixels + surface->pitch * (surface->GetHeight()-1));

	for (int h = surface->GetHeight(); h; --h)
	{
		fwrite(pixels, surface->pitch, 1, 0);
		pixels = (Pixel*)((uint8_t*)pixels - surface->pitch);
	}   
	fclose(f);
#endif // OS_WIN
    
    return true;
}

void FillPolyInside(PXSurface xsurface, POINT *pData, int iCount, Pixel colorLine, Pixel colorFill, const int& nInsideHatchWidth, const int& nCellSize)
{
	Algorithm alg;
	const POINT *pResult;
	alg.SetSize(iCount);

	pResult = (POINT *)alg.SimilarityPolygon(pData, nInsideHatchWidth);
	if(!pResult)	
		return;

	gPattSz = nCellSize;

	vector<POINT> pts;
	int iVertexCount[3];
	iVertexCount[0] = 0;
	iVertexCount[1] = iCount;

	size_t i;
	for(i=0;i<iCount;i++)	{
		pts.push_back(*pData++);
	}

	POINT ptCur; ptCur.x = INT_MAX;
	for(i=0;i<iCount;i++)	{
		if(ptCur.x != (*(pResult+i)).x || ptCur.y != (*(pResult+i)).y)	{
			ptCur = *(pResult+i);
			pts.push_back(ptCur);
		}
	}

	iVertexCount[2] = pts.size();

	FillPolyPoly(xsurface, &pts[0], iVertexCount, 3, colorFill, 10, 255);//chess board
}

void AACircle2(PXSurface xsurface,int x0,int y0,int radius,Pixel color)
{
	if(!xsurface)
		return;
	long w=xsurface->width-1;
	long h=xsurface->height-1;		
	long r=radius>>1;
	long rMod = (radius%2) ? r+1 : r;
	if(r==0)
	{		
		if(radius)
		{
			SetPixel(xsurface,x0,y0,color);
			SetPixel(xsurface,x0+1,y0,color);
			SetPixel(xsurface,x0,y0+1,color);
			SetPixel(xsurface,x0+1,y0+1,color);
		}
		else
		{
			SetPixel(xsurface,x0,y0,color);
		}
		return;
	}
	if((x0+r)>=0&&(x0-r)<=w&&(y0+r)>=0&&(y0-r)<=h)
	{
		long yinc=xsurface->pitch>>1;
		Pixel *pixels=(Pixel *)xsurface->pixel;
		int x = r;
		int y = 0;	
		long T = 0;
		const long I = long(1.3*double(1<<8)); 		
		/*SetPixel(xsurface, x0-rMod, y0, color);
		SetPixel(xsurface, x0+rMod, y0, color);
		SetPixel(xsurface, x0, y0-rMod, color);
		SetPixel(xsurface, x0, y0+rMod, color);	*/
		SetPixel(xsurface,x0-r,y0,color);
		SetPixel(xsurface, x0+r, y0, color);
		SetPixel(xsurface,x0,y0-r,color);
		SetPixel(xsurface, x0, y0+r, color);
		long R2=radius*radius;      
		int iShift = (r > 255) ? 12 : 14;
		R2<<=iShift;
		uint8_t dr;
		for(;x>y;)
		{	  
			R2-=((y<<(iShift+3))+(1<<(iShift+2)));
			++y;	  	   
			x=julery_isqrt(R2);
			dr=(~x)&0xff;	   
			x>>=((iShift>>1) + 1);
			if(x<r)
			{
				++x;	
			}			
			Pixel8Sym(pixels,x0,y0, x,y,(I*((dr)&0xff))>>8, color,yinc,w,h);		
			if(rMod > r)	{
				if(x > rMod - 2 && y <= (rMod>>3) + 1)
					Pixel8Sym(pixels, x0, y0, x,y,(I*((~dr)&0xff))>>8, color,yinc,w,h);	
				else
					Pixel8Sym(pixels, x0, y0, x-1,y,(I*((~dr)&0xff))>>8, color,yinc,w,h);	
			}
			else
				Pixel8Sym(pixels,x0,y0, x-1,y,(I*((~dr)&0xff))>>8, color,yinc,w,h);	
			T = dr;
		}
	}
}

