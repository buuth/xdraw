// Algorithm.h: interface for the Algorithm class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALGORITHM_H__05E8B346_407A_4ADA_8F1E_0916D2BE7EDD__INCLUDED_)
#define AFX_ALGORITHM_H__05E8B346_407A_4ADA_8F1E_0916D2BE7EDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//Phuc

class Algorithm  
{
public:
		
	Algorithm(void);
	~Algorithm(void);
	bool LineIntersection(POINT A, POINT B, POINT C, POINT D);
	bool IsIntersection(const POINT& A, const POINT& B, const POINT& C, const POINT& D);
	bool IsLineintersect(const POINT& A, const POINT& B, const POINT& C, const POINT& D) const;
	POINT *SimilarityPolygon(POINT *Pt, int d);
	void EliminateNearPoint(POINT *Pt, int d);
	void EliminateUnusedPoint(POINT *Pt);
	void EliminateAngle(POINT *Pt);
	void EliminateOutsidePoint(POINT *InsidePolygon,int InsideSize,POINT *OutSidePolygon,int OutsideSize);
	bool PointBelongLine(POINT ptL1,POINT ptL2,POINT pt);
	bool IsPointInPoly(POINT *polygon, POINT Pt,int Polysize);
	bool IsPolyInsidePoly(POINT *polygon,POINT *polygon1,int Polysize);
	bool IsPolyInsideRec(POINT *polygon,POINT *polyRec);
	long DoubleToLong(double num);
	void SetPolygonwiseclock(POINT *Pt);
	double GetPolyPerimeter(POINT *Polygon, int size);
	double PolygonArea(POINT *polygon);
	double GetPolygonArea(POINT *Polygon,int size);
	double GetTriangleArea(POINT *Ptriangle);
	POINT PolygonCentroid(POINT *polygon);
	POINT *PointBoundary(POINT ptCentre,int d);
	float GetAngleBetween3points(POINT p1,POINT p2,POINT p3);

	void GetSimilarity(const POINT& Pt1, const POINT& Pt2, POINT *Pt);
	void SetDistance(int d);
	void SetSize(int m_s);
	void ZoomInPoly(POINT *polygon,int nScale);
	void ZoomOutPoly(POINT *polygon,int nScale);
	//Eclipse
	POINT *EclipseIntersection();
	void SetEclipse(int x0,int y0,int rx,int ry);
	void SetRectangle(int r,int b);
	bool CheckValue();
	void DecreaseValue();
	double Distance(const POINT& Pt1, const POINT& Pt2);
	double PointtoLineDistance(const POINT& Pt, const POINT& PL0, const POINT& PL1) const;
	POINT m_ptCurP1;
	POINT m_ptCurP2;
	POINT m_PtIntersion;
	POINT *m_PtEIntersection;
	POINT *m_PtResult;
	POINT *m_PtRec;
	int m_Scale;
	int m_dist;
	int m_Polysize;
	bool m_bIncrease;
	double m_ptEclipseX_x;
	double m_ptEclipseX_y;
	double m_ptEclipseR_x;
	double m_ptEclipseR_y;
	double m_ptRectRB_x;
	double m_ptRectRB_y;
	
};

#endif // !defined(AFX_ALGORITHM_H__05E8B346_407A_4ADA_8F1E_0916D2BE7EDD__INCLUDED_)
