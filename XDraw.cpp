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
#include "StdAfx.h"
#include "XDraw.h"
#include "XBuffer.h"

#include "XDrawRoad.h"
#include "MyClip.h"

//draw line bitmap patterns
//#define USE_PATTERN

#ifdef USE_PATTERN
#include <vector>
using namespace std;
#include "AggUtil.h"
#include "XInterface.cpp"
#else
#define _GetRValue(pixel)      ((pixel & RED_MASK)>>8)
#define _GetGValue(pixel)      ((pixel & GREEN_MASK)>>3)
#define _GetBValue(pixel)      ((pixel & BLUE_MASK)<<3)
#endif//#ifdef USE_PATTERN

#ifdef USING_AGG_FOR_MAP
#undef USING_AGG_FOR_MAP
// #define		USING_AGG_FOR_ROAD
#endif//#ifdef USING_AGG_FOR_MAP

CXDraw::CXDraw()
{
	m_pXSurface = NULL;
#ifdef OS_WIN
	m_hdc = NULL;
#endif // OS_WIN
	m_pPolygon = NULL;
}
CXDraw::CXDraw(int w, int h)
{		
	m_pXSurface = ::CreateXSurface(w,h);

#ifdef OS_WIN
	m_hdc = CreateSurfaceDC(m_pXSurface);
#endif // OS_WIN

	m_pPolygon = NULL;
}
CXDraw::~CXDraw(void)
{
#ifdef OS_WIN
	// attention! m_hdc must be destroyed before m_pXSurface!
	if (m_hdc)
	{
		::DeleteDC(m_hdc);
		m_hdc = NULL;
	}
#endif // OS_WIN
	
	::CloseXSurface(m_pXSurface); m_pXSurface = NULL;
	CloseRoadDraw();
#ifdef USE_PATTERN
	if (linePatterns.size()>0)
	{
		std::map<int, LinePattern*>::iterator it = linePatterns.begin();
		for(;it!=linePatterns.end();it++)	{
			if(it->second)
				delete it->second;
		}
		linePatterns.clear();
	}
	if(!fillPatterns.empty())	{
		std::map<int, PXSurface>::iterator it = fillPatterns.begin();
		for(;it!=fillPatterns.end();it++)	{
			if(it->second)
				::CloseXSurface(it->second);
		}
		fillPatterns.clear();
	}
#endif//#ifdef USE_PATTERN
}

bool CXDraw::Attach(void* pPixel, int nWidth, int nHeight)
{
	m_pXSurface = new XSurface;
	if (m_pXSurface)
	{
		AttachToSurface(m_pXSurface, pPixel, nWidth, nHeight);
		return true;
	}
	return false;
}

void CXDraw::Detach()
{
	if (m_pXSurface)
	{
		DetachFromSurface(m_pXSurface);
		delete m_pXSurface;
		m_pXSurface = NULL;
	}
}

#ifdef OS_WIN
void CXDraw::Blt(HDC hdc, const RECT & rect, int x , int y)
{
	::BitBlt(hdc,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,m_hdc,x,y,SRCCOPY);	
}
void CXDraw::Update(HDC hdc, RECT* pRec)
{
	int left, top, width, height;
	if(!pRec)	{
		left = 0;
		width = GetWidth(); 
#ifdef DEMO_VERSION
		top = GetHeight()>>4;
		height = GetHeight() - (top<<1);
#else
		top = 0;
		height = GetHeight();
#endif//#ifdef DEMO_VERSION	
	}
	else	{
		left = pRec->left;		
		width = pRec->right - pRec->left;
#ifdef DEMO_VERSION
		top = pRec->bottom>>4;
		height = pRec->bottom - pRec->top - (top<<1);
#else
		top = pRec->top;
		height = pRec->bottom - pRec->top;
#endif//#ifdef DEMO_VERSION		
	}
	::BitBlt(hdc, left, top, width, height, m_hdc, left, top, SRCCOPY);	

// 	if(!pRec)
// 		::BitBlt(hdc,0,0,GetWidth(),GetHeight(),m_hdc,0,0,SRCCOPY);	
// 	else	{
// 		//partial update
// 		::BitBlt(hdc,pRec->left,pRec->top,pRec->right-pRec->left,pRec->bottom-pRec->top,m_hdc,pRec->left,pRec->top,SRCCOPY);	
// 	}
}
#endif // OS_WIN

void CXDraw::Clear(const COLORREF& color)
{
	ClearXSurface(m_pXSurface,Color(color));
}
void CXDraw::DrawHLine(int x0,int y0,int x1,const COLORREF& color)
{
	HLine(m_pXSurface,x0,y0,x1,Color(color));
}
void CXDraw::DrawHLine(int x0,int y0,int x1,const COLORREF& color,uint8_t alpha)
{
	HLine(m_pXSurface,x0,y0,x1,Color(color),alpha);
}
void CXDraw::DrawVLine(int x0,int y0,int y1,const COLORREF& color)
{
	VLine(m_pXSurface,x0,y0,y1,Color(color));
}
void CXDraw::DrawVLine(int x0,int y0,int y1,const COLORREF& color,uint8_t alpha)
{
	VLine(m_pXSurface,x0,y0,y1,Color(color),alpha);
}
void CXDraw::DrawLine(int x0, int y0, int x1, int y1, const COLORREF& color, bool bAntialias)
{
//#ifdef USING_AGG_FOR_MAP
//	AGG_DrawLine(m_pXSurface, x0, y0, x1, y1, 1, color);
//#else
	Line(m_pXSurface, x0, y0, x1, y1, Color(color), bAntialias);
//#endif // USING_AGG_FOR_MAP
}
void CXDraw::DrawLineEx(int x0,int y0,int x1,int y1,const COLORREF& color,uint8_t alpha,bool bAntialias)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawLine(m_pXSurface, x0, y0, x1, y1, 1, color, alpha);
#else
	LineEx(m_pXSurface, x0, y0, x1, y1, Color(color), alpha, bAntialias);
#endif // USING_AGG_FOR_MAP
}
void CXDraw::DrawWideLine(int x0, int y0, int x1, int y1, const COLORREF& color, int w, bool bAntialias)
{
//#ifdef USING_AGG_FOR_MAP
//	AGG_DrawLine(m_pXSurface, x0, y0, x1, y1, w, color);
//#else
	WideLine(m_pXSurface, x0, y0, x1, y1, Color(color), w, bAntialias);
//#endif // USING_AGG_FOR_MAP
}
void CXDraw::DrawWideLineEx(int x0, int y0, int x1, int y1, const COLORREF& color, int w, uint8_t alpha, bool bAntialias)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawLine(m_pXSurface, x0, y0, x1, y1, w, color, alpha);
#else
	WideLineEx(m_pXSurface, x0, y0, x1, y1, Color(color), w, alpha, bAntialias);
#endif // USING_AGG_FOR_MAP
}

#ifdef OS_WIN
HDC CXDraw::GetDC()
{
	return m_hdc;
}
#endif // OS_WIN

void CXDraw::DrawWideOutline(int x0, int y0, int x1, int y1, const COLORREF& color, int wide)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawLineOutline(m_pXSurface, x0, y0, x1, y1, color, wide);
#else
	WideOutline(m_pXSurface, x0, y0, x1, y1, Color(color), wide);
#endif // USING_AGG_FOR_MAP
}
void CXDraw::DrawWideOutlineEx(int x0, int y0, int x1, int y1, const COLORREF& color, int wide, uint8_t alpha)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawLineOutline(m_pXSurface, x0, y0, x1, y1, color, wide, alpha);
#else
	WideOutlineEx(m_pXSurface, x0, y0, x1, y1, Color(color), wide, alpha);
#endif // USING_AGG_FOR_MAP
}
void CXDraw::DrawWideInline(int x0, int y0, int x1, int y1, int wide, const COLORREF& color, const COLORREF& outlinecolor)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawLineInline(m_pXSurface, x0, y0, x1, y1, wide, color);
#else
	WideInline(m_pXSurface,x0, y0, x1, y1, Color(color), wide, Color(outlinecolor));
#endif // USING_AGG_FOR_MAP
}
void CXDraw::DrawWideInlineEx(int x0, int y0, int x1, int y1, int wide, const COLORREF& color, const COLORREF& outlinecolor, uint8_t alpha)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawLineInline(m_pXSurface, x0, y0, x1, y1, wide, color, alpha);
#else
	WideInlineEx(m_pXSurface, x0, y0, x1, y1, Color(color), wide, Color(outlinecolor), alpha);
#endif // USING_AGG_FOR_MAP
}
void CXDraw::DrawWideInline(POINT* pts, int npt, int wide, const COLORREF& color, const COLORREF& outlinecolor)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawPolyInLine(m_pXSurface, pts, npt, wide, color);
#else
	for(int i=0; i<npt-1; i++)	{
		if(i > 0)
			FillAACircle(pts[i].x, pts[i].y, (wide-2), color);
		WideInline(m_pXSurface, pts[i].x, pts[i].y, pts[i+1].x, pts[i+1].y, Color(color), wide, Color(outlinecolor));
	}
#endif // USING_AGG_FOR_MAP
}
void CXDraw::DrawWideInlineEx(POINT* pts, int npt, int wide, const COLORREF& color, const COLORREF& outlinecolor, uint8_t alpha)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawPolyInLine(m_pXSurface, pts, npt, wide, color, alpha);
#else
	for(int i=0; i<npt-1; i++)	{
		if(i > 0)
			FillAACircleEx(pts[i].x, pts[i].y, (wide-2), color,alpha);
		WideInlineEx(m_pXSurface, pts[i].x, pts[i].y, pts[i+1].x, pts[i+1].y, Color(color), wide, Color(outlinecolor), alpha);
	}
#endif // USING_AGG_FOR_MAP
}
void CXDraw::FillRect(const int &x0,const int &y0,const int &x1,const int &y1,const COLORREF& color,uint8_t alpha)
{
	::FillRect(m_pXSurface,x0,y0,x1,y1,Color(color),alpha);
}
void CXDraw::FillRect(const RECT &rect,const COLORREF& color,uint8_t alpha)
{
	::FillRect(m_pXSurface,rect.left,rect.top,rect.right,rect.bottom,Color(color),alpha);
}
void CXDraw::DrawRect(const RECT &rect,const COLORREF& color, int width, uint8_t alpha)
{
	::DrawRect(m_pXSurface,rect,Color(color), width, alpha);
}
void CXDraw::SetPixel(int x,int y,const COLORREF& color,uint8_t alpha)
{
	::SetPixel(m_pXSurface,x,y,Color(color),alpha);
}
void CXDraw::SetPixel(int x,int y,const COLORREF& color)
{
	::SetPixel(m_pXSurface,x,y,Color(color));
}
void CXDraw::BlitSurface(int destX, int destY, const PXSurface source, const RECT* srcRect,unsigned long colormark,uint8_t alpha)
{
	::BlitSurface(m_pXSurface,destX,destY,source,srcRect,alpha,colormark);
}
void CXDraw::BlitSurfaceMask(int nDestX, int nDestY, const PXSurface pSrcSurf, uint8_t* pAlpha, const RECT* pSrcRect)
{
	::BlitSurfaceMask(m_pXSurface, nDestX, nDestY, pSrcSurf, pAlpha, pSrcRect);
}
void CXDraw::GradientSurfaceH(int destX, int destY, const PXSurface source, uint8_t alpha1, uint8_t alpha2, const RECT* srcRect)
{
	::GradientSurfaceH(m_pXSurface, destX, destY, source, alpha1, alpha2, srcRect);
}
void CXDraw::Blit(int destX, int destY, const PXSurface source, unsigned long colormark, uint8_t alpha)
{
	::BlitSurface(m_pXSurface,destX,destY,source, NULL, alpha, colormark);
}
PXSurface CXDraw::SetRenderSurface(PXSurface surface)
{	
	PXSurface oldsurface=m_pXSurface;
	m_pXSurface=surface;
#ifdef OS_WIN
	if (m_hdc)
		::SetXSurfaceDC(m_hdc,m_pXSurface);
	else
		m_hdc=CreateSurfaceDC(m_pXSurface);
#endif // OS_WIN
	return oldsurface;
}
void CXDraw::CloseSurface(PXSurface surface)
{
	::CloseXSurface(surface);
}
#ifdef OS_WIN
PXSurface CXDraw::CreateSurface(HBITMAP hbmp)
{	
	return ::CreateXSurface(hbmp);
}
#endif // OS_WIN
void AACircle2(PXSurface xsurface,int x0,int y0,int radius,Pixel color);
void CXDraw::DrawAACircle(const int& x, const int& y, const int& radius, const COLORREF& color)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawEllipse(m_pXSurface, x, y, radius>>1, radius>>1, 1, color);
#else
	AACircle2(m_pXSurface, x, y, radius, Color(color));
#endif // USING_AGG_FOR_MAP
}
void CXDraw::DrawAACircleEx(const int& x, const int& y, const int& radius, const COLORREF& color, uint8_t alpha)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawEllipse(m_pXSurface, x, y, radius>>1, radius>>1, 1, color, alpha);
#else
	AACircleEx(m_pXSurface, x, y, radius, Color(color), alpha);
#endif // USING_AGG_FOR_MAP
}
void CXDraw::FillAACircle(const int& x, const int& y, const int& radius, const COLORREF& color)
{
#ifdef USING_AGG_FOR_MAP
	AGG_FillEllipse(m_pXSurface, x, y, radius>>1, radius>>1, color);
#else
	::FillAACircle(m_pXSurface,x,y, radius,Color(color));
#endif // USING_AGG_FOR_MAP
}
void CXDraw::FillAACircleEx(const int& x, const int& y, const int& radius, const COLORREF& color, uint8_t alpha)
{
#ifdef USING_AGG_FOR_MAP
	AGG_FillEllipse(m_pXSurface, x, y, radius>>1, radius>>1, color, alpha);
#else
	::FillAACircleEx(m_pXSurface,x,y, radius,Color(color), alpha);
#endif // USING_AGG_FOR_MAP
}

void CXDraw::DrawAAPolyOutLine(POINT *points, int cnt, int w, const COLORREF& color)
{
#if (defined(USING_AGG_FOR_MAP) || defined(USING_AGG_FOR_ROAD))
	AGG_DrawPolyOutLine(m_pXSurface, points, cnt, w, color);
#else
	::AAPolyOutLine(m_pXSurface, points, cnt, w, Color(color));
#endif // USING_AGG_FOR_MAP 
}

///
void DrawAAPolyOL(CXDraw* display, POINT *points, int cnt, int w, const COLORREF& color)
{
#if (defined(USING_AGG_FOR_MAP) || defined(USING_AGG_FOR_ROAD))
	AGG_DrawPolyOL(display->GetRenderSurface(), points, cnt, w, color);
#else
	for(int i=0;i<cnt-1;i++)
		WideOutline(display->GetRenderSurface(), points[i].x, points[i].y, points[i+1].x, points[i+1].y, Color(color), w);
#endif//#if (defined(USING_AGG_FOR_MAP) || defined(USING_AGG_FOR_ROAD))
}
///

void CXDraw::DrawAAPolyOutLineEx(POINT *points, int cnt, int w, const COLORREF& color, uint8_t alpha)
{
#if (defined(USING_AGG_FOR_MAP) || defined(USING_AGG_FOR_ROAD))
	AGG_DrawPolyOutLine(m_pXSurface, points, cnt, w, color, alpha);
#else
	::AAPolyOutLineEx(m_pXSurface, points, cnt, w, Color(color), alpha);
#endif // USING_AGG_FOR_MAP
}

void CXDraw::DrawAAPolyInline(POINT *points, int cnt, int w, const COLORREF& color, const COLORREF& coloroutline)
{
#if (defined(USING_AGG_FOR_MAP) || defined(USING_AGG_FOR_ROAD))
	AGG_DrawPolyInLine(m_pXSurface, points, cnt, w, color);
#else
	::AAPolyInLine(m_pXSurface, points, cnt, w, Color(color), Color(coloroutline));
#endif // USE_AGG_TO_DRAWROAD	
}
void CXDraw::DrawAAPolyInlineEx(POINT *points, int cnt, int w, const COLORREF& color, const COLORREF& coloroutline, uint8_t alpha)
{
#if (defined(USING_AGG_FOR_MAP) || defined(USING_AGG_FOR_ROAD))
	AGG_DrawPolyInLine(m_pXSurface, points, cnt, w, color, alpha);
#else
	::AAPolyInLineEx(m_pXSurface, points, cnt, w, Color(color), Color(coloroutline), alpha);
#endif // USING_AGG_FOR_MAP
}
void CXDraw::BlitStretchSurface(const RECT& destRect, const PXSurface source, const RECT* srcRect_,int colormark, uint8_t alpha)
{
	::BlitStretch(m_pXSurface,destRect,source,srcRect_,alpha,colormark);
}
#ifdef OS_WIN
HDC CXDraw::CreateSurfaceDC(PXSurface xsurface)
{
	return ::CreateDCXSurface(xsurface);
}
#endif // OS_WIN
PXSurface CXDraw::CloneSurface(PXSurface xsurface)
{
	if (!xsurface)	
		xsurface = m_pXSurface;
	return ::CloneXSurface(xsurface);
}
PXSurface CXDraw::CreateSurface(int w, int h)
{
	return ::CreateXSurface(w,h);
}
int CXDraw::GetSurfaceWidth(PXSurface xsurface)
{
	return ::GetXSurfaceWidth(xsurface);
}

int CXDraw::GetSurfaceHeight(PXSurface xsurface)
{
	return ::GetXSurfaceHeight(xsurface);
}
int CXDraw::GetSurfacePitch(PXSurface xsurface)
{
	return ::GetXSurfacePitch(xsurface);
}
void* CXDraw::GetSurfaceBuffer(PXSurface xsurface)
{
	return GetXSurfacebuffer(xsurface);
}

void CXDraw::DrawPoly(POINT *pts, int ncnt, const COLORREF& color, bool bPolygon, bool bAntialias)
{
//#ifdef USING_AGG_FOR_MAP
//	AGG_DrawPoly(m_pXSurface, pts, ncnt, 1, color, bPolygon);
//#else
	POINT *curpt, *nextpt, *lastpt;
	lastpt = pts + ncnt;
	if (bPolygon)
	{
		curpt = lastpt-1;
		nextpt = pts;
	}
	else
	{
		curpt = pts;
		nextpt = pts+1;
	}
	for (;nextpt<lastpt; curpt=nextpt++)
	{
		this->DrawLine(curpt->x, curpt->y, nextpt->x, nextpt->y, color, bAntialias);
	}
//#endif // USING_AGG_FOR_MAP
}

void CXDraw::DrawRoadPoly(POINT *pts, int ncnt, const COLORREF& color, bool bPolygon, bool bAntialias)
{
#if (defined(USING_AGG_FOR_MAP) || defined(USING_AGG_FOR_ROAD))
	AGG_DrawPoly(m_pXSurface, pts, ncnt, 1, color, bPolygon);
#else
	POINT *curpt, *nextpt, *lastpt;
	lastpt = pts + ncnt;
	if (bPolygon)
	{
		curpt = lastpt-1;
		nextpt = pts;
	}
	else
	{
		curpt = pts;
		nextpt = pts+1;
	}
	for (;nextpt<lastpt; curpt=nextpt++)
	{
		this->DrawLine(curpt->x, curpt->y, nextpt->x, nextpt->y, color, bAntialias);
	}
#endif // USING_AGG_FOR_MAP, USING_AGG_FOR_ROAD
}

void CXDraw::DrawPolyEx(POINT *pts, int ncnt, const COLORREF& color, uint8_t alpha, bool bPolygon, bool bAntialias)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawPoly(m_pXSurface, pts, ncnt, 1, color, bPolygon, alpha);
#else
	POINT *curpt, *nextpt, *lastpt;
	lastpt = pts + ncnt;
	if (bPolygon)
	{
		curpt = lastpt-1;
		nextpt = pts;
	}
	else
	{
		curpt = pts;
		nextpt = pts+1;
	}
	for (;nextpt<lastpt; curpt=nextpt++)
	{
		this->DrawLineEx(curpt->x, curpt->y, nextpt->x, nextpt->y, color, alpha, bAntialias);
	}
#endif // USING_AGG_FOR_MAP
}

void CXDraw::DrawRoadPolyEx(POINT *pts, int ncnt, const COLORREF& color, uint8_t alpha, bool bPolygon, bool bAntialias)
{
#if (defined(USING_AGG_FOR_MAP) || defined(USING_AGG_FOR_ROAD))
	AGG_DrawPoly(m_pXSurface, pts, ncnt, 1, color, bPolygon, alpha);
#else
	POINT *curpt, *nextpt, *lastpt;
	lastpt = pts + ncnt;
	if (bPolygon)
	{
		curpt = lastpt-1;
		nextpt = pts;
	}
	else
	{
		curpt = pts;
		nextpt = pts+1;
	}
	for (;nextpt<lastpt; curpt=nextpt++)
	{
		this->DrawLineEx(curpt->x, curpt->y, nextpt->x, nextpt->y, color, alpha, bAntialias);
	}
#endif // USING_AGG_FOR_MAP, USING_AGG_FOR_ROAD
}

void CXDraw::DrawWidePoly(POINT *pts, int ncnt, const COLORREF& color, int w, bool bPolygon, bool bAntialias)
{
	if (w==0)
		return;
#ifdef USING_AGG_FOR_MAP
	AGG_DrawPoly(m_pXSurface, pts, ncnt, w, color, bPolygon);
#else
	if(w<2)
		DrawPoly(pts, ncnt, color, bPolygon, bAntialias);
	else
	{
		POINT *curpt, *nextpt, *lastpt;
		lastpt = pts + ncnt;
		if (bPolygon)
		{
			curpt = lastpt-1;
			nextpt = pts;
		}
		else
		{
			curpt = pts;
			nextpt = pts+1;
		}
		for (;nextpt<lastpt; curpt=nextpt++)
		{
			DrawWideLine(curpt->x, curpt->y, nextpt->x, nextpt->y, color, w, bAntialias);
			FillAACircle(curpt->x, curpt->y, (w-2), color);
		}
		if (!bPolygon)
		{
			FillAACircle(curpt->x, curpt->y, (w-2), color);
		}
	}
#endif // USING_AGG_FOR_MAP
}
void CXDraw::DrawWidePolyEx(POINT *pts, int ncnt, const COLORREF& color, int w, uint8_t alpha, bool bPolygon, bool bAntialias)
{
	if (w==0)
		return;
#ifdef USING_AGG_FOR_MAP
	AGG_DrawPoly(m_pXSurface, pts, ncnt, w, color, bPolygon, alpha);
#else
	if(w<2)
		DrawPolyEx(pts, ncnt, color, alpha, bPolygon, bAntialias);
	else
	{
		POINT *curpt, *nextpt, *lastpt;
		lastpt = pts + ncnt;
		if (bPolygon)
		{
			curpt = lastpt-1;
			nextpt = pts;
		}
		else
		{
			curpt = pts;
			nextpt = pts+1;
		}
		for (;nextpt<lastpt; curpt=nextpt++)
		{
			this->DrawWideLineEx(curpt->x, curpt->y, nextpt->x, nextpt->y, color, w, alpha, bAntialias);
			FillAACircleEx(curpt->x, curpt->y, (w-2), color, alpha);
		}
		if (!bPolygon)
		{
			FillAACircleEx(curpt->x, curpt->y, (w-2), color, alpha);
		}
	}
#endif // USING_AGG_FOR_MAP
}
int CXDraw::GetWidth()
{
	return ::GetXSurfaceWidth(m_pXSurface);
}
int CXDraw::GetHeight()
{
	return ::GetXSurfaceHeight(m_pXSurface);
}

PXSurface CXDraw::GetRenderSurface()
{
	return m_pXSurface;
}

void CXDraw::DrawRectFocus( const POINT &pt1, const POINT &pt2)
{
	POINT pt; 
	pt.x = pt1.x;
	pt.y = pt2.y;
	DrawLineFocus(pt1, pt);
	DrawLineFocus(pt2, pt);
	pt.x = pt2.x;
	pt.y = pt1.y;
	DrawLineFocus(pt1, pt);
	DrawLineFocus(pt2, pt);
}

void CXDraw::DrawRectFocus( const RECT &rec)
{
	DrawLineFocus(rec.left, rec.top, rec.right, rec.top);
	DrawLineFocus(rec.right, rec.top, rec.right, rec.bottom);
	DrawLineFocus(rec.right, rec.bottom, rec.left, rec.bottom);
	DrawLineFocus(rec.left, rec.bottom, rec.left, rec.top);
}

void CXDraw::DrawTracker(const POINT& pt, int size)
{
	::Tracker(m_pXSurface,pt.x-size,pt.y-size,pt.x+size,pt.y+size);
}

void CXDraw::DrawPolyTracker( POINT points[], int iPointCount, int size)
{
	for (int i = 0; i < iPointCount; i++)
		DrawTracker(points[i], size);
}

void CXDraw::DrawLine(const POINT &pt1, const POINT &pt2, const COLORREF& color, const int pattern[], const int np, bool bAntialias)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawLineDash(m_pXSurface, pt1.x, pt1.y, pt2.x, pt2.y, 1, color, pattern, np);
#else
	::linePattern(m_pXSurface, pt1.x, pt1.y, pt2.x, pt2.y, Color(color), NULL, pattern, np);
#endif // USING_AGG_FOR_MAP
}

void CXDraw::DrawLineEx(const POINT &pt1, const POINT &pt2, const COLORREF& color, const int pattern[], const int np, uint8_t alpha, bool bAntialias)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawLineDash(m_pXSurface, pt1.x, pt1.y, pt2.x, pt2.y, 1, color, pattern, np, alpha);
#else
	::linePatternEx(m_pXSurface, pt1.x, pt1.y, pt2.x, pt2.y, Color(color), NULL, pattern, np, alpha);
#endif // USING_AGG_FOR_MAP
}

void CXDraw::DrawLineFocus(const POINT &pt1, const POINT &pt2)
{
	::LineFocus(m_pXSurface,pt1.x,pt1.y,pt2.x,pt2.y);
}
void CXDraw::DrawLineFocus(int x0,int y0,int x1,int y1)
{
	::LineFocus(m_pXSurface,x0,y0,x1,y1);
}

void CXDraw::DrawSnap(const POINT &pt, int nLength)
{
	::LineSnap(m_pXSurface, pt, nLength);
}

void CXDraw::FillRect( const RECT& rec, const COLORREF& cl, int fp)
{
	::FillRect(m_pXSurface,rec.left,rec.top,rec.right,rec.bottom,Color(cl),fp);	
}

void CXDraw::FillRectEx( const RECT& rec, const COLORREF& cl, int fp, uint8_t alpha)
{
	::FillRectEx(m_pXSurface,rec.left,rec.top,rec.right,rec.bottom,Color(cl),fp, alpha);
}

void CXDraw::DrawRect(const RECT& rec, const COLORREF& color, int w, const int pattern[], const int np)
{
	if (w==0)
		return;
// 	::linePattern(m_pXSurface,rec.left,rec.top,rec.right,rec.top,Color(color), NULL, pattern,np);
// 	::linePattern(m_pXSurface,rec.left,rec.bottom,rec.right,rec.bottom,Color(color), NULL, pattern,np);
// 	::linePattern(m_pXSurface,rec.left,rec.top,rec.left,rec.bottom,Color(color), NULL, pattern,np);
// 	::linePattern(m_pXSurface,rec.right,rec.top,rec.right,rec.bottom,Color(color), NULL, pattern,np);
	POINT pPts[4] = {{rec.left, rec.top}, {rec.right, rec.top}, {rec.right, rec.bottom}, {rec.left, rec.bottom}};
	DrawWidePoly(pPts, 4, color, w, pattern, np, true);
}

void CXDraw::DrawRectEx(const RECT& rec, const COLORREF& color, int w, const int pattern[], const int np, uint8_t alpha)
{
	if (w==0)
		return;
	
// 	::linePatternEx(m_pXSurface,rec.left,rec.top,rec.right,rec.top,Color(color), NULL, pattern,np,alpha);
// 	::linePatternEx(m_pXSurface,rec.left,rec.bottom,rec.right,rec.bottom,Color(color), NULL, pattern,np,alpha);
// 	::linePatternEx(m_pXSurface,rec.left,rec.top,rec.left,rec.bottom,Color(color), NULL, pattern,np,alpha);
// 	::linePatternEx(m_pXSurface,rec.right,rec.top,rec.right,rec.bottom,Color(color), NULL, pattern,np,alpha);
	POINT pPts[4] = {{rec.left, rec.top}, {rec.right, rec.top}, {rec.right, rec.bottom}, {rec.left, rec.bottom}};
	DrawWidePolyEx(pPts, 4, color, w, pattern, np, alpha, true);
}

void CXDraw::FillEllipse(const POINT& pt, const int& rx, const int& ry, const COLORREF& cl, int fp)
{
#ifdef USING_AGG_FOR_MAP
	AGG_FillEllipsePattern(m_pXSurface, pt.x, pt.y, rx, ry, cl, fp);
#else
	::FillEllipse(m_pXSurface, pt.x, pt.y, rx, ry, Color(cl), fp);
#endif // USING_AGG_FOR_MAP
}

void CXDraw::FillEllipseEx(const POINT& pt, const int& rx, const int& ry, const COLORREF& cl, uint8_t alpha, int fp)
{
#ifdef USING_AGG_FOR_MAP
	AGG_FillEllipsePattern(m_pXSurface, pt.x, pt.y, rx, ry, cl, fp, alpha);
#else
	::FillEllipseEx(m_pXSurface, pt.x, pt.y, rx, ry, Color(cl), fp, alpha);
#endif // USING_AGG_FOR_MAP
}

void CXDraw::DrawEllipseFocus(const POINT& pt, const int& rx, const int& ry)
{
	EllipseFocus(m_pXSurface,pt.x,pt.y,rx,ry);
}

void CXDraw::DrawRectTracker(const RECT& rec, int size)
{
	POINT pt;
	pt.x = rec.left; pt.y = rec.top;
	DrawTracker(pt, size);

	pt.x = rec.left; pt.y = (rec.top + rec.bottom)>>1;
	DrawTracker(pt, size);

	pt.x = rec.right; pt.y = rec.top;
	DrawTracker(pt, size);

	pt.x = (rec.right + rec.left)>>1; pt.y = rec.top;
	DrawTracker(pt, size);

	pt.x = rec.right; pt.y = rec.bottom;
	DrawTracker(pt, size);

	pt.x = rec.right; pt.y = (rec.bottom + rec.top)>>1;
	DrawTracker(pt, size);

	pt.x = rec.left; pt.y = rec.bottom;
	DrawTracker(pt, size);

	pt.x = (rec.left + rec.right)>>1; pt.y = rec.bottom;
	DrawTracker(pt, size);
}

void CXDraw::DrawPolyFocus(POINT *pts,int ncnt, bool bPolygon)
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
		this->DrawLineFocus(curpt->x,curpt->y,nextpt->x,nextpt->y);
	}
}
void CXDraw::CopyToSurface(PXSurface dest)
{
	::BlitFast(dest,this->m_pXSurface);
}

void CXDraw::DrawPoly(POINT *pts, int ncnt, const COLORREF& color, const int pattern[], const int np, bool bPolygon)
{
	if (np==0)
		return;
#ifdef USING_AGG_FOR_MAP
	AGG_DrawPolyDash(m_pXSurface, pts, ncnt, 1, color, pattern, np, bPolygon);
#else
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
	/*for(;nextpt<lastpt;curpt=nextpt++)
	{
		this->DrawLine(*curpt,*nextpt,color,pattern,np,false);
	}*/
	//////////////////////////////////////////////////////////////////////////
	// Tinh toan kich thuoc va tao pattern mau
	int npp = CalculatePatternLength(pattern, np);
	char* pTplPattern = new char[npp];
	if (pTplPattern==NULL)
	{
		return;
	}
	MakeMarkPattern(pTplPattern, pattern, np);

	// Tinh toan pattern cho tung duong va thuc hien ve
	char* pCurPattern = NULL;
	char* pLastPattern = NULL;
	int nSize = 0;
	int dx, dy;

	for(;nextpt<lastpt;curpt=nextpt++)
	{
		// Tinh toan kich thuoc pattern
		dx = nextpt->x-curpt->x;
		dy = nextpt->y-curpt->y;
		nSize = abs(dx)+abs(dy);
		nSize = (nSize/npp + 1)*npp + npp;

		// Cap phat va tao pattern
		char *pPattern = new char[nSize];
		if (pPattern==NULL)
			continue;
		pCurPattern = pPattern;
		pLastPattern = pPattern+nSize;
		for(; pCurPattern<pLastPattern; pCurPattern+=npp)
		{
			memcpy(pCurPattern, pTplPattern, npp);
		}

		// Thuc hien ve
		pCurPattern = linePattern(m_pXSurface, curpt->x, curpt->y, nextpt->x, nextpt->y, Color(color), pPattern, pattern, np);
		if (pCurPattern!=NULL)
		{
			// Cap nhat lai pattern mau
			memcpy(pTplPattern, pCurPattern, npp);
		}


		// Giai phong bo nho
		if (pPattern)
		{
			delete [] pPattern;
			pPattern = NULL;
		}
	}
	if(pTplPattern)	{
		delete [] pTplPattern;
		pTplPattern = NULL;
	}
#endif // USING_AGG_FOR_MAP
}

void CXDraw::DrawPolyEx(POINT *pts, int ncnt, const COLORREF& color, const int pattern[], const int np, uint8_t alpha, bool bPolygon)
{
	if (np==0)
		return;
#ifdef USING_AGG_FOR_MAP
	AGG_DrawPolyDash(m_pXSurface, pts, ncnt, 1, color, pattern, np, bPolygon, alpha);
#else
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
	//////////////////////////////////////////////////////////////////////////
	// Tinh toan kich thuoc va tao pattern mau
	int npp = CalculatePatternLength(pattern, np);
	char* pTplPattern = new char[npp];
	if (pTplPattern==NULL)
	{
		return;
	}
	MakeMarkPattern(pTplPattern, pattern, np);

	// Tinh toan pattern cho tung duong va thuc hien ve
	char* pCurPattern = NULL;
	char* pLastPattern = NULL;
	int nSize = 0;
	int dx, dy;

	for(;nextpt<lastpt;curpt=nextpt++)
	{
		// Tinh toan kich thuoc pattern
		dx = nextpt->x-curpt->x;
		dy = nextpt->y-curpt->y;
		nSize = abs(dx)+abs(dy);
		nSize = (nSize/npp + 1)*npp + npp;

		// Cap phat va tao pattern
		char *pPattern = new char[nSize];
		if (pPattern==NULL)
			continue;
		pCurPattern = pPattern;
		pLastPattern = pPattern+nSize;
		for(; pCurPattern<pLastPattern; pCurPattern+=npp)
		{
			memcpy(pCurPattern, pTplPattern, npp);
		}

		// Thuc hien ve
		pCurPattern = linePatternEx(m_pXSurface, curpt->x, curpt->y, nextpt->x, nextpt->y, Color(color), pPattern, pattern, np,alpha);
		if (pCurPattern!=NULL)
		{
			// Cap nhat lai pattern mau
			memcpy(pTplPattern, pCurPattern, npp);
		}


		// Giai phong bo nho
		if (pPattern)
		{
			delete [] pPattern;
			pPattern = NULL;
		}
	}
	if(pTplPattern)	{
		delete [] pTplPattern;
		pTplPattern = NULL;
	}
#endif // USING_AGG_FOR_MAP
}

void CXDraw::DrawCallout(const RECT& rec, const POINT& pt, const COLORREF& color, const COLORREF& colorLine)
{
	RECT recMain = rec, recS1, recS2;
	int w = (int)((double)(recMain.right-recMain.left)/ 3.25);
	int iSZRec = min((rec.right-rec.left), (rec.bottom-rec.top))>>1;
	int w2 = (iSZRec > 26) ? 26 : iSZRec>>1;
	int h2 = w2;
	recMain.left+= w2;
	recMain.right-= w2+1;

	POINT ptEl1, ptEl2, ptEl3, ptEl4;
	ptEl1.x = recMain.left;
	ptEl1.y = rec.top + h2;

	ptEl2.x = recMain.right;
	ptEl2.y = rec.top + h2;

	ptEl3.x = recMain.right;
	ptEl3.y = rec.bottom - h2;

	ptEl4.x = recMain.left;
	ptEl4.y = rec.bottom - h2;

	// side recs
	recS1.left = rec.left;
	recS1.top = rec.top + h2;
	recS1.right = rec.left + w;
	recS1.bottom = rec.bottom - h2;

	recS2.left = rec.right - w;
	recS2.top = rec.top + h2;
	recS2.right = rec.right;
	recS2.bottom = rec.bottom - h2;

	POINT pts[3];
	pts[0].x = rec.left + w;
	pts[0].y = rec.bottom -1;
	pts[1] = pt;
	pts[2].x = rec.left + (w>>1) + w;
	pts[2].y = rec.bottom -1;
	
	//draw line first
	//recMain.top--;	
	DrawRect(recMain, colorLine);
	//recMain.top++;
	int w2E = w2 - 1;

	DrawAACircle(rec.left + w2, rec.top + w2E, (w2<<1), colorLine); 
	DrawAACircle(rec.left + w2, rec.bottom - w2, (w2<<1) + 1, colorLine); 
	DrawAACircle(rec.right - w2, rec.top + w2, (w2<<1) + 1, colorLine); 
	DrawAACircle(rec.right - w2 - 1, rec.bottom - w2, (w2<<1) + 1, colorLine); 

	//four conrners
	FillEllipse(ptEl1, w2, h2, color);
	FillEllipse(ptEl2, w2, h2, color);
	FillEllipse(ptEl3, w2, h2-1, color);
	FillEllipse(ptEl4, w2, h2-1, color);
	/*
	DrawAACircle(rec.left + w2, rec.top + w2E, (w2<<1), colorLine); 
	DrawAACircle(rec.left + w2, rec.bottom - w2, (w2<<1) + 1, colorLine); 
	DrawAACircle(rec.right - w2, rec.top + w2, (w2<<1) + 1, colorLine); 
	DrawAACircle(rec.right - w2 - 1, rec.bottom - w2, (w2<<1) + 1, colorLine); 

	//fill four conrners again
	FillEllipse(ptEl1, w2-2, h2-2, color);
	FillEllipse(ptEl2, w2-2, h2-2, color);
	FillEllipse(ptEl3, w2-2, h2-3, color);
	FillEllipse(ptEl4, w2-2, h2-3, color);*/

	// side recs
	FillRect(recS1, color);
	FillRect(recS2, color);

	FillPoly(pts, 3, color);
	DrawPoly(pts, 3, colorLine, true);
	// center rec
	recMain.bottom--;
	FillRect(recMain, color);
	//FillRect(recMain, RGB(0,0,0));
	DrawVLine(recS1.left, recS1.top, recS1.bottom, colorLine);
	DrawVLine(recS2.right-1, recS2.top, recS2.bottom, colorLine);
	DrawHLine(recMain.left, recMain.top, recMain.right, colorLine);
	
	DrawHLine(recMain.left, recMain.bottom, recMain.right, color);
}

void CXDraw::FillPoly(POINT *pt, int n, const COLORREF& color, int fp, bool bClip, uint8_t alpha)
{
#ifdef USING_AGG_FOR_MAP
	AGG_FillPolyPattern(m_pXSurface, pt, n, color, fp, alpha);
#else
	::FillPoly(m_pXSurface,pt,n,Color(color),fp,bClip,alpha);
#endif // USING_AGG_FOR_MAP
}

void CXDraw::FillPolyAA(POINT *pt, int n, const COLORREF& color)
{
	if (m_pPolygon && m_pXSurface)
		XDrawRoad::FillPolyAA(m_pPolygon, (BYTE*)m_pXSurface->pixel, m_pXSurface->pitch, pt, n, Color(color));
	else
		FillPoly(pt, n, color, 1, true, 255);
}

void CXDraw::FillRoadPoly(POINT *pt, int n, const COLORREF& color, int fp, bool bClip, uint8_t alpha)
{
#if (defined(USING_AGG_FOR_MAP) || defined(USING_AGG_FOR_ROAD))
	AGG_FillPolyPattern(m_pXSurface, pt, n, color, fp, alpha);
#else
	::FillPoly(m_pXSurface,pt,n,Color(color),fp,bClip,alpha);
#endif // USING_AGG_FOR_MAP, USING_AGG_FOR_ROAD
}

void CXDraw::FillPolyEx(POINT *pt,int n,const COLORREF& color, int fp, bool bClip, uint8_t alpha)
{
#ifdef USING_AGG_FOR_MAP
	AGG_FillPolyPattern(m_pXSurface, pt, n, color, fp, alpha);
#else
	::FillPolyEx(m_pXSurface, pt, n, Color(color));
#endif // USING_AGG_FOR_MAP
}

void CXDraw::FillRoadPolyEx(POINT *pt, int n, const COLORREF& color, uint8_t alpha)
{
#if (defined(USING_AGG_FOR_MAP) || defined(USING_AGG_FOR_ROAD))
	AGG_FillPolyEx(m_pXSurface, pt, n, color, alpha);
#else
	::FillPoly(m_pXSurface, pt, n, Color(color), 1, true, alpha);
	::DrawPoly(m_pXSurface, pt, n, Color(color), true, true);
#endif // USING_AGG_FOR_MAP, USING_AGG_FOR_ROAD
}

void CXDraw::FillPoly(POINT* pPts, int nPt, PXSurface pSrcSurface, uint8_t alpha)
{
#ifdef USING_AGG_FOR_MAP
	AGG_FillPolyPattern(m_pXSurface, pPts, nPt, pSrcSurface, alpha);
#endif // USING_AGG_FOR_MAP
}

#ifdef OS_WIN
void CXDraw::FillPoly(POINT* pPts, int nPt, HBITMAP hBm, uint8_t alpha)
{
#ifdef USING_AGG_FOR_MAP
	AGG_FillPolyPattern(m_pXSurface, pPts, nPt, hBm, alpha);
#endif // USING_AGG_FOR_MAP
}

void CXDraw::FillPoly(POINT* pPts, int nPt, int iResourceID, uint8_t alpha)
{
#ifdef USING_AGG_FOR_MAP
	if(fillPatterns.find(iResourceID) == fillPatterns.end())	{
		HBITMAP hBm = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(iResourceID));
		if (!hBm)
			return;

		PXSurface fillSurf = ::CreateXSurface(hBm);
		if (!fillSurf)
			return;
		::DeleteObject(hBm);
		fillPatterns[iResourceID] = fillSurf;
	}
	AGG_FillPolyPattern(m_pXSurface, pPts, nPt, fillPatterns[iResourceID], alpha);
#endif // USING_AGG_FOR_MAP
}
#endif // OS_WIN

void CXDraw::DrawEllipse(const POINT& pt, const int& rx, const int& ry, const COLORREF& cl, int iWidth)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawEllipse(m_pXSurface, pt.x, pt.y, rx, ry, iWidth, cl);
#else
	::Ellipse(m_pXSurface, pt.x, pt.y, rx, ry, Color(cl), iWidth);
#endif // USING_AGG_FOR_MAP
}
void CXDraw::DrawEllipseEx(const POINT& pt, const int& rx, const int& ry, const COLORREF& cl, uint8_t alpha, int iWidth)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawEllipse(m_pXSurface, pt.x, pt.y, rx, ry, iWidth, cl, alpha);
#else
	uint8_t alpha_new = alpha;
	if (iWidth>=8)
		alpha_new = alpha_new/8;
	else if (iWidth>0)
		alpha_new = alpha_new/iWidth;
	if (alpha_new==0)
		alpha_new = 1;
	::EllipseEx(m_pXSurface, pt.x, pt.y, rx, ry, Color(cl), alpha_new, iWidth);
#endif // USING_AGG_FOR_MAP
}
void CXDraw::DrawEllipse(const POINT& pt, const int& rx, const int& ry, const COLORREF& cl, int iWidth, const int pattern[], const int np)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawEllipseDash(m_pXSurface, pt.x, pt.y, rx, ry, iWidth, cl, pattern, np);
#else
	::EllipsePattern(m_pXSurface, pt.x, pt.y, rx, ry, Color(cl), iWidth, pattern, np);
#endif // USING_AGG_FOR_MAP
}
void CXDraw::DrawEllipseEx(const POINT& pt, const int& rx, const int& ry, const COLORREF& cl, int iWidth, const int pattern[], const int np, uint8_t alpha)
{
#ifdef USING_AGG_FOR_MAP
	AGG_DrawEllipseDash(m_pXSurface, pt.x, pt.y, rx, ry, iWidth, cl, pattern, np, alpha);
#else
	::EllipsePatternEx(m_pXSurface, pt.x, pt.y, rx, ry, Color(cl), iWidth, pattern, np, alpha);
#endif // USING_AGG_FOR_MAP
}
void CXDraw::DrawWidePoly(POINT *pts, int ncnt, const COLORREF& color, int w, const int pattern[], const int np, bool bPolygon)
{
	if (w==0)
		return;
#ifdef USING_AGG_FOR_MAP
	AGG_DrawPolyDash(m_pXSurface, pts, ncnt, w, color, pattern, np, bPolygon);
#else
	if(w < 2)	{
		DrawPoly(pts, ncnt, color, pattern, np, bPolygon);
		return;
	}
	else
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
		/*
		for(;nextpt<lastpt;curpt=nextpt++)
		{
			WideLinePattern(m_pXSurface,curpt->x,curpt->y,nextpt->x,nextpt->y,w,Color(color),pattern,np);
		}*/
		//////////////////////////////////////////////////////////////////////////
		// Tinh toan kich thuoc va tao pattern mau
		int npp = CalculatePatternLength(pattern, np);
		char* pTplPattern = new char[npp];
		if (pTplPattern==NULL)
		{
			return;
		}
		MakeMarkPattern(pTplPattern, pattern, np);

		// Tinh toan pattern cho tung duong va thuc hien ve
		char* pCurPattern = NULL;
		char* pLastPattern = NULL;
		int nSize = 0;
		int dx, dy;

		for(;nextpt<lastpt;curpt=nextpt++)
		{
			// Tinh toan kich thuoc pattern
			dx = nextpt->x-curpt->x;
			dy = nextpt->y-curpt->y;
			nSize = abs(dx)+abs(dy);
			nSize = (nSize/npp + 1)*npp + npp;

			// Cap phat va tao pattern
			char *pPattern = new char[nSize];
			if (pPattern==NULL)
				continue;
			pCurPattern = pPattern;
			pLastPattern = pPattern+nSize;
			for(; pCurPattern<pLastPattern; pCurPattern+=npp)
			{
				memcpy(pCurPattern, pTplPattern, npp);
			}

			// Thuc hien ve
			pCurPattern = WideLinePattern(m_pXSurface, curpt->x, curpt->y, nextpt->x, nextpt->y, w, Color(color), pPattern, pattern, np);
			if (pCurPattern!=NULL)
			{
				// Cap nhat lai pattern mau
				memcpy(pTplPattern, pCurPattern, npp);
			}


			// Giai phong bo nho
			if (pPattern)
			{
				delete [] pPattern;
				pPattern = NULL;
			}			
		}
		if(pTplPattern)	{
			delete [] pTplPattern;
			pTplPattern = NULL;
		}
	}
#endif // USING_AGG_FOR_MAP
}

void CXDraw::DrawWidePolyEx(POINT *pts, int ncnt, const COLORREF& color, int w, const int pattern[], const int np, uint8_t alpha, bool bPolygon)
{
	if (w==0)
		return;
#ifdef USING_AGG_FOR_MAP
	AGG_DrawPolyDash(m_pXSurface, pts, ncnt, w, color, pattern, np, bPolygon, alpha);
#else
	if(w < 2)	{
		DrawPolyEx(pts, ncnt, color, pattern, np, alpha, bPolygon);
		return;
	}
	else
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
		//////////////////////////////////////////////////////////////////////////
		// Tinh toan kich thuoc va tao pattern mau
		int npp = CalculatePatternLength(pattern, np);
		char* pTplPattern = new char[npp];
		if (pTplPattern==NULL)
		{
			return;
		}
		MakeMarkPattern(pTplPattern, pattern, np);

		// Tinh toan pattern cho tung duong va thuc hien ve
		char* pCurPattern = NULL;
		char* pLastPattern = NULL;
		int nSize = 0;
		int dx, dy;

		for(;nextpt<lastpt;curpt=nextpt++)
		{
			// Tinh toan kich thuoc pattern
			dx = nextpt->x-curpt->x;
			dy = nextpt->y-curpt->y;
			nSize = abs(dx)+abs(dy);
			nSize = (nSize/npp + 1)*npp + npp;

			// Cap phat va tao pattern
			char *pPattern = new char[nSize];
			if (pPattern==NULL)
				continue;
			pCurPattern = pPattern;
			pLastPattern = pPattern+nSize;
			for(; pCurPattern<pLastPattern; pCurPattern+=npp)
			{
				memcpy(pCurPattern, pTplPattern, npp);
			}

			// Thuc hien ve
			pCurPattern = WideLinePatternEx(m_pXSurface, curpt->x, curpt->y, nextpt->x, nextpt->y, w, Color(color), pPattern, pattern, np, alpha);
			if (pCurPattern!=NULL)
			{
				// Cap nhat lai pattern mau
				memcpy(pTplPattern, pCurPattern, npp);
			}


			// Giai phong bo nho
			if (pPattern)
			{
				delete [] pPattern;
				pPattern = NULL;
			}			
		}
		if(pTplPattern)	{
			delete [] pTplPattern;
			pTplPattern = NULL;
		}
	}
#endif // USING_AGG_FOR_MAP
}

void CXDraw::DrawWideLine(const POINT &pt1, const POINT &pt2, const COLORREF& color, int w, const int pattern[], const int np, bool bAntialias)
{
	if (w==0)
		return;
#ifdef USING_AGG_FOR_MAP
	AGG_DrawLineDash(m_pXSurface, pt1.x, pt1.y, pt2.x, pt2.y, w, color, pattern, np);
#else
	if(w<2)
	{
		DrawLine(pt1, pt2, color, pattern, np, bAntialias);
	}
	else
	{
		WideLinePattern(m_pXSurface, pt1.x, pt1.y, pt2.x, pt2.y, w, Color(color), NULL, pattern, np);
	}
#endif // USING_AGG_FOR_MAP
}

void CXDraw::DrawWideLineEx(const POINT &pt1, const POINT &pt2, const COLORREF& color, int w, const int pattern[], const int np, uint8_t alpha, bool bAntialias)
{
	if (w==0)
		return;
#ifdef USING_AGG_FOR_MAP
	AGG_DrawLineDash(m_pXSurface, pt1.x, pt1.y, pt2.x, pt2.y, w, color, pattern, np, alpha);
#else
	if (w<2)
	{
		DrawLineEx(pt1, pt2, color, pattern, np, alpha, bAntialias);
	}
	else
	{
		WideLinePatternEx(m_pXSurface, pt1.x, pt1.y, pt2.x, pt2.y, w, Color(color), NULL, pattern, np, alpha);
	}
#endif // USING_AGG_FOR_MAP
}

void CXDraw::DrawCircleFocus(const POINT& pt, const int& iRadius)
{
	this->DrawEllipseFocus(pt,iRadius,iRadius);
}

void CXDraw::FillPolyPolyMcDowell(POINT vertex_vector[], int vertex_cnt[], int nvertex, const COLORREF& cl, int fp, uint8_t alpha)
{
	FillPolyPoly(m_pXSurface,vertex_vector,vertex_cnt,nvertex,Color(cl),fp, alpha);
}

void CXDraw::FillPolyPolyMcDowellAA(POINT vertex_vector[], int vertex_cnt[], int nvertex, const COLORREF& cl)
{
	if (!vertex_vector || !vertex_cnt)
		return;
	if (m_pPolygon && m_pXSurface)
		XDrawRoad::FillPolyPolyMcDowellAA(m_pPolygon, (BYTE*)m_pXSurface->pixel, m_pXSurface->pitch, vertex_vector, vertex_cnt, nvertex, Color(cl));
	else
		FillPolyPolyMcDowell(vertex_vector, vertex_cnt, nvertex, cl, 1, 255);
}

bool CXDraw::SaveImageA( const PXSurface const_surface, const char* filename )
{
	wchar_t sFile[MAX_PATH];
	memset(sFile, 0, sizeof(wchar_t)*MAX_PATH);
	mbstowcs(sFile, filename, MAX_PATH);
	return SaveXSurface(const_surface, sFile);	
}

bool CXDraw::SaveImageW( const PXSurface const_surface, const wchar_t* filename )
{
	return SaveXSurface(const_surface, filename);	
}

void CXDraw::Draw3DPolygon(POINT *pts, const int& npt, const int& iHeight, const COLORREF& clShadow, const COLORREF& clTopFill, const COLORREF& clLineFront, const COLORREF& clLineBack)
{
	//draw back
	DrawPoly(pts, npt, clLineBack, true);
	//draw shadow 	
	POINT ptSh[4];	
	int i;
	for(i=0;i<npt;i++)	{
		ptSh[0] = pts[i];
		ptSh[1] = pts[(i+1)% npt];
		ptSh[2].x = ptSh[1].x; ptSh[2].y = ptSh[1].y - iHeight;
		ptSh[3].x = ptSh[0].x; ptSh[3].y = ptSh[0].y - iHeight;
		FillPoly(ptSh, 4, clShadow, 1, true, 200);
		DrawPoly(ptSh, 4, clLineFront, true);
	}
	for(i=0;i<npt;i++)
		pts[i].y-= iHeight;	
	FillPoly(pts, npt, clTopFill, 1, true, 180);
	DrawPoly(pts, npt, clLineFront, true);
}

void CXDraw::Draw3DPolygonEx(POINT *pts, const int& npt, const int& iHeight, const COLORREF& clShadow, const COLORREF& clTopFill, const COLORREF& clLineFront, const COLORREF& clLineBack, uint8_t alpha)
{
	//draw back
	DrawPolyEx(pts, npt, clLineBack, alpha, true);
	//draw shadow 	
	POINT ptSh[4];	
	int i;
	for(i=0;i<npt;i++)	{
		ptSh[0] = pts[i];
		ptSh[1] = pts[(i+1)% npt];
		ptSh[2].x = ptSh[1].x; ptSh[2].y = ptSh[1].y - iHeight;
		ptSh[3].x = ptSh[0].x; ptSh[3].y = ptSh[0].y - iHeight;
		FillPoly(ptSh, 4, clShadow, 1, true, (uint8_t)((200*(int)alpha)>>8));
		DrawPolyEx(ptSh, 4, clLineFront, alpha, true);
	}	
	for(i=0;i<npt;i++)
		pts[i].y-= iHeight;	
	FillPoly(pts, npt, clTopFill, 1, true, (uint8_t)((180*(int)alpha)>>8));
	DrawPolyEx(pts, npt, clLineFront, alpha, true);
}

void CXDraw::Draw3DPolyPolygon(POINT vertex_vector[], int vertex_cnt[], int nvertex, const int& iHeight, const COLORREF& clShadow, const COLORREF& clTopFill, const COLORREF& clLineFront, const COLORREF& clLineBack)
{
	//draw back
	int i;
	for(i=0;i<nvertex-1;i++)	
		DrawPoly(&vertex_vector[vertex_cnt[i]], vertex_cnt[i+1] - vertex_cnt[i], clLineBack, true);
	//draw shadow
	POINT ptSh[4];	
	for(int j=0;j<nvertex-1;j++)	{
		int npt = vertex_cnt[j+1] - vertex_cnt[j];
		POINT* pts = &vertex_vector[vertex_cnt[j]];
		int i;
		for(i=0;i<npt;i++)	{
			ptSh[0] = pts[i];
			ptSh[1] = pts[(i+1)% npt];
			ptSh[2].x = ptSh[1].x; ptSh[2].y = ptSh[1].y - iHeight;
			ptSh[3].x = ptSh[0].x; ptSh[3].y = ptSh[0].y - iHeight;
			FillPoly(ptSh, 4, clShadow, 1, true, 200);
			DrawPoly(ptSh, 4, clLineFront, true);
		}
		for(i=0;i<npt;i++)
			pts[i].y-= iHeight;	
	}
	FillPolyPolyMcDowell(vertex_vector, vertex_cnt, nvertex, clTopFill, 1, 180);
	for(i=0;i<nvertex-1;i++)	
		DrawPoly(&vertex_vector[vertex_cnt[i]], vertex_cnt[i+1] - vertex_cnt[i], clLineFront, true);
}

void CXDraw::Draw3DPolyPolygonEx(POINT vertex_vector[], int vertex_cnt[], int nvertex, const int& iHeight, const COLORREF& clShadow, const COLORREF& clTopFill, const COLORREF& clLineFront, const COLORREF& clLineBack, uint8_t alpha)
{
	//draw back
	int i;
	for(i=0;i<nvertex-1;i++)	
		DrawPolyEx(&vertex_vector[vertex_cnt[i]], vertex_cnt[i+1] - vertex_cnt[i], clLineBack, alpha,true);
	//draw shadow
	POINT ptSh[4];	
	for(int j=0;j<nvertex-1;j++)	{
		int npt = vertex_cnt[j+1] - vertex_cnt[j];
		POINT* pts = &vertex_vector[vertex_cnt[j]];
		int i;
		for(i=0;i<npt;i++)	{
			ptSh[0] = pts[i];
			ptSh[1] = pts[(i+1)% npt];
			ptSh[2].x = ptSh[1].x; ptSh[2].y = ptSh[1].y - iHeight;
			ptSh[3].x = ptSh[0].x; ptSh[3].y = ptSh[0].y - iHeight;
			FillPoly(ptSh, 4, clShadow, 1, true, (uint8_t)((200*(int)alpha)>>8));
			DrawPolyEx(ptSh, 4, clLineFront, alpha, true);
		}
		for(i=0;i<npt;i++)
			pts[i].y-= iHeight;	
	}
	FillPolyPolyMcDowell(vertex_vector, vertex_cnt, nvertex, clTopFill, 1, (uint8_t)((180*(int)alpha)>>8));
	for(i=0;i<nvertex-1;i++)	
		DrawPolyEx(&vertex_vector[vertex_cnt[i]], vertex_cnt[i+1] - vertex_cnt[i], clLineFront, alpha, true);
}

void CXDraw::DrawPolyPattern(POINT *pts, int ncnt, PXSurface pSrcSurface, bool bPolygon)
{
#ifdef USE_PATTERN
	DrawPolyUsingPattern(GetRenderSurface(), pts, ncnt, pSrcSurface, bPolygon);
#endif// USE_PATTERN
}

#ifdef OS_WIN
//draw line bitmap patterns
void CXDraw::DrawPolyPattern(POINT *pts, int ncnt, int iResourceID, bool bPolygon)	
{
#ifdef USE_PATTERN
	//clip in the SDK
	DrawPolyUsingPattern(GetRenderSurface(), pts, ncnt, iResourceID, bPolygon);
#endif//#ifdef USE_PATTERN
}

void CXDraw::DrawPolyPattern(POINT *pts, int ncnt, HBITMAP hBm, bool bPolygon)	
{
#ifdef USE_PATTERN
	//clip in the SDK
	DrawPolyUsingPattern(GetRenderSurface(), pts, ncnt, hBm, bPolygon);
#endif//#ifdef USE_PATTERN
}
#endif // OS_WIN

void CXDraw::DrawCompass(const POINT& pt, const int& iSz, const int& iLx, const int& iLy)
{
	//iLx: 1024* sin, iLy: -1024* cos of vehicle direction angle
	FillCircle(pt, (iSz<<1)-1, RGB(204, 255, 204));
	DrawCircle(pt, (iSz<<1), RGB(22, 24, 22));
	POINT ptN, ptS, ptE, ptW;
	int iLyg = ((iLy* iSz)>>10);
	int iLxg = ((iLx* iSz)>>10);
	ptN.x = pt.x + iLyg;
	ptN.y = pt.y - iLxg;

	ptS.x = pt.x - iLyg;
	ptS.y = pt.y + iLxg;

	ptE.x = pt.x + ((iLxg* 3)>>3);
	ptE.y = pt.y + ((iLyg* 3)>>3);

	ptW.x = pt.x - ((iLxg* 3)>>3);
	ptW.y = pt.y - ((iLyg* 3)>>3);

	POINT pts[3];
	pts[0] = ptE;
	pts[1] = ptN;
	pts[2] = ptW;
	
	FillPoly(pts, 3, RGB(255, 0, 0));
	DrawPoly(pts, 3, RGB(22, 24, 22));

	pts[0] = ptS;
	pts[1] = ptE;
	pts[2] = ptW;

	FillPoly(pts, 3, RGB(255, 255, 255));
	DrawPoly(pts, 3, RGB(22, 24, 22), true);
}

void CXDraw::DrawPolyInside(POINT *pData, int iCount, const int& iWidth, const COLORREF& colorLine, const COLORREF& colorFill, const int& nInsideHatchWidth, const int& nCellSize)
{
	FillPolyInside(m_pXSurface, pData, iCount, Color(colorLine), Color(colorFill), nInsideHatchWidth, nCellSize);	
	DrawWidePoly(pData, iCount, colorLine, iWidth, true);	
}

void CXDraw::DrawPolygonEx(const POINT *pt, int np, int w, DWORD color)
{
#ifdef USE_PATTERN
	PolygonEx(m_pXSurface, pt, np, w, color);
#endif//#ifdef USE_PATTERN
}

void CXDraw::DrawArrowOnPolyline(POINT* pPts, int nPt, bool bBack, uint8_t alpha, const COLORREF clrArrow, int nArrowLen, int nArrowOff, int nArrHeadLen, int nArrHeadWid)
{
	if (m_pXSurface==NULL || pPts==NULL ||nPt<=0)
		return;

#ifdef USING_AGG_ARROW
	AGG_DrawArrowOnPoly(m_pXSurface, pPts, nPt, bBack, alpha, clrArrow, nArrowLen, nArrowOff, nArrHeadLen, nArrHeadWid);
#else
	int i;
	if (bBack)	// Doi nguoc cac diem
	{
		int nDiv = nPt/2;
		POINT pt;
		for (i=0; i<nDiv; i++)
		{
			pt = pPts[i];
			pPts[i] = pPts[nPt-1-i];
			pPts[nPt-1-i] = pt;
		}
	}

	// Tinh do dai polyline
	int nPolyLen = 0;
	int ux, uy;
	for (i=0; i<nPt-1; i++)
	{
		ux = pPts[i+1].x - pPts[i].x;
		uy = pPts[i+1].y - pPts[i].y;
		nPolyLen += julery_isqrt(ux*ux+uy*uy);
	}

	// Tinh toan va ve
	int nArrowNum = nPolyLen/(nArrowLen+nArrowOff);
	int nCurLen = (nArrowOff + nPolyLen - (nArrowLen+nArrowOff)*nArrowNum)>>1;
	if (nArrowNum<=0 && nArrowLen<nPolyLen)
	{
		nArrowNum = 1;
		nCurLen = (nPolyLen-nArrowLen)>>1;
		if (nCurLen==0)
			nCurLen = 1;
	}
	int nArrLen, nRemainLen;
	int nLen = 0;
	POINT pt1, pt2, pt3, pt4, pt5;
	int nArrIdx = 0;
	int nPntIdx = 0;
	while (nArrIdx<nArrowNum && nPntIdx<nPt-1 && nCurLen>0)
	{
		ux = pPts[nPntIdx+1].x - pPts[nPntIdx].x;
		uy = pPts[nPntIdx+1].y - pPts[nPntIdx].y;
		nLen = julery_isqrt(ux*ux+uy*uy);
		if (nLen<nCurLen)
		{
			nCurLen = nCurLen-nLen;
			nPntIdx++;
			continue;
		}
		if (nLen==0)
			break;
		pt1.x = pPts[nPntIdx].x + nCurLen*ux/nLen;
		pt1.y = pPts[nPntIdx].y + nCurLen*uy/nLen;
		nArrLen = nArrowLen;
		nRemainLen = nLen - nCurLen;
		while (nRemainLen<nArrLen && nPntIdx<nPt-2)
		{
			if (alpha==255)
				//DrawLine(pt1.x, pt1.y, pPts[nPntIdx+1].x, pPts[nPntIdx+1].y, clrArrow, true);
				DrawWideLine(pt1, pPts[nPntIdx+1], clrArrow, 2, true);
			else
				//DrawLineEx(pt1.x, pt1.y, pPts[nPntIdx+1].x, pPts[nPntIdx+1].y, clrArrow, alpha, true);
				DrawWideLineEx(pt1, pPts[nPntIdx+1], clrArrow, 2, alpha, true);

			pt1 = pPts[nPntIdx+1];
			nPntIdx++;
			ux = pPts[nPntIdx+1].x - pPts[nPntIdx].x;
			uy = pPts[nPntIdx+1].y - pPts[nPntIdx].y;
			nLen = julery_isqrt(ux*ux+uy*uy);
			nArrLen -= nRemainLen;
			nRemainLen = nLen;
			nCurLen = 0;
		}
		if (nLen==0)
			break;
		pt2.x = pt1.x + nArrLen*ux/nLen;
		pt2.y = pt1.y + nArrLen*uy/nLen;
		pt3.x = pt1.x + (nArrLen-nArrHeadLen)*ux/nLen;
		pt3.y = pt1.y + (nArrLen-nArrHeadLen)*uy/nLen;
		int temp = ux;
		ux = -uy;
		uy = temp;
		pt4.x = pt3.x + nArrHeadWid*ux/nLen;
		pt4.y = pt3.y + nArrHeadWid*uy/nLen;
		pt5.x = pt3.x - (nArrHeadWid/*+1*/)*ux/nLen;
		pt5.y = pt3.y - (nArrHeadWid/*+1*/)*uy/nLen;
		if (alpha==255)
		{
			//DrawLine(pt1.x, pt1.y, pt2.x, pt2.y, clrArrow, true);
			DrawWideLine(pt1, pt2, clrArrow, 2, true);
			//DrawLine(pt2.x, pt2.y, pt4.x, pt4.y, clrArrow, true);
// 			DrawWideLine(pt2, pt4, clrArrow, 2, true);
			//DrawLine(pt2.x, pt2.y, pt5.x, pt5.y, clrArrow, true);
// 			DrawWideLine(pt2, pt5, clrArrow, 2, true);
// 			DrawWideLine(pt4, pt5, clrArrow, 2, true);
			POINT arrPts[3]; arrPts[0] = pt2; arrPts[1] = pt4; arrPts[2] = pt5;
			FillPoly(arrPts, 3, clrArrow);
			DrawLine(pt2.x, pt2.y, pt4.x, pt4.y, clrArrow, true);
			DrawLine(pt2.x, pt2.y, pt5.x, pt5.y, clrArrow, true);
		}
		else
		{
			//DrawLineEx(pt1.x, pt1.y, pt2.x, pt2.y, clrArrow, alpha, true);
			DrawWideLineEx(pt1, pt2, clrArrow, 2, alpha, true);
			//DrawLineEx(pt2.x, pt2.y, pt4.x, pt4.y, clrArrow, alpha, true);
			DrawWideLineEx(pt2, pt4, clrArrow, 2, alpha, true);
			//DrawLineEx(pt2.x, pt2.y, pt5.x, pt5.y, clrArrow, alpha, true);
			DrawWideLineEx(pt2, pt5, clrArrow, 2, alpha, true);
		}
		nArrIdx++;
		nCurLen += nArrowLen + nArrowOff;
	}

	if (bBack)	// Doi nguoc cac diem de ve trang thai cu
	{
		int nDiv = nPt/2;
		POINT pt;
		for (i=0; i<nDiv; i++)
		{
			pt = pPts[i];
			pPts[i] = pPts[nPt-1-i];
			pPts[nPt-1-i] = pt;
		}
	}
#endif // USING_AGG_ARROW
}

void CXDraw::DrawArrowOnRoad(POINT* pPts, int nPt, bool bBack, uint8_t alpha, const COLORREF& clrArrow, int nArrowLen, int nArrowOff, int nArrHeadLen, int nArrHeadWid)
{
#ifdef USING_AGG_ARROW
	AGG_DrawArrowOnRoad(m_pXSurface, pPts, nPt, bBack, alpha, clrArrow, nArrowLen, nArrowOff, nArrHeadLen, nArrHeadWid);
#endif // USING_AGG_ARROW
}

void CXDraw::DrawArrow(POINT *pPts, int nPt, int nArrowWid, uint8_t alpha, const COLORREF& clrArrow, int nArrHeadLen, int nArrHeadWid, bool bHaveTail)
{
#ifdef USING_AGG_ARROW
	AGG_DrawArrow(m_pXSurface, pPts, nPt, nArrowWid, alpha, clrArrow, nArrHeadLen, nArrHeadWid, bHaveTail);
#endif // USING_AGG_ARROW
}

void CXDraw::InitRoadDraw()
{
	CloseRoadDraw();
	if (m_pXSurface)
		XDrawRoad::InitDraw(m_pPolygon, m_pXSurface->width, m_pXSurface->height);
}
void CXDraw::CloseRoadDraw()
{
	if (m_pPolygon)
		XDrawRoad::CloseDraw(m_pPolygon);
}

float CXDraw::Get3DRoadWidth(const POINT& pt, int nWid)
{
	return XDrawRoad::Get3DRoadWidth(pt, nWid);
}

bool CXDraw::Get3DRoadWidth(const POINT& pt1, const POINT& pt2, int nWid, float& dWid1, float& dWid2)
{
	return XDrawRoad::Get3DRoadWidth(pt1, pt2, nWid, dWid1, dWid2);
}

size_t fclip(POINT *,int ,float ,float ,float  ,float ,vector<int>&, bool);
void CXDraw::Draw2DRoad(POINT *pPts, int nPt, int nWid, COLORREF clrInline, COLORREF clrOutline, bool bOutline, bool bButtCap)
{
	if (m_pPolygon && m_pXSurface && pPts)
	{
		//RECT rcClip = {0, 0, m_pXSurface->width, m_pXSurface->height};
		RECT rcClip = {-nWid, -nWid, m_pXSurface->width + nWid, m_pXSurface->height + nWid};
		vector<VPOINT> vPolylines;
		if (CMyClip::ClipPolyline(rcClip, pPts, nPt, vPolylines))
		{
			vector<VPOINT>::iterator itEnd = vPolylines.end();
			int nPt = 0;
			for (vector<VPOINT>::iterator it = vPolylines.begin(); it!=itEnd; it++)
			{
				nPt = it->size();
				if (nPt>1) {
					XDrawRoad::Draw2DRoad(m_pPolygon, (BYTE*)m_pXSurface->pixel, m_pXSurface->pitch, it->data(), nPt, (float)nWid, Color(clrInline), Color(clrOutline), bOutline, bButtCap);
				}
			}
		}
	
// 		XDrawRoad::Draw2DRoad(m_pPolygon, (BYTE*)m_pXSurface->pixel, m_pXSurface->pitch, pPts, nPt, (float)nWid, Color(clrInline), Color(clrOutline), bOutline, bButtCap);
		
// 		vector<int> vPts; vPts.reserve(2*nPt);
// 		int nNum = fclip(pPts, nPt, 0, 0, m_pXSurface->width, m_pXSurface->height, vPts, false);
// 		if (nNum>1)
// 			XDrawRoad::Draw2DRoad(m_pPolygon, (BYTE*)m_pXSurface->pixel, m_pXSurface->pitch, (POINT*)(&vPts[0]), nNum, (float)nWid, Color(clrInline), Color(clrOutline), bOutline, bButtCap);
	}
}

void CXDraw::Draw3DRoad(POINT *pPts, int nPt, int nWid, COLORREF clrInline, COLORREF clrOutline, bool bOutline, bool bButtCap)
{
	if (m_pPolygon && m_pXSurface && pPts)
	{
		RECT rcClip = {0, 0, m_pXSurface->width, m_pXSurface->height};
		vector<VPOINT> vPolylines;
		if (CMyClip::ClipPolyline(rcClip, pPts, nPt, vPolylines))
		{
			vector<VPOINT>::iterator itEnd = vPolylines.end();
			int nPt = 0;
			for (vector<VPOINT>::iterator it = vPolylines.begin(); it!=itEnd; it++)
			{
				nPt = it->size();
				if (nPt>1)
					XDrawRoad::Draw3DRoad(m_pPolygon, (BYTE*)m_pXSurface->pixel, m_pXSurface->pitch, it->data(), nPt, (float)nWid, Color(clrInline), Color(clrOutline), bOutline, bButtCap);
			}
		}

// 		XDrawRoad::Draw3DRoad(m_pPolygon, (BYTE*)m_pXSurface->pixel, m_pXSurface->pitch, pPts, nPt, (float)nWid, Color(clrInline), Color(clrOutline), bOutline, bButtCap);

// 		vector<int> vPts; vPts.reserve(2*nPt);
// 		int nNum = fclip(pPts, nPt, 0, 0, m_pXSurface->width, m_pXSurface->height, vPts, false);
// 		if (nNum>1)
// 			XDrawRoad::Draw3DRoad(m_pPolygon, (BYTE*)m_pXSurface->pixel, m_pXSurface->pitch, (POINT*)(&vPts[0]), nNum, (float)nWid, Color(clrInline), Color(clrOutline), bOutline, bButtCap);
	}
}

#include "./XDrawRoad/src/SubPolygon.cpp"
#include "./XDrawRoad/src/PolygonVersionF.cpp"
#include "./XDrawRoad/src/XDrawRoad.cpp"

