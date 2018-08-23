#pragma once

#include "XDraw.h"

#define		LEFTAREA_CODE			1
#define		RIGHTAREA_CODE			2
#define		TOPAREA_CODE			4
#define		BOTTOMAREA_CODE			8

enum MYCLIP_EDGETYPE
{
	MYCLIP_EDGELEFT,
	MYCLIP_EDGERIGHT,
	MYCLIP_EDGETOP,
	MYCLIP_EDGEBOTTOM
};

// Clip Line, Polyline va Polygon su dung thuat toan Cohen-Sutherland
class CMyClip
{
public:
	CMyClip();
	~CMyClip();

	/********************* Clip Line *********************/
	// Tra ve true neu co doan thang o trong hoac giao voi vung hien thi
	static bool ClipLine(const RECT& rcView, POINT& pt1, POINT& pt2);
	static bool ClipLine(const RECT& rcView, const POINT& pt1, const POINT& pt2, POINT& ptClip1, POINT& ptClip2);
	/*****************************************************/
	
	/********************* Clip Polyline *********************/
	// Tra ve so Polyline sau khi Clip
	static int ClipPolyline(const RECT& rcView, const POINT* pPts , int nPt, vector<VPOINT>& vPolylines, bool bClosed = false);
	static int ClipPolyline(const RECT& rcView, const vector<POINT>& vPts, vector<VPOINT>& vPolylines, bool bClosed = false);
	/*********************************************************/

	/********************* Clip Polygon *********************/
	// Tra ve so dinh moi trong Polygon
	static int ClipPolygon(const RECT& rcView, const POINT* pPts , int nPt, vector<POINT>& vPolygon);
	static int ClipPolygon(const RECT& rcView, const vector<POINT>& vPts, vector<POINT>& vPolygon);
	/********************************************************/

private:
	// Ham gan ma vung cho diem
	static inline int EncodeArea(const RECT& rcView, const POINT& pt);
	static inline int EncodeArea(const RECT& rcView, double x, double y);

	static inline void ClipEdge(const RECT& rcView, const POINT* pPts , int nPt, MYCLIP_EDGETYPE edgetype, vector<POINT>& vPts);
	static inline bool Inside(const RECT& rcView, const POINT& pt, MYCLIP_EDGETYPE edgetype);
	static inline void Intersect(const RECT& rect, const POINT& pt1, const POINT& pt2, MYCLIP_EDGETYPE edgetype, POINT& pt);
};
