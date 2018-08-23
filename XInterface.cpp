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

#include "AggUtil.h"

#ifndef USING_AGG_FOR_MAP
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgb_packed.h"
#include "agg_renderer_base.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_path_storage.h"
#include "agg_conv_stroke.h"
#include "agg_scanline_p.h"
#include "agg_pattern_filters_rgba.h"
#include "agg_renderer_outline_aa.h"
#include "agg_renderer_outline_image.h"
#include "agg_rasterizer_outline_aa.h"
#endif // USING_AGG_FOR_MAP

#include "xdraw.h"
#include "xbuffer.h"

typedef agg::pixfmt_rgb565 pixfmt;
typedef agg::pixfmt_rgb565_pre pixfmt_pre;
typedef agg::rgba8 color_type;

using namespace agg;
static agg::int8u brightness_to_alpha[256 * 3] = 
{
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 254, 254, 254, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 253, 253, 
    253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 252, 
    252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 251, 251, 251, 251, 251, 
    251, 251, 251, 251, 250, 250, 250, 250, 250, 250, 250, 250, 249, 249, 249, 249, 
    249, 249, 249, 248, 248, 248, 248, 248, 248, 248, 247, 247, 247, 247, 247, 246, 
    246, 246, 246, 246, 246, 245, 245, 245, 245, 245, 244, 244, 244, 244, 243, 243, 
    243, 243, 243, 242, 242, 242, 242, 241, 241, 241, 241, 240, 240, 240, 239, 239, 
    239, 239, 238, 238, 238, 238, 237, 237, 237, 236, 236, 236, 235, 235, 235, 234, 
    234, 234, 233, 233, 233, 232, 232, 232, 231, 231, 230, 230, 230, 229, 229, 229, 
    228, 228, 227, 227, 227, 226, 226, 225, 225, 224, 224, 224, 223, 223, 222, 222, 
    221, 221, 220, 220, 219, 219, 219, 218, 218, 217, 217, 216, 216, 215, 214, 214, 
    213, 213, 212, 212, 211, 211, 210, 210, 209, 209, 208, 207, 207, 206, 206, 205, 
    204, 204, 203, 203, 202, 201, 201, 200, 200, 199, 198, 198, 197, 196, 196, 195, 
    194, 194, 193, 192, 192, 191, 190, 190, 189, 188, 188, 187, 186, 186, 185, 184, 
    183, 183, 182, 181, 180, 180, 179, 178, 177, 177, 176, 175, 174, 174, 173, 172, 
    171, 171, 170, 169, 168, 167, 166, 166, 165, 164, 163, 162, 162, 161, 160, 159, 
    158, 157, 156, 156, 155, 154, 153, 152, 151, 150, 149, 148, 148, 147, 146, 145, 
    144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 
    128, 128, 127, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 
    112, 111, 110, 109, 108, 107, 106, 105, 104, 102, 101, 100,  99,  98,  97,  96,  
     95,  94,  93,  91,  90,  89,  88,  87,  86,  85,  84,  82,  81,  80,  79,  78, 
     77,  75,  74,  73,  72,  71,  70,  69,  67,  66,  65,  64,  63,  61,  60,  59, 
     58,  57,  56,  54,  53,  52,  51,  50,  48,  47,  46,  45,  44,  42,  41,  40, 
     39,  37,  36,  35,  34,  33,  31,  30,  29,  28,  27,  25,  24,  23,  22,  20, 
     19,  18,  17,  15,  14,  13,  12,  11,   9,   8,   7,   6,   4,   3,   2,   1
};


class pattern_src_brightness_to_alpha_rgba8
{
public:
    pattern_src_brightness_to_alpha_rgba8(agg::rendering_buffer& rb) : 
        m_rb(&rb), m_pf(*m_rb){}

    unsigned width()  const { return m_pf.width();  }
    unsigned height() const { return m_pf.height(); }
    agg::rgba8 pixel(int x, int y) const
    {
        agg::rgba8 c = m_pf.pixel(x, y);
        c.a = brightness_to_alpha[c.r + c.g + c.b];
        return c;
    }

private:
    agg::rendering_buffer* m_rb;
    pixfmt m_pf;
};

struct LinePattern
{
	typedef agg::pixfmt_rgb565 pixfmt;	
	typedef agg::renderer_base<pixfmt> ren_base;
	typedef agg::line_image_pattern<agg::pattern_filter_bilinear_rgba8> pattern_type;	
	typedef agg::renderer_outline_image<ren_base, pattern_type> renderer_img_type;
	typedef agg::rasterizer_outline_aa<renderer_img_type, agg::line_coord_sat> rasterizer_img_type;
	typedef agg::renderer_outline_aa<ren_base> renderer_line_type;
	typedef agg::rasterizer_outline_aa<renderer_line_type, agg::line_coord_sat> rasterizer_line_type;

	PXSurface linepattern;
	agg::rendering_buffer *pattern;
	pattern_src_brightness_to_alpha_rgba8 *p1;
	agg::pattern_filter_bilinear_rgba8 fltr;           // Filtering functor
	pattern_type *patt;  
	agg::trans_affine             scale;

	LinePattern(PXSurface pxSurface)
	{
		patt = NULL;
		p1 = NULL;
		pattern = NULL;
		linepattern = NULL;

		if (!pxSurface)
			return;

		linepattern = ::CloneXSurface(pxSurface);
		if (!linepattern)
			return;

		pattern = new agg::rendering_buffer((unsigned char*)::GetXSurfacebuffer(linepattern),
			(unsigned)GetXSurfaceWidth(linepattern),
			(unsigned)GetXSurfaceHeight(linepattern),
			(unsigned)GetXSurfacePitch(linepattern)
			);
		p1 = new pattern_src_brightness_to_alpha_rgba8(*pattern);
		patt = new pattern_type(fltr);
		patt->create(*p1);
	}

#ifdef OS_WIN
	LinePattern(int iResourceID)
	{
		patt = NULL;
		p1 = NULL;
		pattern = NULL;
		linepattern = NULL;

		HBITMAP hbmp = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(iResourceID));
		if (!hbmp)
			return;

		linepattern = ::CreateXSurface(hbmp);
		if (!linepattern)
			return;
		::DeleteObject(hbmp);	
		pattern = new agg::rendering_buffer((unsigned char*)::GetXSurfacebuffer(linepattern),
									(unsigned)GetXSurfaceWidth(linepattern),
									(unsigned)GetXSurfaceHeight(linepattern),
									(unsigned)GetXSurfacePitch(linepattern)
									);
		p1 = new pattern_src_brightness_to_alpha_rgba8(*pattern);
		patt = new pattern_type(fltr);
		patt->create(*p1);
	}
	LinePattern(HBITMAP hBmp)
	{
		patt = NULL;
		p1 = NULL;
		pattern = NULL;
		linepattern = NULL;
		if (!hBmp)
			return;

		linepattern = ::CreateXSurface(hBmp);
		if( !linepattern)
			return;

		pattern = new agg::rendering_buffer((unsigned char*)::GetXSurfacebuffer(linepattern),
			(unsigned)GetXSurfaceWidth(linepattern),
			(unsigned)GetXSurfaceHeight(linepattern),
			(unsigned)GetXSurfacePitch(linepattern)
			);
		p1 = new pattern_src_brightness_to_alpha_rgba8(*pattern);
		patt = new pattern_type(fltr);
		patt->create(*p1);
	}
#endif // OS_WIN
	~LinePattern()
	{
		if (patt)
		{
			delete patt;
			patt = NULL;
		}
		if (p1)
		{
			delete p1;
			p1 = NULL;
		}
		if (pattern)
		{
			delete pattern;
			pattern = NULL;
		}
		if (linepattern)
		{
			::CloseXSurface(linepattern);
			linepattern = NULL;
		}
	}
	void draw_poly(agg::renderer_base<pixfmt> renb,const double* polyline,int num_points)
	{
		renderer_img_type ren_img(renb, *patt);
		rasterizer_img_type ras_img(ren_img);
		agg::poly_plain_adaptor<double> vs(polyline, num_points, false);
		agg::conv_transform<agg::poly_plain_adaptor<double> > trans(vs, scale);
		ras_img.add_path(trans);
	}

};
#include <map>
static std::map<int, LinePattern*> linePatterns; //
static std::map<int, PXSurface> fillPatterns; //
void DrawPolyUsingPattern(PXSurface surface, const POINT *pt, const int& np, PXSurface pSrcSurface, bool bPolygon)
{
	//in: surface to draw line, point array, number of points, resource id
	LinePattern* pPattern = new LinePattern(pSrcSurface);
	if (!pPattern)
		return;
	if (!pPattern->pattern)
	{
		delete pPattern;
		return;
	}

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(surface),
		(unsigned)GetXSurfaceWidth(surface),
		(unsigned)GetXSurfaceHeight(surface),
		(unsigned)GetXSurfacePitch(surface)
		);	
	typedef agg::pixfmt_rgb565 pixfmt;
	pixfmt pixf(rendbuff);
	typedef agg::renderer_base<pixfmt> ren_base;
	ren_base renb(pixf);
	int nNum = np;
	if (bPolygon)
		nNum++;
	double *poly = new double[nNum<<1];
	for (int i=0; i<np; i++)
	{
		poly[i<<1]=pt[i].x;
		poly[(i<<1)+1]=pt[i].y;
	}
	if (bPolygon)
	{
		poly[np<<1] = pt[0].x;
		poly[(np<<1)+1] = pt[0].y;
	}
	renb.clip_box(0,0,GetXSurfaceWidth(surface)-1,GetXSurfaceHeight(surface)-1);	
	pPattern->draw_poly(renb,poly,nNum);
	delete []poly;
	delete pPattern;
}

#ifdef OS_WIN
void DrawPolyUsingPattern(PXSurface surface, const POINT *pt, const int& np, const int& iResourceID, bool bPolygon)
{
	//in: surface to draw line, point array, number of points, resource id
	if(linePatterns.find(iResourceID) == linePatterns.end())
	{
		LinePattern* pPattern = new LinePattern(iResourceID);
		if (!pPattern)
			return;
		if (!pPattern->pattern)
		{
			delete pPattern;
			return;
		}
		linePatterns[iResourceID] = pPattern;
	}
	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(surface),
									(unsigned)GetXSurfaceWidth(surface),
									(unsigned)GetXSurfaceHeight(surface),
									(unsigned)GetXSurfacePitch(surface)
									);	
	typedef agg::pixfmt_rgb565 pixfmt;
	pixfmt pixf(rendbuff);
	typedef agg::renderer_base<pixfmt> ren_base;
	ren_base renb(pixf);
	int nNum = np;
	if (bPolygon)
		nNum++;
	double *poly = new double[nNum<<1];
	for (int i=0; i<np; i++)
	{
		poly[i<<1]=pt[i].x;
		poly[(i<<1)+1]=pt[i].y;
	}
	if (bPolygon)
	{
		poly[np<<1] = pt[0].x;
		poly[(np<<1)+1] = pt[0].y;
	}
	renb.clip_box(0,0,GetXSurfaceWidth(surface)-1,GetXSurfaceHeight(surface)-1);
	linePatterns[iResourceID]->draw_poly(renb,poly,nNum);
	delete []poly;	
}

void DrawPolyUsingPattern(PXSurface surface, const POINT *pt, const int& np, HBITMAP hBm, bool bPolygon)
{
	//in: surface to draw line, point array, number of points, resource id
	LinePattern* pPattern = new LinePattern(hBm);
	if (!pPattern)
		return;
	if (!pPattern->pattern)
	{
		delete pPattern;
		return;
	}

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(surface),
		(unsigned)GetXSurfaceWidth(surface),
		(unsigned)GetXSurfaceHeight(surface),
		(unsigned)GetXSurfacePitch(surface)
		);	
	typedef agg::pixfmt_rgb565 pixfmt;
	pixfmt pixf(rendbuff);
	typedef agg::renderer_base<pixfmt> ren_base;
	ren_base renb(pixf);
	int nNum = np;
	if (bPolygon)
		nNum++;
	double *poly = new double[nNum<<1];
	for (int i=0; i<np; i++)
	{
		poly[i<<1]=pt[i].x;
		poly[(i<<1)+1]=pt[i].y;
	}
	if (bPolygon)
	{
		poly[np<<1] = pt[0].x;
		poly[(np<<1)+1] = pt[0].y;
	}
	renb.clip_box(0,0,GetXSurfaceWidth(surface)-1,GetXSurfaceHeight(surface)-1);	
	pPattern->draw_poly(renb,poly,nNum);
	delete []poly;
	delete pPattern;
}
#endif // OS_WIN

void PolygonEx(PXSurface surface,const POINT *pt, int np, int w, DWORD color)
{
	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(surface),
		(unsigned)GetXSurfaceWidth(surface),
		(unsigned)GetXSurfaceHeight(surface),
		(unsigned)GetXSurfacePitch(surface)
		);	
	typedef agg::pixfmt_rgb565 pixfmt;
	pixfmt pixf(rendbuff);
	typedef agg::renderer_base<pixfmt> ren_base;
	ren_base renb(pixf);
	agg::path_storage path;
	agg::scanline_p8 sl;
	path.move_to(pt->x,pt->y);	
	for(int i=1;i<np;i++)
	{
		path.line_to(pt[i].x,pt[i].y);
	}	
	agg::conv_stroke<agg::path_storage> stroke(path);
	stroke.line_join(round_join);
	stroke.line_cap(round_cap);
	//stroke.miter_limit(m_miter_limit.value());
	stroke.width((double)w);
	agg::rasterizer_scanline_aa<> ras;
	ras.add_path(stroke);
	agg::render_scanlines_aa_solid(ras, sl, renb, agg::rgba((double)GetRValue(color)/255, (double)GetGValue(color)/255, (double)GetBValue(color)/255,double(color>>24)/255));
}

#ifndef USING_AGG_FOR_MAP
#include "./agg/src/agg_arc.cpp"
#include "./agg/src/agg_curves.cpp"
#include "./agg/src/agg_line_aa_basics.cpp"
#include "./agg/src/agg_line_profile_aa.cpp"
#include "./agg/src/agg_trans_affine.cpp"
#ifndef	USING_AGG_ARROW
#include "./agg/src/agg_vcgen_stroke.cpp"
#endif//#ifndef	USING_AGG_ARROW
#endif // USING_AGG_FOR_MAP
