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
#include "MyClip.h"

CMyClip::CMyClip()
{
}

CMyClip::~CMyClip()
{
}

inline int CMyClip::EncodeArea(const RECT& rcView, const POINT& pt)
{
	int nRet = 0;
	if (pt.x<rcView.left)
		nRet |= LEFTAREA_CODE;
	if (pt.x>rcView.right)
		nRet |= RIGHTAREA_CODE;
	if (pt.y<rcView.top)
		nRet |= TOPAREA_CODE;
	if (pt.y>rcView.bottom)
		nRet |= BOTTOMAREA_CODE;
	
	return nRet;
}

inline int CMyClip::EncodeArea(const RECT& rcView, double x, double y)
{
	int nRet = 0;
	if (x<rcView.left)
		nRet |= LEFTAREA_CODE;
	if (x>rcView.right)
		nRet |= RIGHTAREA_CODE;
	if (y<rcView.top)
		nRet |= TOPAREA_CODE;
	if (y>rcView.bottom)
		nRet |= BOTTOMAREA_CODE;
	
	return nRet;
}

bool CMyClip::ClipLine(const RECT& rcView, POINT& pt1, POINT& pt2)
{
	int nAreaCode1 = EncodeArea(rcView, pt1);
	int nAreaCode2 = EncodeArea(rcView, pt2);
	int nAreaCode;
	
	// Tinh do doc duong thang
	double m = 0;
	if (pt2.x^pt1.x)
		m = ((double)(pt2.y-pt1.y))/(pt2.x-pt1.x);
	POINT pt0 = pt1;
	double x, y;

	bool bDone = false;
	bool bAccept = false;
	do 
	{
		if (!(nAreaCode1 | nAreaCode2))
		{
			// Ca hai diem nam trong cua so
			bDone = true;
			bAccept = true;
		}
		else if (nAreaCode1 & nAreaCode2)
		{
			// Ca hai diem nam mot phia cua so
			bDone = true;
		}
		else
		{
			// It nhat co mot diem o ngoai, lay no
			if (nAreaCode1)
				nAreaCode = nAreaCode1;
			else
				nAreaCode = nAreaCode2;

			// Tinh giao diem
			if (nAreaCode & LEFTAREA_CODE)
			{
				x = rcView.left;
				y = pt0.y + m*(x-pt0.x);
			}
			else if (nAreaCode & RIGHTAREA_CODE)
			{
				x = rcView.right;
				y = pt0.y + m*(x-pt0.x);
			}
			else if (nAreaCode & TOPAREA_CODE)
			{
				y = rcView.top;
				if (pt2.x^pt1.x)
					x = pt0.x+(y-pt0.y)/m;
				else
					x = pt0.x;
			}
			else if (nAreaCode & BOTTOMAREA_CODE)
			{
				y = rcView.bottom;
				if (pt2.x^pt1.x)
					x = pt0.x+(y-pt0.y)/m;
				else
					x = pt0.x;
			}

			// Cap nhat lai giao diem
			if (nAreaCode ^ nAreaCode2)
			{
				pt1.x = x;
				pt1.y = y;
				nAreaCode1 = EncodeArea(rcView, x, y);
			}
			else
			{
				pt2.x = x;
				pt2.y = y;
				nAreaCode2 = EncodeArea(rcView, x, y);
			}
		}
	} while (!bDone);

	return bAccept;
}

bool CMyClip::ClipLine(const RECT& rcView, const POINT& pt1, const POINT& pt2, POINT& ptClip1, POINT& ptClip2)
{
	ptClip1 = pt1;
	ptClip2 = pt2;
	return ClipLine(rcView, ptClip1, ptClip2);
}

int CMyClip::ClipPolyline(const RECT& rcView, const POINT* pPts , int nPt, vector<VPOINT>& vPolylines, bool bClosed)
{
	VPOINT vPts;
	vPts.vPts.reserve(nPt);
	if (nPt==2)
	{
		POINT pt1, pt2;
		if (ClipLine(rcView, *pPts, *(pPts+1), pt1, pt2))
		{
			vPts.push_back(pt1);
			vPts.push_back(pt2);
			vPolylines.push_back(vPts);
		}
	}
	else if (nPt>2)
	{
		POINT ptOld, pt1, pt2;
		ptOld = pPts[0];
		ptOld.x++;
		for (int i=0; i<nPt-1; i++)
		{
			if (ClipLine(rcView, pPts[i], pPts[i+1], pt1, pt2))
			{
				if (ptOld.x==pt1.x && ptOld.y==pt1.y)
				{
					vPts.push_back(pt2);
					ptOld = pt2;
				}
				else
				{
					if (vPts.size())
						vPolylines.push_back(vPts);
					vPts.clear();
					vPts.push_back(pt1);
					vPts.push_back(pt2);
					ptOld = pt2;
				}
			}
		}
		if (bClosed)
		{
			if (ClipLine(rcView, pPts[nPt-1], pPts[0], pt1, pt2))
			{
				if (ptOld.x==pt1.x && ptOld.y==pt1.y)
				{
					vPts.push_back(pt2);
					ptOld = pt2;
				}
				else
				{
					if (vPts.size())
						vPolylines.push_back(vPts);
					vPts.clear();
					vPts.push_back(pt1);
					vPts.push_back(pt2);
					ptOld = pt2;
				}
			}
		}
		if (vPts.size())
			vPolylines.push_back(vPts);
	}

	return vPolylines.size();
}

int CMyClip::ClipPolyline(const RECT& rcView, const vector<POINT>& vPts, vector<VPOINT>& vPolylines, bool bClosed)
{
	if (vPts.size()>0)
		return ClipPolyline(rcView, &vPts[0], vPts.size(), vPolylines, bClosed);
	else
		return 0;
}

inline bool CMyClip::Inside(const RECT& rcView, const POINT& pt, MYCLIP_EDGETYPE edgetype)
{
	if (edgetype==MYCLIP_EDGELEFT)
		return (pt.x>=rcView.left);
	else if (edgetype==MYCLIP_EDGERIGHT)
		return (pt.x<=rcView.right);
	else if (edgetype==MYCLIP_EDGETOP)
		return (pt.y>=rcView.top);
	else if (edgetype==MYCLIP_EDGEBOTTOM)
		return (pt.y<=rcView.bottom);
	else
		return false;
}

inline void CMyClip::Intersect(const RECT& rect, const POINT& pt1, const POINT& pt2, MYCLIP_EDGETYPE edgetype, POINT& pt)
{
	double m = 0;
	if (pt2.x^pt1.x)
		m = (double)(pt2.y-pt1.y)/(pt2.x-pt1.x);
	if (edgetype==MYCLIP_EDGELEFT)
	{
		pt.x = rect.left;
		pt.y = pt1.y + m*(pt.x-pt1.x);
	}
	else if (edgetype==MYCLIP_EDGERIGHT)
	{
		pt.x = rect.right;
		pt.y = pt1.y + m*(pt.x-pt1.x);
	}
	else if (edgetype==MYCLIP_EDGETOP)
	{
		pt.y = rect.top;
		if (pt2.x^pt1.x)
			pt.x = pt1.x+(pt.y-pt1.y)/m;
		else
			pt.x = pt1.x;
	}
	else if (edgetype==MYCLIP_EDGEBOTTOM)
	{
		pt.y = rect.bottom;
		if (pt2.x^pt1.x)
			pt.x = pt1.x+(pt.y-pt1.y)/m;
		else
			pt.x = pt1.x;
	}
}

inline void CMyClip::ClipEdge(const RECT& rcView, const POINT* pPts , int nPt, MYCLIP_EDGETYPE edgetype, vector<POINT>& vPts)
{
	vPts.clear();
	if (nPt>2)
	{
		POINT pt0, pt, ptIntersect;
		pt0 = pPts[nPt-1];
		for (int i=0; i<nPt; i++)
		{
			pt = pPts[i];
			if (Inside(rcView, pt, edgetype))
			{
				if (!Inside(rcView, pt0, edgetype))
				{
					Intersect(rcView, pt0, pt, edgetype, ptIntersect);
					vPts.push_back(ptIntersect);
				}
				vPts.push_back(pt);
			}	
			else if (Inside(rcView, pt0, edgetype))
			{
				Intersect(rcView, pt0, pt, edgetype, ptIntersect);
				vPts.push_back(ptIntersect);
			}
			pt0 = pt;
		}
	}
}

int CMyClip::ClipPolygon(const RECT& rcView, const POINT* pPts , int nPt, vector<POINT>& vPolygon)
{
	vPolygon.clear();
	vector<POINT> vPts;
	vPts.reserve(nPt+1);
	ClipEdge(rcView, pPts, nPt, MYCLIP_EDGELEFT, vPts);
	ClipEdge(rcView, (vPts.size()>0)?&vPts[0]:NULL, vPts.size(), MYCLIP_EDGERIGHT, vPolygon);
	ClipEdge(rcView, (vPolygon.size()>0)?&vPolygon[0]:NULL, vPolygon.size(), MYCLIP_EDGETOP, vPts);
	ClipEdge(rcView, (vPts.size()>0)?&vPts[0]:NULL, vPts.size(), MYCLIP_EDGEBOTTOM, vPolygon);
	
	return vPolygon.size();
}

int CMyClip::ClipPolygon(const RECT& rcView, const vector<POINT>& vPts, vector<POINT>& vPolygon)
{
	if (vPts.size()>0)
		return ClipPolygon(rcView, &vPts[0], vPts.size(), vPolygon);
	else
		return 0;
}