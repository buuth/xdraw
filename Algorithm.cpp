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

// Algorithm.cpp: implementation of the Algorithm class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Algorithm.h"
#include <math.h>

 
#define RTD 57.29577951 // 180/pi

static float ANGLE_TABLE_COS[]={1.000000f,
0.999848f,0.999391f,0.998630f,0.997564f,0.996195f,0.994522f,0.992546f,0.990268f,0.987688f,0.984808f,
0.981627f,0.978148f,0.974370f,0.970296f,0.965926f,0.961262f,0.956305f,0.951057f,0.945519f,0.939693f,
0.933580f,0.927184f,0.920505f,0.913545f,0.906308f,0.898794f,0.891007f,0.882948f,0.874620f,0.866025f,
0.857167f,0.848048f,0.838671f,0.829038f,0.819152f,0.809017f,0.798636f,0.788011f,0.777146f,0.766044f,
0.754710f,0.743145f,0.731354f,0.719340f,0.707107f,0.694658f,0.681998f,0.669131f,0.656059f,0.642788f,
0.629320f,0.615661f,0.601815f,0.587785f,0.573576f,0.559193f,0.544639f,0.529919f,0.515038f,0.500000f,
0.484810f,0.469472f,0.453991f,0.438371f,0.422618f,0.406737f,0.390731f,0.374607f,0.358368f,0.342020f,
0.325568f,0.309017f,0.292372f,0.275637f,0.258819f,0.241922f,0.224951f,0.207912f,0.190809f,0.173648f,
0.156434f,0.139173f,0.121869f,0.104528f,0.087156f,0.069757f,0.052336f,0.034899f,0.017452f,0.000000f};

double Tacos(double dCos)
{
	//assert(dCos >= -1 && dCos <= 1);
	int sign = 1;
	if(dCos < 0) {
		dCos = -dCos;
		sign = -1;
	}
	int i;
	for(i=1;i<90;i++)	{
		if(ANGLE_TABLE_COS[i] < dCos)
			break;
	}
	double dAngle = i - (dCos - ANGLE_TABLE_COS[i])/ (ANGLE_TABLE_COS[i-1] - ANGLE_TABLE_COS[i]);
	if (sign<0)
		dAngle = 180-dAngle;
	return dAngle;
}

typedef struct fvector_s
{
	float dx,dy;
} fvector_t;

typedef struct fvertex_s
{
	float x,y;
} fvertex_t;

float fdot(const fvector_t *v1, const fvector_t *v2 );
float fAngleDeg(fvector_t *v1, fvector_t *v2 );
float fVectorLength(const fvector_t *v);
void fVectorNormalize(fvector_t *v);

// returns the dot product of two vectors
float fdot(const fvector_t *v1, const fvector_t *v2 )
{return( (v1->dx*v2->dx) + (v1->dy*v2->dy) );}

// returns the length of a vector
float fVectorLength(const fvector_t *v)
{return (float)sqrt(fdot(v, v));}

//return the angle in degrees between two lines
float fAngleDeg(fvector_t *v1, fvector_t *v2 )
{
	fVectorNormalize(v1);
	fVectorNormalize(v2);
	
	double angle = fdot(v1,v2);
	if (angle>1)
		angle = 1;
	else if (angle<-1)
		angle = -1;
	return (float)Tacos(angle);
	//return (acos(fdot(v1,v2)) * RTD);
}

// Makes a unit vector, a vector whose length is 1.
void fVectorNormalize(fvector_t *v)
{
	float temp = 1;
	
	float d = fVectorLength(v);
	if (d!=0)
		temp = 1 / fVectorLength(v);

	v->dx = v->dx * temp;
	v->dy = v->dy * temp;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*
* Summary:     Constructor function for Algorithm class
*				Initialization all necessary value for using later
* Parameters:  void
* Return:      
* Created:    21/09/2008 
* Modified : 
* Author    : Phuc Tran
*/
Algorithm::Algorithm()
{
	m_ptCurP1.x = m_ptCurP1.y = 0;
	m_ptCurP2.y = m_ptCurP2.y = 0;
	m_PtIntersion.x = m_PtIntersion.y = 0;
	
	m_PtResult = new POINT[2];
	m_PtEIntersection = new POINT[4]; //This default value is used for testing only
	m_PtRec = new POINT[4];

	m_Scale = 1;
	m_dist = 1;
	m_Polysize = 0;
	m_bIncrease = false;
	m_ptEclipseX_x = m_ptEclipseX_y = 0;
	m_ptEclipseR_x = m_ptEclipseR_y = 0;
	m_ptRectRB_x = m_ptRectRB_y = 0;
}


/*
* Summary:     Destructor function for Algorithm class
*				Clean up and to free the resources
* Parameters:  void
* Return:      
* Created:   21/09/2008
* Modified : 
* Author    : Phuc Tran
*/
Algorithm::~Algorithm()
{
	if(m_PtResult)
		delete []m_PtResult;	
	if(m_PtEIntersection)
		delete []m_PtEIntersection;
	if(m_PtRec)
		delete []m_PtRec;
}


/*
 * Summary:     Determines the intersection point of the line defined by points A and B with the
 *				line defined by points C and D
 * Parameters:  The list of point which make the input polygon
 * Return:      Returns YES if the intersection point was found, and stores that point in m_PtIntersion
 *				Returns NO if there is no determinable intersection point, in which case m_PtIntersion will
 *				be unmodified.
 * Created:     03/09/2008 
 * Modified : 
 * Author    : Phuc Tran
 */

bool Algorithm::LineIntersection(POINT A, POINT B, POINT C, POINT D) 
{
	double  distAB, theCos, theSin, newX, ABpos ;
	POINT PtTemp = C;
	 //  Fail if either line is undefined.
	 
	if (A.x==B.x && A.y==B.y || C.x==D.x && C.y==D.y)
	{
		m_PtIntersion.x=A.x;
		m_PtIntersion.y=B.y;		
		return false;
	}
	else if (B.x==C.x && B.y==C.y) //B is the same with C
	{
		m_PtIntersion.x=B.x;
		m_PtIntersion.y=B.y;		
		return false;

	}
	else if ( A.x==B.x && B.x==C.x)
	{
		m_PtIntersion.x=C.x;
		m_PtIntersion.y=C.y;		
		return false;

	}
	else if ( A.y==B.y &&B.y==C.y)
	{
		m_PtIntersion.x=C.x;
		m_PtIntersion.y=C.y;		
		return false;
	}


	//  (1) Translate the system so that point A is on the origin.
	B.x-=A.x; B.y-=A.y;
	C.x-=A.x; C.y-=A.y;
	D.x-=A.x; D.y-=A.y;
	
	//  Discover the length of segment A-B.
	//distAB=sqrt(float(B.x*B.x+B.y*B.y));
	distAB=sqrt(double(double(B.x)*double(B.x)+double(B.y)*double(B.y)));
	
	//  (2) Rotate the system so that point B is on the positive X axis.
	theCos = double(B.x)/double(distAB);
	theSin = double(B.y)/double(distAB);
	newX = double(C.x)*theCos+double(C.y)*theSin;
	C.y = (long)(double(C.y)*theCos-double(C.x)*theSin);
	C.x = (long)newX;
	newX = double(D.x)*theCos+double(D.y)*theSin;
	D.y = (long)(double(D.y)*theCos-double(D.x)*theSin);
	D.x = (long)newX;

	//  Fail if the lines are parallel.
	if (C.y==D.y) 
	{
		m_PtIntersion=PtTemp;
		return false;
	}
	
	//  (3) Discover the position of the intersection point along line A-B.
	ABpos=D.x+(C.x-D.x)*double(D.y)/double(D.y-C.y);
	//  (4) Apply the discovered position to line A-B in the original coordinate system.
	m_PtIntersion.x=DoubleToLong(A.x+ABpos*theCos);
	m_PtIntersion.y=DoubleToLong (A.y+ABpos*theSin);

	//  Success.
	return true; 
}

/*
* Summary:     Calculate the distance from point to line
*				
* Parameters:  pL0,pL1 which make the line, the point Pt
* Return:      True if the 2 lines intersect , otherwise, return false
* Created:    22/09/2008 
* Modified : 
* Author    : Phuc Tran
*/
double Algorithm::PointtoLineDistance(const POINT& Pt, const POINT& PL0, const POINT& PL1) const
{
	double A,B,C,D,E,F,G,H;
	A=double(PL0.y-PL1.y)*Pt.x;
	B=double(PL1.x-PL0.x)*Pt.y;
	C=double(PL0.x)*PL1.y-double(PL1.x)*PL0.y;
	D=double(PL1.x-PL0.x)*(PL1.x-PL0.x);
	E=double(PL1.y-PL0.y)*(PL1.y-PL0.y);
	F=D+E;
	G=sqrt(F);
	H=(A+B+C)/G;
	return H;
}

void NormalizeRect(RECT &rect)
{
	int temp;

	if (rect.left > rect.right) {
		temp = rect.left;
		rect.left = rect.right;
		rect.right = temp;
	}
	
	if (rect.top > rect.bottom) {
		temp = rect.top;
		rect.top = rect.bottom;
		rect.bottom = temp;
	}

}
BOOL PtInRect(const RECT& r, const double &x, const double &y)
{
	return (x >= r.left &&
		y >= r.top &&
		x <= r.right &&
		y <= r.bottom);
}
bool IsIntersectedWith(RECT r1,RECT r2)
{
	
	NormalizeRect(r1);
	NormalizeRect(r2);
	if(((r1.top <= r2.bottom && r1.top >= r2.top) || (r1.bottom <= r2.bottom && r1.bottom >= r2.top)) 
		&& r1.right >= r2.right && r1.left <= r2.left)
		return true;
	if(((r2.top <= r1.bottom && r2.top >= r1.top) || (r2.bottom <= r1.bottom && r2.bottom >= r1.top))
		&& r2.right >= r1.right && r2.left <= r1.left)
		return true;
	
	if(PtInRect(r2,r1.left, r1.top) || PtInRect(r2,r1.right, r1.top)
		|| PtInRect(r2,r1.right, r1.bottom) || PtInRect(r2,r1.left, r1.bottom))
		return true;
	if(PtInRect(r1,r2.left, r2.top) || PtInRect(r1,r2.right, r2.top)
		|| PtInRect(r1,r2.right, r2.bottom) || PtInRect(r1,r2.left, r2.bottom))
		return true;
	return false;

}
/*
* Summary:     Check whether 2 line intersect or not
*				
* Parameters:  point A, B which make the first line,point C, D which make the second line
* Return:      True if the 2 lines intersect , otherwise, return false
* Created:    28/12/2008 
* Modified : 
* Author    : Phuc Tran
*/
bool Algorithm::IsLineintersect(const POINT& A, const POINT& B, const POINT& C, const POINT& D) const
{
	RECT r1,r2;
	if(A.x>B.x)
	{
		r1.left=B.x;
		r1.right=A.x;
	}
	else
	{
		r1.left=A.x;
		r1.right=B.x;

	}
	if(A.y>B.y)
	{
		r1.top=B.y;
		r1.bottom=A.y;

	}
	else
	{
		r1.top=A.y;
		r1.bottom=B.y;

	}

	if(C.x>D.x)
	{
		r2.left=D.x;
		r2.right=C.x;
	}
	else
	{
		r2.left=C.x;
		r2.right=D.x;

	}
	if(C.y>D.y)
	{
		r2.top=D.y;
		r2.bottom=C.y;

	}
	else
	{
		r2.top=C.y;
		r2.bottom=D.y;

	}

	return !(r2.left>r1.right
		||r2.right<r1.left
		||r2.top>r1.bottom
		||r2.bottom<r1.top);
}
/*
* Summary:     Get the intersection point of two lines
*				
* Parameters:  point A, B which make the first line,point C, D which make the second line
* Return:      True if the 2 lines intersect , otherwise, return false
* Created:    22/09/2008 
* Modified : 
* Author    : Phuc Tran
*/


bool Algorithm::IsIntersection(const POINT& A, const POINT& B, const POINT& C, const POINT& D)
{
	if (C.x==D.x &&C.y==D.y)
		return false;
	if (A.x==D.x&&A.y==C.y)
		return false;
	if (B.x==C.x&&B.y==C.y)
		return false;
	if (A.x==D.x&&A.y==D.y)
		return false;
		
	if (LineIntersection(A,B,C,D))
	{
		if (PointBelongLine(A,B,m_PtIntersion))
			if (PointBelongLine(C,D,m_PtIntersion))
				return true;
	}
	
	return false;
}



/*
* Summary:     Check whether a point belong to a polygon or not
*				
* Parameters:  Poly: a list of point, the point pt
* Return:   return True if the point Pt is inside the polygon, or
			False if it is not.  If the point is exactly on the edge of the polygon,
			then the function may return true or false.
* Created:    22/09/2008 
* Modified : 
* Author    : Phuc Tran
*/
bool Algorithm::IsPointInPoly(POINT *polygon, POINT Pt,int Polysize)
{
	int i, j;
	bool c = false;
	for (i = 0, j = Polysize-1; i < Polysize; j = i++) {
		if ( ((polygon[i].y>Pt.y) != (polygon[j].y>Pt.y)) &&
			(Pt.x < (polygon[j].x  -polygon[i].x ) * (Pt.y- polygon[i].y) / (polygon[j].y-polygon[i].y) +polygon[i].x) )
			c = !c;
	}
	return c;
}


/*
* Summary:     Check whether a point belong to a line or not
*				
* Parameters:  ptL1,ptL2 which make the line, the point pt
* Return:      True if the point pt belong to the line (ptL1,ptL2)
* Created:    22/09/2008 
* Modified : 
* Author    : Phuc Tran
*/
bool Algorithm::PointBelongLine(POINT ptL1,POINT ptL2,POINT pt)
{
	double d=m_dist>0? m_dist/2:-m_dist/2;
	

	if(pt.x>ptL1.x-d &&pt.x<ptL2.x+d || pt.x>ptL2.x-d &&pt.x<ptL1.x+d )
		if(pt.y>ptL1.y-d &&pt.y<ptL2.y+d || pt.y>ptL2.y-d &&pt.y<ptL1.y+d )
			return true;
	return false;
	
	
}


double Algorithm::GetPolygonArea(POINT *Polygon,int size)
{
	int i,j;
	double area = 0;

	for (i=0;i<size;i++) {
		j = (i + 1) % size;
		area += Polygon[i].x * Polygon[j].y;
		area -= Polygon[i].y * Polygon[j].x;
	}

	area /= 2;
	return(area < 0 ? -area : area);
}





/*
* Summary:     calculate triangle area with Heron's formula 
*				
* Parameters:  The list of 3 points which makes the triangle 
* Return:      
* Created:    22/09/2008 
* Modified : 
* Author    : Phuc Tran
*/

double Algorithm::GetTriangleArea(POINT *Ptriangle)
{
	double s1, s2, s3, S, area;
	

	s1=Distance(Ptriangle[0],Ptriangle[1]);
	s2=Distance(Ptriangle[1],Ptriangle[2]);
	s3=Distance(Ptriangle[2],Ptriangle[0]);
	S = (s1 + s2 + s3) / 2;
	area = sqrt(abs(S*(S - s1)*(S - s2)*(S - s3)));
    
	return area;
}

/*
* Summary:     Check whether a polygon is inside another polygon
*				
* Parameters:  The first polygon, the second polygon, the size of the polygon
* Return:      True if the polygon1 is inside the polygon2
* Created:    22/09/2008 
* Modified : 
* Author    : Phuc Tran
*/
bool Algorithm::IsPolyInsidePoly(POINT *polygon,POINT *polygon1,int Polysize)
{
	for(int i=0;i<Polysize;i++)
	{
		if(!IsPointInPoly(polygon1,polygon[i],Polysize))
			return false;

	}
	return true;

}

/* 
* Summary:    Eliminate the 2 consecutive points
*			  if the distance between them is small than d
*				
* Parameters:  The list of point which make the polygon, d
* Return:      void
* Created:    10/11/2008 
* Modified : 
* Author    : Phuc Tran
*/

void Algorithm::EliminateNearPoint(POINT *Pt, int d)
{
	POINT * PtResult;
	PtResult=new POINT[m_Polysize];
	int size=m_Polysize;
	int next=0;
	double dist;
	
	int i,j;

	for(i=0;i<size-1;i++)
	{   
		PtResult[next]=Pt[i];
		j=i+1;
		dist = Distance(Pt[i],Pt[j]);
		while(dist < d && j < size-1)
		{
			dist+=Distance(Pt[j],Pt[j+1]);
			Pt[j]=Pt[i];
			j++;			
		}
		i=j-1;
		next++;
	}
	m_Polysize=next; 
	for(i=next;i<size;i++)
		PtResult[i]=PtResult[next-1];

	for(i=0;i<size;i++)
		Pt[i]=PtResult[i];

	delete []PtResult;

}
/*
* Summary:     eliminate some unnecessary points
*				
* Parameters:  The list of point which make the similarity polygon
* Return:      The list of point which make the similarity polygon after elimination unnecessary point
* Created:    22/09/2008 
* Modified : 
* Author    : Phuc Tran
*/

void Algorithm::EliminateUnusedPoint(POINT *Pt)
{
	POINT *FirstPoly;
	POINT *SecondPoly;
	FirstPoly=new POINT[m_Polysize];
	SecondPoly=new POINT[m_Polysize];
	for(int i=0;i<m_Polysize;i++)
	{
		for(int j=i+2;j<m_Polysize-1;j++)
		{
			if(IsLineintersect(Pt[i],Pt[i+1],Pt[j],Pt[j+1]))
			{

			
			if(IsIntersection(Pt[i],Pt[i+1],Pt[j],Pt[j+1]))			
			{
				
				for(int k=0;k<m_Polysize;k++)
				{
					FirstPoly[k]=Pt[k];
					SecondPoly[k]=Pt[k];

				}
				//Make the first polygon
				int i1;
				for(i1=i+1;i1<j+1;i1++)
					FirstPoly[i1]=m_PtIntersion;

				//Make the Second polygon
				for(i1=0;i1<i+1;i1++)
					SecondPoly[i1]=m_PtIntersion;
				for(i1=j+1;i1<m_Polysize;i1++)
					SecondPoly[i1]=m_PtIntersion;

				double s1,s2;				
				
				s1=GetPolygonArea(FirstPoly,m_Polysize);
				s2=GetPolygonArea(SecondPoly,m_Polysize);
				
				
								
				if(s1>s2)
				{
					for(int i2=0;i2<m_Polysize;i2++)
						Pt[i2]=FirstPoly[i2];

				}
				else
				{
					for(int i2=0;i2<m_Polysize;i2++)
						Pt[i2]=SecondPoly[i2];

				}

			//j=m_Polysize-1;
			}//if intersection 
			}//if lineintersection
			
		} 

		if(IsLineintersect(Pt[i],Pt[i+1],Pt[m_Polysize-1],Pt[0]))
		{
		
		if(IsIntersection(Pt[i],Pt[i+1],Pt[m_Polysize-1],Pt[0]))
		{
			for(int k=0;k<m_Polysize;k++)
			{
			FirstPoly[k]=Pt[k];
			SecondPoly[k]=Pt[k];

			}
			//Make the first polygon
			int i1;
			for(i1=i+1;i1<m_Polysize;i1++)
			FirstPoly[i1]=m_PtIntersion;
			//Make the Second polygon
			for(i1=0;i1<i+1;i1++)
			SecondPoly[i1]=m_PtIntersion;
			double s1,s2;

			s1=GetPolygonArea(FirstPoly,m_Polysize);
			s2=GetPolygonArea(SecondPoly,m_Polysize);

			if(s1>s2)
			{
			for(int i2=0;i2<m_Polysize;i2++)
			Pt[i2]=FirstPoly[i2];

			}
			else
			{
			for(int i2=0;i2<m_Polysize;i2++)
			Pt[i2]=SecondPoly[i2];

			}

		}
		}
	}//for i*/
		delete []FirstPoly;
		delete []SecondPoly;

}


/* 
* Summary:    Eliminate some outside point of the similarity polygon with the original polygon 
*				
* Parameters:  The list of point which make the polygon
               The size of the polygon	
* Return:      void
* Created:    10/11/2008 
* Modified : 
* Author    : Phuc Tran
*/

void Algorithm::EliminateOutsidePoint(POINT *InsidePolygon,int InsideSize,POINT *OutSidePolygon,int OutsideSize)
{
	int i;
	int maxnum=100;
	int Numtime=0;
	int Pos[100];
	int PosStart;
	int PosEnd;
	for(i=0;i<maxnum;i++)
		Pos[i]=0;	
	i=0;	

	while(Numtime<5 && i<InsideSize-2)
	{   
		if(!IsPointInPoly(OutSidePolygon,InsidePolygon[i],OutsideSize))
			{ 
				PosStart=i;
				while(!IsPointInPoly(OutSidePolygon,InsidePolygon[i+1],OutsideSize) &&i<InsideSize-2)
					i++;
				PosEnd=i+1;
				if(PosStart!=PosEnd)
				{
					for(int j=PosStart;j<PosEnd;j++)
						InsidePolygon[j]=InsidePolygon[PosEnd];
				}
				else
				{
					InsidePolygon[PosStart]=InsidePolygon[PosEnd];
				}
				Numtime++;
			}
				
		i++;

	}
	if(!IsPointInPoly(OutSidePolygon,InsidePolygon[InsideSize-2],OutsideSize))
	{
		PosEnd=InsideSize-2;
		PosStart=InsideSize-3;
		if(PosStart>=0)
		{
			while(!IsPointInPoly(OutSidePolygon,InsidePolygon[PosStart],OutsideSize) &&PosStart>1)
				PosStart--;
			if(PosStart!=PosEnd-1)
			{
				for(int j=PosEnd;j>PosStart;j--)
					InsidePolygon[j]=InsidePolygon[PosStart];
			}
			else
			{
				InsidePolygon[PosEnd]=InsidePolygon[PosStart];
			}
		}
	}

	if(!IsPointInPoly(OutSidePolygon,InsidePolygon[InsideSize-1],OutsideSize))
	{
		PosEnd=InsideSize-1;
		PosStart=InsideSize-2;
		if(PosStart>=0)
		{
			while(!IsPointInPoly(OutSidePolygon,InsidePolygon[PosStart],OutsideSize) &&PosStart>1)
				PosStart--;
			if(PosStart!=PosEnd-1)
			{
				for(int j=PosEnd;j>PosStart;j--)
					InsidePolygon[j]=InsidePolygon[PosStart];

			}
			else
			{
				InsidePolygon[PosEnd]=InsidePolygon[PosStart];

			}

		}
		
	}
}


/*
* Summary:     Get the Smallest Angle (in degree) between 3 points
*				
* Parameters:  POINT p1,p2,p3
* Return:      
* Created:    27/11/2008 
* Modified : 
* Author    : Phuc Tran
*/

float Algorithm::GetAngleBetween3points(POINT p1,POINT p2,POINT p3)
{
	fvector_t v1,v2; //two vectors that represent our lines.
	//fvertex_t p1,p2,p3,p4; // 4 vertices that represent our lines.
	float angle_between_lines;

	//make two lines (vectors) from these vertices
	v1.dx = p2.x - p1.x;
	v1.dy = p2.y - p1.y;

	v2.dx = p3.x - p1.x;
	v2.dy = p3.y - p1.y;

	//get the smallest angle between them.
	angle_between_lines = fAngleDeg(&v1,&v2);
	return  angle_between_lines;

}

/*
* Summary:     Eliminate some corner with the small degree
*				
* Parameters:  void
* Return:      
* Created:    27/11/2008 
* Modified : 
* Author    : Phuc Tran
*/
void Algorithm::EliminateAngle(POINT *Pt)
{
	int i;
	float degree;
	POINT p1,p2,p3;


	for(i=0;i<m_Polysize-2;i++)
	{
		p1=Pt[i];
		p2=Pt[i+1];
		p3=Pt[i+2];
		//Consequence: P2, P1, P3
		degree=GetAngleBetween3points(p2,p1,p3);
		if(degree<10)
			//Eliminate point
		{
			Pt[i+1]=Pt[i];

		}
		else if(degree<45 )
			//eliminate
		{
			if(Distance(p1,p2)<30)
			{
				if(i>0)
					LineIntersection(Pt[i-1],Pt[i],Pt[i+1],Pt[i+2]);
				else				
				{
					if(Pt[m_Polysize-1].x!=Pt[0].x &&Pt[m_Polysize-1].y!=Pt[0].y)
						LineIntersection(Pt[0],Pt[m_Polysize-1],Pt[i+1],Pt[i+2]);
					else
						LineIntersection(Pt[0],Pt[m_Polysize-2],Pt[i+1],Pt[i+2]);


				}
				Pt[i+1]=m_PtIntersion;
			}
			else if(Distance(p2,p3)<30)
			{
				if(i+3<m_Polysize)
					LineIntersection(Pt[i],Pt[i+1],Pt[i+2],Pt[i+3]);
				else
				{
					if(Pt[i+2].x==Pt[0].x &&Pt[i+2].y==Pt[0].y)
						LineIntersection(Pt[i],Pt[i+1],Pt[i+2],Pt[1]);
					else
						LineIntersection(Pt[i],Pt[i+1],Pt[i+2],Pt[0]);

				}				
				Pt[i+1]=m_PtIntersion;


			}


		}
		else if(degree>170 &&Distance(p1,p2)<50 )
		{
			Pt[i+1]=Pt[i];

		}


	}



}

/* 
* Summary:    calculate polygon perimeter 
*				
* Parameters:  The list of point which make the polygon
			   The size of the polygon	
* Return:      void
* Created:    10/11/2008 
* Modified : 
* Author    : Phuc Tran
*/
double Algorithm::GetPolyPerimeter(POINT *Polygon, int size)
{
	double perimeter=0;
	double d;
	for(int i=0;i<size-1;i++)
	{
		d=Distance(Polygon[i],Polygon[i+1]);
		perimeter+=d;
	}
	d=Distance(Polygon[0],Polygon[size-1]);
	perimeter+=d;	
	return perimeter;
}

/*

* Summary:     Set the direction of the polygon (wiseclock or otherwise)
*				
* Parameters:  The list of point which make the polygon
* Return:      void
* Created:    21/10/2008 
* Modified : 
* Author    : Phuc Tran
*/
void Algorithm::SetPolygonwiseclock(POINT *Pt)
{
	POINT tmpP[4];
	int Inside=0;
	int OutSide=0;

	GetSimilarity(Pt[0],Pt[1],Pt);
	tmpP[0]=m_ptCurP1;
	tmpP[1]=m_ptCurP2;

	for(int i=0;i<m_Polysize-2;i++)
	{
		GetSimilarity(Pt[i+1],Pt[i+2],Pt);
		tmpP[2]=m_ptCurP1;
		tmpP[3]=m_ptCurP2;
		LineIntersection(tmpP[0],tmpP[1],tmpP[2],tmpP[3]);
		if(m_dist>0)
		{
			if(!IsPointInPoly(Pt,m_PtIntersion,m_Polysize))
				OutSide++;
			else
				Inside++;
		}
		else
		{
			if(IsPointInPoly(Pt,m_PtIntersion,m_Polysize))
				OutSide++;
			else
				Inside++;

		}

		tmpP[0]=m_ptCurP1;
		tmpP[1]=m_ptCurP2;
	}
	if(OutSide>Inside)
		m_dist=-m_dist;		
}

/*
* Summary:     Zoom in a polygon with nscale
*				
* Parameters:  The list of point which make the input polygon
* Return:      The list of point after zomming in
* Created:    03/10/2008 
* Modified : 
* Author    : Phuc Tran
*/
void Algorithm::ZoomInPoly(POINT *polygon,int nScale)
{
	for(int i=0;i<m_Polysize;i++)
	{
		polygon[i].x*=nScale;
		polygon[i].y*=nScale;
	}

}

/*
* Summary:     Zoom out a polygon with nscale
*				
* Parameters:  The list of point which make the input polygon
* Return:      The list of point after zomming out
* Created:    03/10/2008 
* Modified : 
* Author    : Phuc Tran
*/
void Algorithm::ZoomOutPoly(POINT *polygon,int nScale)
{
	for(int i=0;i<m_Polysize;i++)
	{
		polygon[i].x=DoubleToLong(polygon[i].x/nScale);
		polygon[i].y=DoubleToLong(polygon[i].y/nScale);

	}

}

/*
 * Summary:     Make a similarity polygon with the input polygon
 *				
 * Parameters:  The list of point which make the input polygon
 * Return:      The list of point which make the similarity polygon
 * Created:    03/09/2008 
 * Modified : 
 * Author    : Phuc Tran
 */
POINT* Algorithm::SimilarityPolygon(POINT *Pt, int d)
{
	POINT tmpP[4];
	POINT ptTemp1,ptTemp2;
	POINT ptCentre;
	POINT *ptRec,P0;
	POINT *SavePt;
	int savePolysize;			
	savePolysize=m_Polysize;
	P0.x=0;
	P0.y=0;
	//return Pt;//test only
	if (m_PtResult!=NULL)
	{
		delete []m_PtResult;
		m_PtResult = NULL;
	}
	m_PtResult = new POINT[m_Polysize];//This default value is used for testing only

	ptCentre=PolygonCentroid(Pt);
	SetDistance(d);	
	
	ptRec=PointBoundary(ptCentre,10);
			
	SavePt=new POINT[savePolysize];
	int i;
	for(i=0;i<savePolysize;i++)
		SavePt[i]=Pt[i];

	EliminateAngle(Pt);
	EliminateNearPoint(Pt,3);
	SetPolygonwiseclock(Pt);
//return NULL;///		
	if(IsPolyInsideRec(Pt,ptRec)&&d!=1)
	{
		ptRec=PointBoundary(ptCentre,30);
		if(IsPolyInsideRec(Pt,ptRec) &&d>10)
		SetDistance(10);
		else {
			ptRec=PointBoundary(ptCentre,15);
			if(IsPolyInsideRec(Pt,ptRec))
			{
				for(int i=0;i<savePolysize;i++)
					m_PtResult[i]=P0;
				return m_PtResult;

			}
		}
		
	}
	
	GetSimilarity(Pt[0],Pt[1],Pt);
	tmpP[0]=m_ptCurP1;
	tmpP[1]=m_ptCurP2;
	ptTemp1=m_ptCurP1;
	ptTemp2=m_ptCurP2;

	

	for(i=0;i<m_Polysize-1;i++)
	{
		if(i==m_Polysize-2)
		{
			 	if(Pt[i+1].x==Pt[0].x &&Pt[i+1].y==Pt[0].y)
				{
					
				tmpP[2]=ptTemp1;
				tmpP[3]=ptTemp2;
				LineIntersection(tmpP[0],tmpP[1],tmpP[2],tmpP[3]);
				m_PtResult[i]=m_PtIntersion;
				m_PtResult[m_Polysize-1]=m_PtIntersion;

				}
				else
				{
				GetSimilarity(Pt[i+1],Pt[0],Pt);
				tmpP[2]=m_ptCurP1;
				tmpP[3]=m_ptCurP2;
				LineIntersection(tmpP[0],tmpP[1],tmpP[2],tmpP[3]);
				m_PtResult[i]=m_PtIntersion;
				tmpP[0]=ptTemp1;
				tmpP[1]=ptTemp2;
				LineIntersection(tmpP[0],tmpP[1],tmpP[2],tmpP[3]);
				m_PtResult[m_Polysize-1]=m_PtIntersion;
				}						

		}
		else
		{
				//Eliminate the same points
				bool bflag=true;
				int pos;
				while(Pt[i+1].x==Pt[i+2].x &&Pt[i+1].y==Pt[i+2].y )
				{

					if(bflag)  	
					{
						pos=i;
						bflag=false;
					}

					i++;

				}
		
				GetSimilarity(Pt[i+1],Pt[i+2],Pt);
				tmpP[2]=m_ptCurP1;
				tmpP[3]=m_ptCurP2;
				LineIntersection(tmpP[0],tmpP[1],tmpP[2],tmpP[3]);
				m_PtResult[i]=m_PtIntersion;
				tmpP[0]=m_ptCurP1;
				tmpP[1]=m_ptCurP2;
				if(!bflag)
				{
					for(int k=pos;k<i;k++)
						m_PtResult[k]=m_PtIntersion;
				}
		
	  }

	}
	
	if(m_Polysize!=3)
		EliminateUnusedPoint(m_PtResult);
	
	EliminateOutsidePoint(m_PtResult,m_Polysize,SavePt,savePolysize);
	
	for(i=m_Polysize;i<savePolysize;i++)
		m_PtResult[i]=m_PtResult[m_Polysize-1];
	for(i=0;i<savePolysize;i++)
		Pt[i]=SavePt[i];	
	
	if(!IsPolyInsidePoly(m_PtResult,Pt,savePolysize))
	{
		for(int i=0;i<savePolysize;i++)
		m_PtResult[i]=P0;	
	
	}
	
	delete []SavePt;
	return m_PtResult;
}
/*
 * Summary:     Make a similarity polygon with the input polygon
 *				
 * Parameters:  The three points
 * Return:      The 2 points m_ptCurP1,m_ptCurP2  
 * Created:    03/09/2008 
 * Modified : 
 * Author    : Phuc Tran
 */
void Algorithm::GetSimilarity(const POINT& Pt1, const POINT& Pt2, POINT *Pt)
{
	double A,B,C,D,E;

	A=Pt1.y-Pt2.y;
	B=Pt2.x-Pt1.x;
	C=double(Pt1.x)*Pt2.y-double(Pt2.x)*Pt1.y;
	D=sqrt((double(B)*B+double(A)*A));
	if (abs(B)==0)
	{
		m_ptCurP1.x=Pt1.x-m_dist;
		m_ptCurP1.y=Pt1.y;
		m_ptCurP2.y=(Pt2.y+Pt1.y)/2;
		m_ptCurP2.x=Pt2.x-m_dist;				
		if(IsPointInPoly(Pt,m_ptCurP2,m_Polysize))
			m_ptCurP2.y=Pt2.y;				
		else
		{
			m_ptCurP1.x=Pt1.x+m_dist;
			m_ptCurP2.x=Pt2.x+m_dist;
			m_ptCurP2.y=Pt2.y;			
		}
	}
	else
	{
		m_ptCurP1.x=Pt1.x;
		E=C+A*m_ptCurP1.x;
		m_ptCurP1.y=DoubleToLong((D*m_dist-E)/B);

		m_ptCurP2.x=Pt2.x;
		E=C+A*m_ptCurP2.x;
		m_ptCurP2.y=DoubleToLong((D*m_dist-E)/B);
	}
 
}
void Algorithm::SetDistance(int d)
{
	m_dist=d;
}

void Algorithm::SetSize(int m_s)
{
	m_Polysize=m_s;
}


/*
* Summary:     Set the necessary variable for making an eclipse
*				
* Parameters:  the centre and rx,ry
* Return:      void
* Created:    21/9/2008 
* Modified : 
* Author    : Phuc Tran
*/
void Algorithm::SetEclipse(int x0,int y0,int rx,int ry)
{
	m_ptEclipseX_x=x0;
	m_ptEclipseX_y=y0;
	m_ptEclipseR_x=rx;
	m_ptEclipseR_y=ry;
}
void Algorithm::SetRectangle(int r,int b)
{
m_ptRectRB_x=r;
m_ptRectRB_y=b;
}

/*
* Summary:     Get the intersection point of a rectange and an elipse
*				
* Parameters:  the Eclipse centre(0,0) ,(rx,ry), the rectangle Rect(0,0,r,b)
* Return:      the list of intersection points if the eclipse intersect the rectangle
* Created:    22/09/2008 
* Modified : 
* Author    : Phuc Tran
*/
POINT* Algorithm::EclipseIntersection()
{
	double Delta,A;
	double x1,x2,y1,y2;

	int pos=0;
	//analyse 4 case
	for(int i=0;i<4;i++)
	{
		m_PtEIntersection[i].x=-1;
		m_PtEIntersection[i].y=-1;

	}
	if(m_bIncrease=CheckValue())
	{
		m_Scale=100;
		DecreaseValue();

	}
		
	//case 1: y=b
	A=m_ptEclipseX_x*m_ptEclipseR_y;
	Delta=-m_ptRectRB_y*m_ptRectRB_y+2*m_ptRectRB_y*m_ptEclipseX_y -m_ptEclipseX_y*m_ptEclipseX_y+ m_ptEclipseR_y*m_ptEclipseR_y;
	if(Delta>=0)
	{
		x1=(A+m_ptEclipseR_x*sqrt(Delta))/m_ptEclipseR_y;
		x2=(A-m_ptEclipseR_x*sqrt(Delta))/m_ptEclipseR_y;
		if(Delta==0)
		{
			if(x1>=0 && x1<=m_ptRectRB_x)
			{
				m_PtEIntersection[pos].x=x1*m_Scale;
				m_PtEIntersection[pos].y=m_ptRectRB_y*m_Scale;
				pos++;

			}

		}
		else
		{
			if(x1>=0 && x1<=m_ptRectRB_x)
			{
				m_PtEIntersection[pos].x=x1*m_Scale;
				m_PtEIntersection[pos].y=m_ptRectRB_y*m_Scale;
				pos++;

			}
			if(x2>0 && x2<=m_ptRectRB_x)
			{
					m_PtEIntersection[pos].x=x2*m_Scale;
					m_PtEIntersection[pos].y=m_ptRectRB_y*m_Scale;
					pos++;
				
			}
		}
	}
	//case 2: y=0
	Delta=-m_ptEclipseX_y*m_ptEclipseX_y+m_ptEclipseR_y*m_ptEclipseR_y;
	if(Delta>=0)
	{
		x1=(A+m_ptEclipseR_x*sqrt(Delta))/m_ptEclipseR_y;
		x2=(A-m_ptEclipseR_x*sqrt(Delta))/m_ptEclipseR_y;
		if(Delta==0)
		{
			if(x1>=0 && x1<=m_ptRectRB_x)
			{
					m_PtEIntersection[pos].x=x1*m_Scale;
					m_PtEIntersection[pos].y=0;
					pos++;
				
			}

		}
		else
		{
			if(x1>=0 && x1<=m_ptRectRB_x)
			{
					m_PtEIntersection[pos].x=x1*m_Scale;
					m_PtEIntersection[pos].y=0;
					pos++;
				
			}
			if(x2>=0 && x2<=m_ptRectRB_x)
			{
					m_PtEIntersection[pos].x=x2*m_Scale;
					m_PtEIntersection[pos].y=0;
					pos++;
				

			}
		}
	}
	//case 3: x=r

		
	A=m_ptEclipseX_y*m_ptEclipseR_x;
	Delta=-m_ptRectRB_x*m_ptRectRB_x+2*m_ptRectRB_x*m_ptEclipseX_x-m_ptEclipseX_x*m_ptEclipseX_x+m_ptEclipseR_x*m_ptEclipseR_x;
	if(Delta>=0)
	{
		y1=(A+m_ptEclipseR_y*sqrt(Delta))/m_ptEclipseR_x;
		y2=(A-m_ptEclipseR_y*sqrt(Delta))/m_ptEclipseR_x;
		if(Delta==0)
		{
			if(y1>=0 && y1<=m_ptRectRB_y)
			{
				m_PtEIntersection[pos].x=m_ptRectRB_x*m_Scale;
				m_PtEIntersection[pos].y=y1*m_Scale;
				pos++;

			}
		}
		else //Delta >0
		{
			if(y1>=0 && y1<=m_ptRectRB_y)
			{
				m_PtEIntersection[pos].x=m_ptRectRB_x*m_Scale;
				m_PtEIntersection[pos].y=y1*m_Scale;
				pos++;

			}
			if(y2>=0 && y2<=m_ptRectRB_y)
			{
				m_PtEIntersection[pos].x=m_ptRectRB_x*m_Scale;
				m_PtEIntersection[pos].y=y2*m_Scale;
				pos++;


			}
		}
	}

	//case 4: x=0

	Delta=-m_ptEclipseX_x*m_ptEclipseX_x+m_ptEclipseR_x*m_ptEclipseR_x;
	if(Delta>=0)
	{
		y1=(A+m_ptEclipseR_y*sqrt(Delta))/m_ptEclipseR_x;
		y2=(A-m_ptEclipseR_y*sqrt(Delta))/m_ptEclipseR_x;
		if(Delta==0)
		{
			if(y1>=0 && y1<=m_ptRectRB_y)
			{
				m_PtEIntersection[pos].x=0;
				m_PtEIntersection[pos].y=y1*m_Scale;
				pos++;

			}

		}
		else //Delta>0
		{
		
			if(y1>=0 && y1<=m_ptRectRB_y)
			{
				m_PtEIntersection[pos].x=0;
				m_PtEIntersection[pos].y=y1*m_Scale;
				pos++;

			}
			if(y2>=0 && y2<=m_ptRectRB_y)
			{
				m_PtEIntersection[pos].x=0;
				m_PtEIntersection[pos].y=y2*m_Scale;
				pos++;


			}
		}
	}

	m_Scale=1;
	//Eliminate the similar point
	for(int k=0;k<4;k++)
		for(int h=k+1;h<4;h++)
		{
			if(m_PtEIntersection[k].x!=-1)
			{
				if(m_PtEIntersection[k].x==m_PtEIntersection[h].x && m_PtEIntersection[k].y==m_PtEIntersection[h].y)
				{
					m_PtEIntersection[h].x=-1;
					m_PtEIntersection[h].y=-1;
				}


			}
			
		}

	return m_PtEIntersection;
}

/*
* Summary:     Get the distance from point A to point B
*				
* Parameters:  Point A,B
* Return:      The distance from point A to point B
* Created:    22/09/2008 
* Modified : 
* Author    : Phuc Tran
*/
double Algorithm::Distance(const POINT& Pt1, const POINT& Pt2)
{
	double A,B;
	A=(Pt2.x-Pt1.x)*double(Pt2.x-Pt1.x);
	B=(Pt2.y-Pt1.y)*double(Pt2.y-Pt1.y);
	return sqrt(A+B); 

}

double Algorithm::PolygonArea(POINT *polygon)
{
	int i,j;
	double area = 0;

	for (i=0;i<m_Polysize;i++) {
		j = (i + 1) % m_Polysize;
		area += polygon[i].x * polygon[j].y;
		area -= polygon[i].y * polygon[j].x;
	}

	area /= 2;
	return(area < 0 ? -area : area);


}

POINT Algorithm::PolygonCentroid(POINT *polygon)
{
	double cx=0,cy=0;
	//float A=(float)SignedPolygonArea(polygon,N);
	//Point2Df res=new Point2Df();
	double A=PolygonArea(polygon);
	POINT CentrePoint;
	int i,j;

	float factor=0;
	for (i=0;i<m_Polysize;i++) {
		j = (i + 1) % m_Polysize;
		factor=(polygon[i].x*polygon[j].y-polygon[j].x*polygon[i].y);
		cx+=(polygon[i].x+polygon[j].x)*factor;
		cy+=(polygon[i].y+polygon[j].y)*factor;
	}
	A*=6.0f;
	factor=1/A;
	cx*=factor;
	cy*=factor;
	CentrePoint.x=cx;
	CentrePoint.y=cy;
	return CentrePoint;

}

POINT * Algorithm::PointBoundary(POINT ptCentre,int d)
{
	m_PtRec[0].x=ptCentre.x -d;
	m_PtRec[0].y=ptCentre.y-d;
	m_PtRec[1].x=ptCentre.x +d;
	m_PtRec[1].y=ptCentre.y-d;
	m_PtRec[2].x=ptCentre.x +d;
	m_PtRec[2].y=ptCentre.y+d;
	m_PtRec[3].x=ptCentre.x -d;
	m_PtRec[3].y=ptCentre.y+d;
	return m_PtRec;

}

bool Algorithm::IsPolyInsideRec(POINT *polygon,POINT *polyRec)
{
	for(int i=0;i<m_Polysize;i++)
		if(!IsPointInPoly(polyRec,polygon[i],4))
			return false;
	return true;
}


/*
* Summary:     Rounder a double number to an long number
*				
* Parameters:  double number
* Return:      long number
* Created:    29/10/2008 
* Modified : 
* Author    : Phuc Tran
*/
long Algorithm::DoubleToLong(double num)
{
	long a=num;
	double b=num-a;

	if(num>=0)
	{
		if(b<0.5)
			return a;
		else
			return a+1;

	}
	else
	{

		if(b>-0.5)
			return a;
		else
			return a-1;

	}
	

}

/*
* Summary:     Check the input value
*				
* Parameters:  the input value
* Return:      true if the input value> maximum allowed number
* Created:    22/09/2008 
* Modified : 
* Author    : Phuc Tran
*/
bool Algorithm::CheckValue()
{
	int m_MaxValue = 45000;
	if (m_ptEclipseX_x>m_MaxValue)
		return true;
	else if (m_ptEclipseX_y>m_MaxValue)
		return true;
	else if (m_ptEclipseR_x>m_MaxValue)
		return true;
	else if (m_ptEclipseR_y>m_MaxValue)
		return true;
	else if (m_ptRectRB_x>m_MaxValue)
		return true;
	else if (m_ptRectRB_y>m_MaxValue)
		return true;
	else
	{
		m_bIncrease=true;
		return false;
	}
}
/*
* Summary:     Decrease the value with a scale
*				
* Parameters:  
* Return:      
* Created:    22/09/2008 
* Modified : 
* Author    : Phuc Tran
*/
void Algorithm::DecreaseValue()
{
	m_ptEclipseX_x=m_ptEclipseX_x/m_Scale;
	m_ptEclipseX_y=m_ptEclipseX_y/m_Scale;
	m_ptEclipseR_x=m_ptEclipseR_x/m_Scale;
	m_ptEclipseR_y=m_ptEclipseR_y/m_Scale;
	m_ptRectRB_x=m_ptRectRB_x/m_Scale;
	m_ptRectRB_y=m_ptRectRB_y/m_Scale;
}
