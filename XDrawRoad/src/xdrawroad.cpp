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

#include "PolygonVersionF.h"

namespace XDrawRoad
{
	#define			PI		3.1415926535897932384626433832795
	#define			ANGLE_DEGREE_STEP				10
	#define			ANGLE_DEGREE_STEP_BUTT_CAP		2	
	#define         DELTA_ARCOS  0.0000005
	#define         DELTA_WIDE  0.45
	#define _SWAP(a,b)tmp=a;a=b;b=tmp
	#define			CheckAndPush(ptAdd)				{if (ptAdd.mX!=ptOld.mX || ptAdd.mY!=ptOld.mY) {vPoly.push_back(ptAdd); ptOld = ptAdd;}}

	vector<VECTOR2D> m_vPoly;

	const int v1 = 1;
	const int v2 = 8;
	const int HEIGHTS = (v2*320)/(v2-v1);
	const int H1 = HEIGHTS*v1/v2;
	const unsigned int WTable[] = {	262144,262144,262144,262145,262145,262147,262148,262150,262151,262154,262156,262159,262161,262165,262168,262172,262175,262180,262184,262189,262193,262199,262204,262210,262215,262222,262228,262235,262241,262249,262256,262264,262271,262280,262288,262297,262305,262315,262324,262334,262343,262354,262364,262375,262385,262397,262408,262419,262431,262443,262456,262468,262481,262494,262508,262521,262535,262549,262564,262578,262593,262608,262624,262639,
		262655,262671,262687,262704,262721,262738,262755,262773,262791,262809,262827,262846,262865,262884,262903,262922,262942,262962,262983,263003,263024,263045,263066,263088,263110,263132,263154,263177,263199,263222,263246,263269,263293,263317,263341,263366,263391,263416,263441,263466,263492,263518,263544,263571,263597,263624,263652,263679,263707,263735,263763,263791,263820,263849,263878,263908,263937,263967,263997,264028,264059,264089,264121,264152,
		264184,264215,264248,264280,264313,264345,264378,264412,264445,264479,264513,264548,264582,264617,264652,264687,264723,264759,264795,264831,264867,264904,264941,264978,265016,265053,265091,265130,265168,265207,265246,265285,265324,265364,265404,265444,265484,265525,265566,265607,265648,265690,265731,265773,265816,265858,265901,265944,265987,266031,266075,266118,266163,266207,266252,266297,266342,266387,266433,266479,266525,266571,266618,266665,
		266712,266759,266807,266854,266902,266951,266999,267048,267097,267146,267195,267245,267295,267345,267395,267446,267497,267548,267599,267651,267703,267755,267807,267859,267912,267965,268018,268072,268125,268179,268233,268288,268342,268397,268452,268507,268563,268619,268675,268731,268787,268844,268901,268958,269015,269073,269131,269189,269247,269306,269365,269424,269483,269542,269602,269662,269722,269782,269843,269904,269965,270026,270088,270149,
		270211,270274,270336,270399,270462,270525,270588,270652,270715,270779,270844,270908,270973,271038,271103,271168,271234,271300,271366,271432,271499,271565,271632,271699,271767,271834,271902,271970,272039,272107,272176,272245,272314,272384,272453,272523,272593,272664,272734,272805,272876,272947,273018,273090,273162,273234,273306,273379,273452,273525,273598,273671,273745,273819,273893,273967,274041,274116,274191,274266,274342,274417,274493,274569,
		274645,274722,274799,274875,274953,275030,275107,275185,275263,275341,275420,275498,275577,275656,275736,275815,275895,275975,276055,276135,276216,276297,276378,276459,276540,276622,276704,276786,276868,276950,277033,277116,277199,277282,277366,277450,277534,277618,277702,277787,277872,277957,278042,278127,278213,278299,278385,278471,278558,278644,278731,278818,278906,278993,279081,279169,279257,279345,279434,279523,279612,279701,279790,279880,
		279969,280059,280150,280240,280331,280421,280512,280604,280695,280787,280879,280971,281063,281155,281248,281341,281434,281527,281620,281714,281808,281902,281996,282091,282185,282280,282375,282471,282566,282662,282757,282854,282950,283046,283143,283240,283337,283434,283531,283629,283727,283825,283923,284022,284120,284219,284318,284417,284517,284616,284716,284816,284916,285017,285117,285218,285319,285420,285522,285623,285725,285827,285929,286031,
		286134,286236,286339,286442,286546,286649,286753,286857,286961,287065,287169,287274,287379,287484,287589,287694,287800,287906,288012,288118,288224,288331,288437,288544,288651,288759,288866,288974,289081,289189,289298,289406,289515,289623,289732,289841,289951,290060,290170,290280,290390,290500,290610,290721,290832,290943,291054,291165,291277,291388,291500,291612,291725,291837,291950,292062,292175,292288,292402,292515,292629,292743,292857,292971,
		293085,293200,293315,293430,293545,293660,293776,293891,294007,294123,294239,294356,294472,294589,294706,294823,294940,295057,295175,295293,295411,295529,295647,295765,295884,296003,296122,296241,296360,296480,296600,296719,296839,296960,297080,297201,297321,297442,297563,297684,297806,297927,298049,298171,298293,298415,298538,298660,298783,298906,299029,299152,299276,299399,299523,299647,299771,299895,300020,300144,300269,300394,300519,300644,
		300770,300895,301021,301147,301273,301399,301526,301652,301779,301906,302033,302160,302288,302415,302543,302671,302799,302927,303055,303184,303313,303442,303571,303700,303829,303959,304088,304218,304348,304478,304609,304739,304870,305000,305131,305262,305394,305525,305657,305788,305920,306052,306184,306317,306449,306582,306715,306848,306981,307114,307248,307381,307515,307649,307783,307917,308052,308186,308321,308456,308591,308726,308861,308997,
		309132,309268,309404,309540,309676,309812,309949,310086,310222,310359,310497,310634,310771,310909,311047,311184,311322,311461,311599,311737,311876,312015,312154,312293,312432,312571,312711,312850,312990,313130,313270,313411,313551,313691,313832,313973,314114,314255,314396,314538,314679,314821,314963,315105,315247,315389,315532,315674,315817,315960,316103,316246,316389,316532,316676,316820,316963,317107,317252,317396,317540,317685,317829,317974,
		318119,318264,318410,318555,318700,318846,318992,319138,319284,319430,319577,319723,319870,320016,320163,320310,320458,320605,320752,320900,321048,321195,321343,321492,321640,321788,321937,322085,322234,322383,322532,322681,322831,322980,323130,323280,323430,323580,323730,323880,324030,324181,324332,324482,324633,324784,324936,325087,325238,325390,325542,325694,325846,325998,326150,326302,326455,326607,326760,326913,327066,327219,327373,327526,
		327680,327833,327987,328141,328295,328449,328603,328758,328912,329067,329222,329377,329532,329687,329842,329998,330153,330309,330465,330621,330777,330933,331089,331246,331402,331559,331716,331873,332030,332187,332344,332502,332659,332817,332975,333133,333291,333449,333607,333765,333924,334083,334241,334400,334559,334718,334878,335037,335196,335356,335516,335676,335836,335996,336156,336316,336477,336637,336798,336959,337120,337281,337442,337603,
		337764,337926,338087,338249,338411,338573,338735,338897,339059,339222,339384,339547,339710,339873,340036,340199,340362,340525,340689,340852,341016,341180,341344,341508,341672,341836,342001,342165,342330,342494,342659,342824,342989,343154,343319,343485,343650,343816,343982,344147,344313,344479,344645,344812,344978,345145,345311,345478,345645,345812,345979,346146,346313,346480,346648,346815,346983,347151,347319,347487,347655,347823,347991,348160,
		348328,348497,348665,348834,349003,349172,349341,349511,349680,349850,350019,350189,350359,350529,350699,350869,351039,351209,351380,351550,351721,351891,352062,352233,352404,352575,352747,352918,353089,353261,353433,353604,353776,353948,354120,354292,354465,354637,354810,354982,355155,355328,355500,355673,355846,356020,356193,356366,356540,356713,356887,357061,357235,357409,357583,357757,357931,358105,358280,358454,358629,358804,358979,359154,
		359329,359504,359679,359854,360030,360205,360381,360557,360732,360908,361084,361260,361437,361613,361789,361966,362142,362319,362496,362673,362850,363027,363204,363381,363558,363736,363913,364091,364269,364446,364624,364802,364980,365159,365337,365515,365694,365872,366051,366230,366408,366587,366766,366945,367125,367304,367483,367663,367842,368022,368202,368381,368561,368741,368921,369102,369282,369462,369643,369823,370004,370184,370365,370546,370727
	}; 

	inline void MovePoint(const POINT& pt, int ux, int uy, float dWid, VECTOR2D& ptNew)
	{
		float u = sqrt((float)(ux*ux+uy*uy));	
		ptNew.mX = pt.x + ux*dWid/u;
		ptNew.mY = pt.y + uy*dWid/u;
	}

	inline void MovePoint(const POINT& pt, int ux, int uy, float u, float dWid, VECTOR2D& ptNew)
	{
		if (dWid==0)
			int kk = 0;
		ptNew.mX = pt.x + ux*dWid/u;
		ptNew.mY = pt.y + uy*dWid/u;
	}

	inline void Get3DWide(float nWidth, const POINT &pt1, const POINT &pt2, float &nWideStart, float &nWideEnd, bool bOutline)
	{
		float Dx, Dy;
		float dx, dy, tmp, wide, dwide;

		nWideStart = 0;
		nWideEnd = 0;

		int x1 = pt1.x;
		int y1 = pt1.y;
		dx = pt2.x - pt1.x;
		dy = pt2.y - pt1.y;
		if (!dx && !dy)
			return;
		Dx = fabs(dx);
		Dy = fabs(dy);						
		bool bSwap = false;
		if (Dy<Dx)
		{		
			_SWAP(Dx, Dy);
			_SWAP(dx, dy);
			_SWAP(x1, y1);
			bSwap = true;
		}
		wide = WTable[0]* nWidth / 65536;
		if (bSwap)
		{
			dwide = wide*dx/(HEIGHTS*dy);
			nWideStart = wide*(H1+x1)/HEIGHTS;		
		}
		else
		{
			dwide = wide/HEIGHTS;		
			nWideStart = wide*(H1+y1)/HEIGHTS;
		}

		nWideEnd = nWideStart + dwide*dy;

		nWideStart = nWideStart / 8;
		nWideEnd = nWideEnd / 8;
		if (bOutline)
		{
			nWideStart += DELTA_WIDE;
			nWideEnd += DELTA_WIDE;
		}
		else
		{
			nWideStart -= DELTA_WIDE;
			nWideEnd -= DELTA_WIDE;
		}
	}

	inline float GetAngle(const POINT& pt1, const VECTOR2D& pt2)
	{
		float dx = pt2.mX - pt1.x;
		float dy = pt2.mY - pt1.y;
		float d = sqrt(dx*dx + dy*dy);	
		float dAngle = 0;
		if (d>0)
		{
			float aCos = dx/d;
			if (aCos>1) aCos = 1; else if (aCos<-1) aCos = -1;
			dAngle = acos(aCos);
			if (dy<0)
				dAngle = -dAngle; 
		}
		return dAngle;
	}

	inline VECTOR2D GetIntersecPoint(const VECTOR2D& pt1, const VECTOR2D& pt2, const VECTOR2D& pt3, const VECTOR2D& pt4)
	{
		VECTOR2D pt;

		float k = (pt4.mY - pt3.mY)*(pt2.mX - pt1.mX) - (pt4.mX - pt3.mX)*(pt2.mY - pt1.mY);
		if (k!=0)
		{
			float kt = (pt4.mX - pt3.mX)*(pt1.mY - pt3.mY) - (pt4.mY - pt3.mY)*(pt1.mX - pt3.mX);
			float t = kt/k;
			pt.mX = pt1.mX + t*(pt2.mX - pt1.mX);
			pt.mY = pt1.mY + t*(pt2.mY - pt1.mY);
		}
		else
		{
			pt.mY = (pt2.mX + pt3.mY) / 2;
			pt.mY = (pt2.mX + pt3.mY) / 2;
		}

		return pt;
	}

	inline POINT* GetNextPoint(POINT* p, int dp, POINT* pCheck)
	{
		POINT* pNext = p + dp;
		while (pNext!=pCheck && pNext->x==p->x && pNext->y==p->y)
			pNext += dp;
		return pNext;
	}

	inline int Generate3DPoly(POINT* pPts, int nPt, float nWidth, vector<VECTOR2D>& vPoly, bool bOutline, bool bButtCap)
	{	
		float dWid, dWid1, dWid2, dWid3;	
		VECTOR2D ptOld, pt;

		vPoly.clear();		
		if(nPt * NUM_OF_JOIN_POINT + NUM_OF_CAP_POINT > NUM_OF_POLYGON_VERTEX)			
			vPoly.reserve(nPt * NUM_OF_JOIN_POINT + NUM_OF_CAP_POINT);
		ptOld.mX = ptOld.mY = -1000;	
		for (int nStep = 0; nStep<2; nStep++)
		{
			POINT* pCheck = pPts + nPt;
			POINT* p = pPts;
			int dp = 1;
			if (nStep)
			{
				p = pPts + nPt - 1;
				dp = -1;
				pCheck = pPts - 1;
			}
			//POINT* p1 = p + dp;
			POINT* p1 = GetNextPoint(p, dp, pCheck);
			if (p1==pCheck)
				return 0;
			//POINT* p2 = p1 + dp;
			POINT* p2 = GetNextPoint(p1, dp, pCheck);

			// Tinh toan doan dau tien
			long v1x, v1y, u1x, u1y, v2x, v2y, u2x, u2y;
			float v1, v2;
			Get3DWide(nWidth, *p, *p1, dWid, dWid1, bOutline);
			u1x = p1->x - p->x;		// vector chi phuong
			u1y = p1->y - p->y;
			v1x = -u1y;				// vector phap tuyen huong sang phai
			v1y = u1x;
			v1 = sqrt((float)(v1x*v1x + v1y*v1y));	//  v1x*v1x + v1y*v1y => Co the tran

			VECTOR2D pt11, pt12, pt21, pt22;
			MovePoint(*p, v1x, v1y, v1, dWid, pt11);
			MovePoint(*p1, v1x, v1y, v1, dWid1, pt12);
			CheckAndPush(pt11);

			while (p1!=pCheck)
			{
				if (p2==pCheck)
				{
					// Sinh arc
					float dStartAngle = GetAngle(*p1, pt12);
					if (bButtCap)
					{	
						pt.mX = p1->x + dWid1*cos(dStartAngle);
						pt.mY = p1->y + dWid1*sin(dStartAngle);
						CheckAndPush(pt);
						if (!bOutline)
						{			
							float dAngeRadStep = ANGLE_DEGREE_STEP_BUTT_CAP*PI/180;			
							int last = 180 / ANGLE_DEGREE_STEP_BUTT_CAP - 1;
							pt.mX = p1->x + dWid1*cos(dStartAngle - dAngeRadStep);
							pt.mY = p1->y + dWid1*sin(dStartAngle - dAngeRadStep);
							CheckAndPush(pt);							
							pt.mX = p1->x + dWid1*cos(dStartAngle - dAngeRadStep * last);
							pt.mY = p1->y + dWid1*sin(dStartAngle - dAngeRadStep * last);
							CheckAndPush(pt);							
						}						
					}
					else
					{
						float dAngeRadStep = ANGLE_DEGREE_STEP*PI/180;
						for (int nAngle=0; nAngle<180; nAngle+=ANGLE_DEGREE_STEP)
						{
							pt.mX = p1->x + dWid1*cos(dStartAngle);
							pt.mY = p1->y + dWid1*sin(dStartAngle);
							CheckAndPush(pt);							
							dStartAngle -= dAngeRadStep;
						}
					}
					break;
				}
				else
				{
					Get3DWide(nWidth, *p1, *p2, dWid2, dWid3, bOutline);
					u2x = p2->x - p1->x;	// vector chi phuong
					u2y = p2->y - p1->y;
					v2x = -u2y;		// vector phap tuyen huong sang phai
					v2y = u2x;
					v2 = sqrt((float)(v2x*v2x + v2y*v2y));
					MovePoint(*p1, v2x, v2y, v2, dWid2, pt21);
					MovePoint(*p2, v2x, v2y, v2, dWid3, pt22);

					bool bIntersec = (v1x*u2x + v1y*u2y)>0;
					if (bIntersec)
					{
						pt = GetIntersecPoint(pt11, pt12, pt21, pt22);
						CheckAndPush(pt);
					}
					else
					{
						float dStartAngle = GetAngle(*p1, pt12);
						float dEndAngle = GetAngle(*p1, pt21);
						float dAngeRadStep = ANGLE_DEGREE_STEP*PI/180;
						while (dStartAngle<dEndAngle)
							dEndAngle -= 2*PI;

						CheckAndPush(pt12);

						float dAngleDelta = dStartAngle-dEndAngle;
						if (dAngleDelta>(PI / 18) && dAngleDelta<PI)
						{
							// Sinh them diem noi
							float dw = (dWid2 - dWid1)*dAngeRadStep/(dStartAngle - dEndAngle);
							float dWidTmp = dWid1 + dw;
							dStartAngle -= dAngeRadStep;
							while (dStartAngle>dEndAngle)
							{
								pt.mX = p1->x + dWidTmp*cos(dStartAngle);
								pt.mY = p1->y + dWidTmp*sin(dStartAngle);
								CheckAndPush(pt);
								dStartAngle -= dAngeRadStep;
								dWidTmp += dw;
							}
						}

						CheckAndPush(pt21);
					}

					v1x = v2x; v1y = v2y; v1 = v2;
					dWid = dWid2;
					dWid1 = dWid3;
					pt11 = pt21;
					pt12 = pt22;
					
					//p += dp; p1 += dp; p2 += dp;
					p = p1; p1 = p2;
					p2 = GetNextPoint(p1, dp, pCheck);
				}
			}
		}

		return vPoly.size();
	}

	inline int Generate2DPoly(POINT* pPts, int nPt, float nWidth, vector<VECTOR2D>& vPoly, bool bOutline, bool bButtCap)
	{	
		float dWid = nWidth;
		dWid = dWid/2;
		VECTOR2D ptOld, pt;

		vPoly.clear();	
		if(nPt * NUM_OF_JOIN_POINT + NUM_OF_CAP_POINT > NUM_OF_POLYGON_VERTEX)			
			vPoly.reserve(nPt * NUM_OF_JOIN_POINT + NUM_OF_CAP_POINT);
		ptOld.mX = ptOld.mY = -1000;
		for (int nStep = 0; nStep<2; nStep++)
		{
			POINT* pCheck = pPts + nPt;
			POINT* p = pPts;
			int dp = 1;
			if (nStep)
			{
				p = pPts + nPt - 1;
				dp = -1;
				pCheck = pPts - 1;
			}
			//POINT* p1 = p + dp;
			POINT* p1 = GetNextPoint(p, dp, pCheck);
			if (p1==pCheck)
				return 0;
			//POINT* p2 = p1 + dp;
			POINT* p2 = GetNextPoint(p1, dp, pCheck);

			// Tinh toan doan dau tien
			long v1x, v1y, u1x, u1y, v2x, v2y, u2x, u2y;
			float v1, v2;
			u1x = p1->x - p->x;		// vector chi phuong
			u1y = p1->y - p->y;
			v1x = -u1y;				// vector phap tuyen huong sang phai
			v1y = u1x;
			v1 = sqrt((float)(v1x*v1x + v1y*v1y));	// v1x*v1x + v1y*v1y => Co the tran

			VECTOR2D pt11, pt12, pt21, pt22;
			MovePoint(*p, v1x, v1y, v1, dWid, pt11);
			MovePoint(*p1, v1x, v1y, v1, dWid, pt12);
			CheckAndPush(pt11);

			while (p1!=pCheck)
			{
				if (p2==pCheck)
				{
					// Sinh arc
					float dStartAngle = GetAngle(*p1, pt12);
					if (bButtCap)
					{	
						pt.mX = p1->x + dWid*cos(dStartAngle);
						pt.mY = p1->y + dWid*sin(dStartAngle);
						CheckAndPush(pt);
						if(!bOutline)
						{			
							float dAngeRadStep = ANGLE_DEGREE_STEP_BUTT_CAP*PI/180;			
							int last = 180 / ANGLE_DEGREE_STEP_BUTT_CAP - 1;
							pt.mX = p1->x + dWid*cos(dStartAngle - dAngeRadStep);
							pt.mY = p1->y + dWid*sin(dStartAngle - dAngeRadStep);
							CheckAndPush(pt);
							pt.mX = p1->x + dWid*cos(dStartAngle - dAngeRadStep * last);
							pt.mY = p1->y + dWid*sin(dStartAngle - dAngeRadStep * last);
							CheckAndPush(pt);							
						}						
					}
					else
					{
						float dAngeRadStep = ANGLE_DEGREE_STEP*PI/180;
						for (int nAngle=0; nAngle<180; nAngle+=ANGLE_DEGREE_STEP)
						{
							pt.mX = p1->x + dWid*cos(dStartAngle);
							pt.mY = p1->y + dWid*sin(dStartAngle);
							CheckAndPush(pt);
							dStartAngle -= dAngeRadStep;
						}	
					}									
					break;
				}
				else
				{
					u2x = p2->x - p1->x;	// vector chi phuong
					u2y = p2->y - p1->y;
					v2x = -u2y;		// vector phap tuyen huong sang phai
					v2y = u2x;
					v2 = sqrt((float)(v2x*v2x + v2y*v2y));
					MovePoint(*p1, v2x, v2y, v2, dWid, pt21);
					MovePoint(*p2, v2x, v2y, v2, dWid, pt22);

					bool bIntersec = (v1x*u2x + v1y*u2y)>0;
					if (bIntersec)
					{
						pt = GetIntersecPoint(pt11, pt12, pt21, pt22);
						CheckAndPush(pt);
					}
					else
					{
						float dStartAngle = GetAngle(*p1, pt12);
						float dEndAngle = GetAngle(*p1, pt21);
						float dAngeRadStep = ANGLE_DEGREE_STEP*PI/180;
						while (dStartAngle<dEndAngle)
							dEndAngle -= 2*PI;

						CheckAndPush(pt12);

						float dAngleDelta = dStartAngle-dEndAngle;
						if (dAngleDelta>(PI / 18) && dAngleDelta<PI)
						{
							// Sinh them diem noi
							dStartAngle -= dAngeRadStep;
							while (dStartAngle>dEndAngle)
							{
								pt.mX = p1->x + dWid*cos(dStartAngle);
								pt.mY = p1->y + dWid*sin(dStartAngle);
								CheckAndPush(pt);
								dStartAngle -= dAngeRadStep;
							}
						}

						CheckAndPush(pt21);
					}

					v1x = v2x; v1y = v2y; v1 = v2;
					pt11 = pt21;
					pt12 = pt22;
					
					// p += dp; p1 += dp; p2 += dp;
					p = p1; p1 = p2;
					p2 = GetNextPoint(p1, dp, pCheck);
				}
			}
		}

		return vPoly.size();
	}

	void InitDraw(PolygonVersionF* &pPolygon, unsigned int aWidth, unsigned int aHeight)
	{
		pPolygon = new PolygonVersionF();
		pPolygon->init(aWidth, aHeight, NUM_OF_EDGE);
		m_vPoly.reserve(NUM_OF_POLYGON_VERTEX);	
	}

	void CloseDraw(PolygonVersionF* &pPolygon)
	{
		delete pPolygon;
		pPolygon = NULL;
	}

	float Get3DRoadWidth(const POINT& pt, int nWid)
	{
		POINT ptEnd = pt;
		ptEnd.x++; ptEnd.y++;
		float dWid1, dWid2;
		Get3DWide(nWid, pt, ptEnd, dWid1, dWid2, true);
		return dWid1;
	}

	bool Get3DRoadWidth(const POINT& pt1, const POINT& pt2, int nWid, float& dWid1, float& dWid2)
	{
		Get3DWide(nWid, pt1, pt2, dWid1, dWid2, true);
		return true;
	}

	void Draw2DRoad(PolygonVersionF* pPolygon, BYTE* pDisplay, int nPitch, POINT *pPts, int nPt, float fWid, Pixel clrInline, Pixel clrOutline, bool bOutline, bool bButtCap)
	{
		if (!bOutline)
		{
			if (fWid > 0)
				fWid = fWid - 1.0;
			int numPoint = Generate2DPoly(pPts, nPt, fWid, m_vPoly, bOutline, bButtCap);
			if (numPoint>2)
				//pPolygon->renderNonZeroWinding(pDisplay, nPitch, &m_vPoly[0], numPoint, clrInline);
				pPolygon->renderEvenOdd(pDisplay, nPitch, &m_vPoly[0], numPoint, clrInline);

		}
		else
		{
			fWid = fWid + 1.0;
			int numPoint = Generate2DPoly(pPts, nPt, fWid, m_vPoly, bOutline, bButtCap);
			if (numPoint>2)
				//pPolygon->renderNonZeroWinding(pDisplay, nPitch, &m_vPoly[0], numPoint, clrOutline);
				pPolygon->renderEvenOdd(pDisplay, nPitch, &m_vPoly[0], numPoint, clrOutline);
		}
	}

	void Draw3DRoad(PolygonVersionF* pPolygon, BYTE* pDisplay, int nPitch, POINT *pPts, int nPt, float fWid, Pixel clrInline, Pixel clrOutline, bool bOutline, bool bButtCap)
	{
		if (!bOutline)
		{			
			if (fWid > 3.0)
				fWid = fWid - 5 /fWid;
			int numPoint = Generate3DPoly(pPts, nPt, fWid, m_vPoly, bOutline, bButtCap);
			if (numPoint>2)
				//pPolygon->renderNonZeroWinding(pDisplay, nPitch, &m_vPoly[0], numPoint, clrInline);
				pPolygon->renderEvenOdd(pDisplay, nPitch, &m_vPoly[0], numPoint, clrInline);
		}
		else
		{
			if (fWid > 3.0)
				fWid = fWid + 5 /fWid;
			int numPoint = Generate3DPoly(pPts, nPt, fWid, m_vPoly, bOutline, bButtCap);
			if (numPoint>2)
				//pPolygon->renderNonZeroWinding(pDisplay, nPitch, &m_vPoly[0], numPoint, clrOutline);
				pPolygon->renderEvenOdd(pDisplay, nPitch, &m_vPoly[0], numPoint, clrOutline);
		}
	}

	void FillPolyAA(PolygonVersionF* pPolygon, BYTE* pDisplay, int nPitch, POINT *pPts, int nPt, Pixel clrColor)
	{
		if (nPt>2)
		{
			m_vPoly.clear();
			Vector2d v2d;
			for (int i=0; i<nPt; i++)
			{
				v2d.mX = pPts[i].x;
				v2d.mY = pPts[i].y;
				m_vPoly.push_back(v2d);
			}
			pPolygon->renderEvenOdd(pDisplay, nPitch, &m_vPoly[0], m_vPoly.size(), clrColor);
		}
	}

	void FillPolyPolyMcDowellAA(PolygonVersionF* pPolygon, BYTE* pDisplay, int nPitch, POINT* pPts, int* pVertextIdx, int nVertexCnt, Pixel clrColor)
	{
		int nPt = pVertextIdx[nVertexCnt-1];
		if (nPt>2)
		{
			m_vPoly.clear();
			Vector2d v2d;
			for (int i=0; i<nPt; i++)
			{
				v2d.mX = pPts[i].x;
				v2d.mY = pPts[i].y;
				m_vPoly.push_back(v2d);
			}
			pPolygon->renderEvenOddEx(pDisplay, nPitch, &m_vPoly[0], pVertextIdx, nVertexCnt, clrColor);
		}
	}
}

