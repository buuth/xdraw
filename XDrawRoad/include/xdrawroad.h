#ifndef __XDRAW_ROAD_H__
#define __XDRAW_ROAD_H__

namespace XDrawRoad
{
	void InitDraw(PolygonVersionF* &pPolygon, unsigned int aWidth, unsigned int aHeight);
	void CloseDraw(PolygonVersionF* &pPolygon);
	//float Get3DWide(const POINT &pt1, const POINT &pt2, float fWidth);
	float Get3DRoadWidth(const POINT& pt, int nWid);
	bool Get3DRoadWidth(const POINT& pt1, const POINT& pt2, int nWid, float& dWid1, float& dWid2);
	void Draw2DRoad(PolygonVersionF* pPolygon, BYTE* pDisplay, int nPitch, POINT *pPts, int nPt, float fWid, Pixel clrInline, Pixel clrOutline, bool bOutline, bool bButtCap);
	void Draw3DRoad(PolygonVersionF* pPolygon, BYTE* pDisplay, int nPitch, POINT *pPts, int nPt, float fWid, Pixel clrInline, Pixel clrOutline, bool bOutline, bool bButtCap);
	void FillPolyAA(PolygonVersionF* pPolygon, BYTE* pDisplay, int nPitch, POINT *pPts, int nPt, Pixel clrColor);
	void FillPolyPolyMcDowellAA(PolygonVersionF* pPolygon, BYTE* pDisplay, int nPitch, POINT* pPts, int* pVertextIdx, int nVertexCnt, Pixel clrColor);
}

#endif // !__XDRAW_ROAD_H__
