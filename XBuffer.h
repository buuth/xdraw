//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the MapSDK Drawing Engine
// Copyright (C) 17 Degree Inc http://www.mapsdk.com
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
#pragma once
#include "stdafx.h"

#ifdef __ANDROID__
#include "XDraw.h"
#endif //#ifdef __ANDROID__

PXSurface CreateXSurface(uint16_t ,uint16_t );
void CloseXSurface(PXSurface );
void AttachToSurface(PXSurface pSurface, void* pPixel, int nWidth, int nHeight);
void DetachFromSurface(PXSurface pSurface);
uint16_t GetXSurfaceWidth(PXSurface);
uint16_t GetXSurfaceHeight(PXSurface);
uint16_t GetXSurfacePitch(PXSurface);
void* GetXSurfacebuffer(PXSurface);
void* GetXSurfaceEndBuffer(PXSurface);
void ClearXSurface(PXSurface ,const Pixel &);
void HLine(PXSurface ,int ,int ,int ,Pixel);
void HLine(PXSurface ,int ,int ,int ,Pixel,uint8_t);
void VLine(PXSurface ,int ,int ,int ,Pixel);
void VLine(PXSurface ,int ,int ,int ,Pixel,uint8_t);
void Line(PXSurface ,int ,int ,int ,int ,Pixel,bool);
void LineEx(PXSurface ,int ,int ,int ,int ,Pixel, uint8_t, bool);
void WideLine(PXSurface ,int ,int ,int ,int ,Pixel ,int ,bool );
void WideLineEx(PXSurface ,int ,int ,int ,int ,Pixel ,int ,uint8_t ,bool );
void WideOutline(PXSurface ,int ,int ,int ,int ,Pixel ,int );
void WideOutlineEx(PXSurface ,int ,int ,int ,int ,Pixel ,int , uint8_t);
void WideInline(PXSurface ,int ,int ,int ,int ,Pixel ,int ,Pixel);
void WideInlineEx(PXSurface ,int ,int ,int ,int ,Pixel ,int ,Pixel,uint8_t);
void FillPoly(PXSurface , POINT *, int , Pixel ,int,bool,uint8_t );
void FillPolyEx(PXSurface , POINT *, int , Pixel);
void FillRect(PXSurface ,int ,int ,int ,int ,Pixel ,uint8_t );
void DrawRect(PXSurface ,const RECT &,Pixel ,int ,uint8_t );
void SetPixel(PXSurface ,int ,int ,Pixel ,uint8_t );
void SetPixel(PXSurface ,int ,int ,Pixel );
void BlitSurface(PXSurface ,int , int , const PXSurface , const RECT* ,uint8_t , unsigned long );
void BlitSurfaceMask(PXSurface, int, int, const PXSurface, uint8_t*, const RECT*);
void GradientSurfaceH(PXSurface ,int , int , const PXSurface , uint8_t , uint8_t, const RECT*);
#ifdef OS_WIN
PXSurface CreateXSurface(HBITMAP );
#endif // OS_WIN
void AACircle(PXSurface ,int ,int ,int ,Pixel );
void AACircleEx(PXSurface ,int ,int ,int ,Pixel ,uint8_t);
void FillAACircle(PXSurface ,int ,int ,int ,Pixel );
void FillAACircleEx(PXSurface ,int ,int ,int ,Pixel ,uint8_t);
void AAPolyOutLine( PXSurface ,POINT *, int , int ,Pixel );
void AAPolyOutLineEx( PXSurface ,POINT *, int , int ,Pixel ,uint8_t);
void AAPolyInLine( PXSurface ,POINT *, int , int ,Pixel ,Pixel );
void AAPolyInLineEx( PXSurface ,POINT *, int , int ,Pixel ,Pixel ,uint8_t);
void BlitStretch( PXSurface ,const RECT& , const PXSurface , const RECT* ,int ,int );
#ifdef OS_WIN
HDC CreateDCXSurface(PXSurface);
void SetXSurfaceDC(HDC ,PXSurface );
#endif // OS_WIN
PXSurface CloneXSurface(PXSurface );
void BlitFast(PXSurface ,const PXSurface );
void DrawPoly(PXSurface ,POINT *,int ,Pixel ,bool ,bool );
void DrawPolyEx(PXSurface, POINT *, int, Pixel, Pixel);
unsigned julery_isqrt(unsigned long ) ;
void LineFocus(PXSurface ,int ,int ,int ,int );
void LineSnap(PXSurface , const POINT& , int );
void Tracker(PXSurface ,int ,int ,int ,int );
void Ellipse( PXSurface , int ,int , int ,int , Pixel,int);
void EllipseEx( PXSurface , int ,int , int ,int , Pixel,uint8_t,int);
void EllipseFocus( PXSurface , int ,int , int ,int );
void FillRect(PXSurface ,int ,int ,int ,int ,Pixel , int );
void FillRectEx(PXSurface ,int ,int ,int ,int ,Pixel , int ,uint8_t);
void FillEllipse( PXSurface ,int ,int , int , int , Pixel ,int  );
void FillEllipseEx( PXSurface ,int ,int , int , int , Pixel ,int ,uint8_t);
char* linePattern(PXSurface ,int , int , int , int , Pixel , char*, const int*,int );
char* linePatternEx(PXSurface ,int , int , int , int , Pixel , char*, const int*,int ,uint8_t);
char* WideLinePattern (PXSurface , int , int , int , int ,int , Pixel, char*, const int* ,const int );
void EllipsePattern( PXSurface , int ,int , int ,int , Pixel ,int ,const int *,const int );
void EllipsePatternEx( PXSurface , int ,int , int ,int , Pixel ,int ,const int *,const int, uint8_t alpha);
char* WideLinePatternEx (PXSurface , int , int , int , int ,int , Pixel, char*, const int* ,const int ,uint8_t alpha);
void FillPolyPoly(PXSurface , POINT *, int*,int , Pixel ,int ,uint8_t  );
bool SaveXSurface( const PXSurface , const wchar_t*  );
void FillPolyInside(PXSurface xsurface, POINT *pData, int iCount, Pixel colorLine, Pixel colorFill, const int& nInsideHatchWidth, const int& nCellSize);

int CalculatePatternLength(const int* fp,int np);
void MakeMarkPattern(char *mark_patten,const int* fp,int np);
