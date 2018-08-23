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

#if !defined(max)
#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))
#endif//#if !defined(max)

#include "Defs.h"
#include <vector>

unsigned julery_isqrt(unsigned long val);

using namespace std;
typedef struct _VPOINT
{
	vector<POINT> vPts;

	void push_back(const POINT& pt)
	{
		vPts.push_back(pt);
	}

	void clear()
	{
		vPts.clear();
	}

	int size() const
	{
		return (int)vPts.size();
	}

	POINT* data()
	{
		if (vPts.size()>0)
			return (&vPts[0]);
		return NULL;
	}
} VPOINT;

//////////////////////////////////////////////////////////////////////////////
//
// PixelShader - Use to create your own raster operations
//
///////////////////////////////////////////////////////////////////////////////

#define SIZE_TRACKER 3
typedef struct XSurface XSurface;

#ifdef UNICODE
#define SaveImage SaveImageW
#else
#define SaveImage SaveImageA
#endif//#ifdef UNICODE

class PolygonVersionF;

class CXDraw
{
public:
	CXDraw();
	CXDraw(int w, int h);
	virtual ~CXDraw();
private:
	PXSurface m_pXSurface;
	PolygonVersionF* m_pPolygon;
#ifdef OS_WIN
	HDC m_hdc;
#endif // OS_WIN

public:
	bool Attach(void* pPixel, int nWidth, int nHeight);
	void Detach();

#ifdef OS_WIN
	void Update(HDC hdc, RECT* pRec=NULL);
	void Blt(HDC hdc, const RECT & rect, int x , int y);
#endif // OS_WIN
	void Clear(const COLORREF& color);	
	void BlitSurface(int destX, int destY, const PXSurface source, const RECT* srcRect_=0, unsigned long colormark=ULONG_MAX, uint8_t alpha=255);
	void BlitSurfaceMask(int nDestX, int nDestY, const PXSurface pSrcSurf, uint8_t* pAlpha, const RECT* pSrcRect=NULL);
	void GradientSurfaceH(int destX, int destY, const PXSurface source, uint8_t alpha1, uint8_t alpha2, const RECT* srcRect=0);
	void Blit(int destX, int destY, const PXSurface source, unsigned long colormark=ULONG_MAX, uint8_t alpha=255);
	PXSurface SetRenderSurface(PXSurface surface);
#ifdef OS_WIN
	HDC GetDC();
#endif // OS_WIN
	void BlitStretchSurface(const RECT& destRect, const PXSurface source, const RECT* srcRect_=0, int colormark=-1, uint8_t alpha=255);
	int GetWidth();
	int GetHeight();
	PXSurface GetRenderSurface();		
	
	//drawing

	void SetPixel(int x,int y,const COLORREF& color);
	void SetPixel(int x,int y,const COLORREF& color,uint8_t alpha);
	
	void DrawHLine(int x0,int y0,int x1,const COLORREF& color);
	void DrawHLine(int x0,int y0,int x1,const COLORREF& color,uint8_t alpha);
	
	void DrawVLine(int x0,int y0,int y1,const COLORREF& color);
	void DrawVLine(int x0,int y0,int y1,const COLORREF& color,uint8_t alpha);
	
	void DrawLine(int x0,int y0,int x1,int y1,const COLORREF& color,bool bAntialias=false);
	void DrawLineEx(int x0,int y0,int x1,int y1,const COLORREF& color,uint8_t alpha,bool bAntialias=false);

	void DrawWideLine(int x0,int y0,int x1,int y1,const COLORREF& color,int w,bool bAntialias=false);
	void DrawWideLineEx(int x0,int y0,int x1,int y1,const COLORREF& color,int w,uint8_t alpha,bool bAntialias=false);
	void DrawWideLine(const POINT& pt1,const POINT& pt2,const COLORREF& color,int w,bool bAntialias=false)	{
		DrawWideLine(pt1.x, pt1.y, pt2.x, pt2.y, color, w, bAntialias);
	}
	void DrawWideLineEx(const POINT& pt1,const POINT& pt2,const COLORREF& color,int w,uint8_t alpha, bool bAntialias=false)	{
		DrawWideLineEx(pt1.x, pt1.y, pt2.x, pt2.y, color, w, alpha, bAntialias);
	}

	void DrawWideOutline(int x0,int y0,int x1,int y1,const COLORREF& color,int wide);
	void DrawWideOutlineEx(int x0,int y0,int x1,int y1,const COLORREF& color,int wide, uint8_t alpha);

	void DrawWideInline(int x0,int y0,int x1,int y1,int wide,const COLORREF& color,const COLORREF& outlinecolor);
	void DrawWideInlineEx(int x0,int y0,int x1,int y1,int wide,const COLORREF& color,const COLORREF& outlinecolor, uint8_t alpha);
	void DrawWideInline(POINT* pts, int npt, int wide, const COLORREF& color, const COLORREF& outlinecolor);
	void DrawWideInlineEx(POINT* pts, int npt, int wide, const COLORREF& color, const COLORREF& outlinecolor,uint8_t alpha);

	void DrawRect(const RECT &rect,const COLORREF& color, int width = 1, uint8_t alpha=255);

	void DrawAACircle(const int& x, const int& y, const int& radius, const COLORREF& color);
	void DrawAACircleEx(const int& x, const int& y, const int& radius, const COLORREF& color,uint8_t alpha);
	void DrawCircle(const POINT& pt, const int& radius, const COLORREF& color)	{
		DrawAACircle(pt.x, pt.y, radius, color);
	}
	void DrawCircleEx(const POINT& pt, const int& radius, const COLORREF& color, uint8_t alpha)	{
		DrawAACircleEx(pt.x, pt.y, radius, color, alpha);
	}

	void FillAACircle(const int& x, const int& y, const int& radius, const COLORREF& color);
	void FillAACircleEx(const int& x, const int& y, const int& radius, const COLORREF& color, uint8_t alpha);
	void FillCircle(const POINT& pt, const int& radius, const COLORREF& color)	{
		FillAACircle(pt.x, pt.y, radius, color);
	}
	void FillCircleEx(const POINT& pt, const int& radius, const COLORREF& color, uint8_t alpha)	{
		FillAACircleEx(pt.x, pt.y, radius, color, alpha);
	}

	void FillRect(const RECT &rect,const COLORREF& color,uint8_t alpha=255);
	void FillRect(const int &x0,const int &y0,const int &x1,const int &y1,const COLORREF& color,uint8_t alpha=255);

	// Ve outline bi loi voi duong nam ngang
	// Voi duong ngang thi lai to mau
	void DrawAAPolyOutLine(POINT *points, int cnt, int w,const COLORREF& color);
	void DrawAAPolyOutLineEx(POINT *points, int cnt, int w,const COLORREF& color,uint8_t alpha);
	void DrawAAPolyInline(POINT *points, int cnt, int w,const COLORREF& color,const COLORREF& coloroutline );
	void DrawAAPolyInlineEx(POINT *points, int cnt, int w,const COLORREF& color,const COLORREF& coloroutline ,uint8_t alpha);

	void DrawLineFocus(const POINT &pt1, const POINT &pt2);
	void DrawLineFocus(int x0,int y0,int x1,int y1);

	void DrawSnap(const POINT &pt, int nLenth = 20);

	void DrawRectFocus( const POINT &pt1, const POINT &pt2);
	void DrawRectFocus( const RECT &rec);

	void DrawTracker(const POINT& pt, int size=SIZE_TRACKER);
	void DrawPolyTracker(POINT points[], int iPointCount, int size=SIZE_TRACKER);

	void DrawLine(const POINT &pt1, const POINT &pt2, const COLORREF& color, const int pattern[], const int np, bool bAntialias);
	void DrawLineEx(const POINT &pt1, const POINT &pt2, const COLORREF& color, const int pattern[], const int np, uint8_t alpha, bool bAntialias);
	void DrawWideLine(const POINT &pt1, const POINT &pt2, const COLORREF& color, int w, const int pattern[], const int np, bool bAntialias=true);
	void DrawWideLineEx(const POINT &pt1, const POINT &pt2, const COLORREF& color, int w, const int pattern[], const int np, uint8_t alpha, bool bAntialias=true);

	void DrawRect(const RECT& rec, const COLORREF& color, int w, const int pattern[], const int np);
	void DrawRectEx(const RECT& rec, const COLORREF& color, int w, const int pattern[], const int np, uint8_t alpha);

	void FillRect(const RECT& rec, const COLORREF& cl, int fp);
	void FillRectEx(const RECT& rec, const COLORREF& cl, int fp, uint8_t alpha);

	void DrawRectTracker(const RECT& rec, int size=SIZE_TRACKER);

	void FillEllipse(const POINT& pt, const int& rx, const int& ry, const COLORREF& cl, int fp=1);
	void FillEllipseEx(const POINT& pt, const int& rx, const int& ry, const COLORREF& cl, uint8_t alpha, int fp);
	void DrawEllipseFocus(const POINT& pt, const int& rx, const int& ry);

	void DrawPoly(POINT *pts,int ncnt,const COLORREF& color, bool bPolygon=false, bool bAntialias=true);
	void DrawPolyEx(POINT *pts,int ncnt,const COLORREF& color, uint8_t alpha, bool bPolygon=false, bool bAntialias=true);
	void DrawPolyFocus(POINT *pts,int ncnt, bool bPolygon=false);
	void DrawPoly(POINT *pts,int ncnt,const COLORREF& color, const int pattern[],const int np, bool bPolygon=false);
	void DrawPolyEx(POINT *pts,int ncnt,const COLORREF& color, const int pattern[],const int np, uint8_t alpha, bool bPolygon=false);

	void DrawWidePoly(POINT *pts,int ncnt,const COLORREF& color,int w, bool bPolygon=false,bool bAntialias=true);
	void DrawWidePolyEx(POINT *pts,int ncnt,const COLORREF& color,int w, uint8_t alpha, bool bPolygon=false,bool bAntialias=true);
	void DrawWidePoly(POINT *pts,int ncnt,const COLORREF& color,int w, const int pattern[],const int np, bool bPolygon=false);
	void DrawWidePolyEx(POINT *pts,int ncnt,const COLORREF& color,int w, const int pattern[],const int np, uint8_t alpha,bool bPolygon=false);
	
	void DrawCallout(const RECT& rec, const POINT& pt, const COLORREF& color, const COLORREF& colorLine=RGB(0,0,0));

	void FillPoly(POINT *pt, int n, const COLORREF& color, int fp=1, bool bClip=true, uint8_t alpha=255);
	void FillPolyAA(POINT *pt, int n, const COLORREF& color);
	void FillPolyEx(POINT *pt, int n, const COLORREF& color, int fp=1, bool bClip=true, uint8_t alpha=255);
	void FillPoly(POINT* pPts, int nPt, PXSurface pSrcSurface, uint8_t alpha=255);
#ifdef OS_WIN
	void FillPoly(POINT* pPts, int nPt, HBITMAP hBm, uint8_t alpha=255);
	void FillPoly(POINT* pPts, int nPt, int iResourceID, uint8_t alpha=255);
#endif // OS_WIN

	void DrawRoadPoly(POINT *pts,int ncnt,const COLORREF& color, bool bPolygon=false, bool bAntialias=true);
	void DrawRoadPolyEx(POINT *pts,int ncnt,const COLORREF& color, uint8_t alpha, bool bPolygon=false, bool bAntialias=true);
	void FillRoadPoly(POINT *pt, int n, const COLORREF& color, int fp=1, bool bClip=true, uint8_t alpha=255);
	void FillRoadPolyEx(POINT *pt, int n, const COLORREF& color, uint8_t alpha=255);	// Fill poly co antilias
	
	void DrawEllipse(const POINT& pt, const int& rx, const int& ry, const COLORREF& cl,  int iWidth=1);
	void DrawEllipseEx(const POINT& pt, const int& rx, const int& ry, const COLORREF& cl, uint8_t alpha,  int iWidth=1);
	void DrawEllipse(const POINT& pt, const int& rx, const int& ry, const COLORREF& cl, int iWidth, const int pattern[], const int np);
	void DrawEllipseEx(const POINT& pt, const int& rx, const int& ry, const COLORREF& cl, int iWidth, const int pattern[], const int np, uint8_t alpha);

	void DrawCircleFocus(const POINT& pt, const int& iRadius);

	void FillPolyPolyMcDowell( POINT vertex_vector[], int vertex_cnt[], int nvertex, const COLORREF& cl, int fp=1, uint8_t alpha=255);
	void FillPolyPolyMcDowellAA(POINT vertex_vector[], int vertex_cnt[], int nvertex, const COLORREF& cl);

	void Draw3DPolygon(POINT *pts, const int& npt, const int& iHeight, const COLORREF& clShadow, const COLORREF& clTopFill, const COLORREF& clLineFront, const COLORREF& clLineBack);
	void Draw3DPolygonEx(POINT *pts, const int& npt, const int& iHeight, const COLORREF& clShadow, const COLORREF& clTopFill, const COLORREF& clLineFront, const COLORREF& clLineBack, uint8_t alpha);
	void Draw3DPolyPolygon(POINT vertex_vector[], int vertex_cnt[], int nvertex, const int& iHeight, const COLORREF& clShadow, const COLORREF& clTopFill, const COLORREF& clLineFront, const COLORREF& clLineBack);
	void Draw3DPolyPolygonEx(POINT vertex_vector[], int vertex_cnt[], int nvertex, const int& iHeight, const COLORREF& clShadow, const COLORREF& clTopFill, const COLORREF& clLineFront, const COLORREF& clLineBack, uint8_t alpha);

	void DrawPolyPattern(POINT *pts, int ncnt, PXSurface pSrcSurface, bool bPolygon=false);
#ifdef OS_WIN
	void DrawPolyPattern(POINT *pts, int ncnt, int iResourceID, bool bPolygon=false);
	void DrawPolyPattern(POINT *pts, int ncnt, HBITMAP hBm, bool bPolygon=false);
#endif // OS_WIN
	void DrawCompass(const POINT& pt, const int& iSz, const int& iLx, const int& iLy);
	void DrawPolyInside(POINT *pts, int iCount, const int& iWidth, const COLORREF& colorLine, const COLORREF& colorFill, const int& nInsideHatchWidth, const int& nCellSize);
	void DrawPolygonEx(const POINT *pt, int np, int w, DWORD color);

	// Ve mui ten theo polyline
	// nArrowLen: Do dai mui ten - nArrowOff: Khoang cach cac mui ten
	// nArrHeadLen: Do dai phan dau mui ten, do rong phan mui ten
	void DrawArrowOnPolyline(POINT* pPts, int nPt, bool bBack, uint8_t alpha = 255, const COLORREF clrArrow = RGB(128, 0, 0), int nArrowLen = 12, int nArrowOff = 250, int nArrHeadLen = 3, int nArrHeadWid = 2);
	void DrawArrowOnRoad(POINT* pPts, int nPt, bool bBack, uint8_t alpha = 255, const COLORREF& clrArrow = RGB(128, 0, 0), int nArrowLen = 12, int nArrowOff = 250, int nArrHeadLen = 3, int nArrHeadWid = 2);
	void DrawArrow(POINT *pPts, int nPt, int nArrowWid, uint8_t alpha, const COLORREF& clrArrow, int nArrHeadLen, int nArrHeadWid, bool bHaveTail=false);

	// Ve theo cach moi
	void InitRoadDraw();
	void CloseRoadDraw();
	static float Get3DRoadWidth(const POINT& pt, int nWid);
	static bool Get3DRoadWidth(const POINT& pt1, const POINT& pt2, int nWid, float& dWid1, float& dWid2);
	void Draw2DRoad(POINT *pPts, int nPt, int nWid, COLORREF clrInline, COLORREF clrOutline, bool bOutline, bool bButtCap=false);
	void Draw3DRoad(POINT *pPts, int nPt, int nWid, COLORREF clrInline, COLORREF clrOutline, bool bOutline, bool bButtCap=false);

	PXSurface CloneSurface(PXSurface xsurface=NULL);
	void CopyToSurface(PXSurface dest);
	static void CloseSurface(PXSurface surface);
#ifdef OS_WIN
	static PXSurface CreateSurface(HBITMAP hbmp);
	static HDC CreateSurfaceDC(PXSurface xsurface);	
#endif // OS_WIN
	static PXSurface CreateSurface(int w, int h);
	static int GetSurfaceWidth(PXSurface xsurface);
	static int GetSurfaceHeight(PXSurface xsurface);
	static int GetSurfacePitch(PXSurface xsurface);
	static void* GetSurfaceBuffer(PXSurface xsurface);
	static bool SaveImageA( const PXSurface const_surface, const char* filename );
	static bool SaveImageW( const PXSurface const_surface, const wchar_t* filename );
};

