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
#pragma once

#if (defined(_WIN32) || defined(WIN32) || defined(UNDER_CE))
#define		OS_WIN			1
#elif (defined(__ANDROID__))
#define		OS_ANDROID		1
#endif


#ifdef OS_WIN
#include "windows.h"
#elif (defined(OS_ANDROID))
#include "android_types.h"
#endif

#ifndef SIZE_TRACKER
#define SIZE_TRACKER 3
#endif
const int   RED_SHIFT   = 11;
const int   GREEN_SHIFT = 5;
const int   BLUE_SHIFT  = 0;

typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;
typedef uint16_t Pixel;
typedef unsigned __int32 uint32_t;
const Pixel RED_MASK    = 0x1F << 11;
const Pixel GREEN_MASK  = 0x3F << 5;
const Pixel BLUE_MASK   = 0x1F;
const Pixel RB_MASK= RED_MASK|BLUE_MASK;
inline Pixel Color(const int& red, const int& green, const int& blue)
{
   return ((red << 8) & RED_MASK) | ((green << 3) & GREEN_MASK) | (blue >> 3);
}
inline Pixel Color( const COLORREF& color )
{
    return Color( GetRValue(color), GetGValue(color), GetBValue(color) );
}

typedef struct XSurface
{	
	uint16_t width;
	uint16_t height;
	uint16_t pitch;
#ifdef OS_WIN
	HBITMAP hbmp;
#endif // OS_WIN
	uint8_t *pixel;
	uint8_t *lastpixel;
	void (*PixelFunc)(const Pixel&,Pixel &);	
	bool (*Pattern)(int,int );
	uint16_t GetWidth()	const {
		return width;
	}
	uint16_t GetHeight() const {
		return height;
	}
#ifdef OS_WIN
	HBITMAP GetBitmap()	const {
		return hbmp;
	}
#endif // OS_WIN
	XSurface()
	{
		width = height = 0;
		pitch = 0;
#ifdef OS_WIN
		hbmp = NULL;
#endif // OS_WIN
		pixel = NULL;
		lastpixel = NULL;
		PixelFunc = NULL;
		Pattern = NULL;
	}
	~XSurface()
	{
#ifdef OS_WIN
		if (hbmp)	
			::DeleteObject(hbmp);
#else
		if (pixel)
			delete [] pixel;
#endif // OS_WIN
	}
} *PXSurface;