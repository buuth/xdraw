#ifndef __AGGUTIL_H__
#define	__AGGUTIL_H__


#ifdef USING_FOR_WEB
#define		USING_AGG_FOR_MAP
#endif //#ifdef USING_FOR_WEB

#define		USING_AGG_ARROW

#define		AGG_AUTO_CLIP

#if (defined(USING_AGG_FOR_MAP) || defined(USING_AGG_ARROW))
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgb_packed.h"
#include "agg_renderer_base.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_path_storage.h"
#include "agg_conv_stroke.h"
#include "agg_scanline_p.h"
#endif // USING_AGG_FOR_MAP, USING_AGG_ARROW

#if (defined(USING_AGG_FOR_MAP))
#include "agg_pattern_filters_rgba.h"
#include "agg_renderer_outline_aa.h"
#include "agg_renderer_outline_image.h"
#include "agg_ellipse.h"
#include "agg_scanline_bin.h"
#include "agg_conv_dash.h"
#include "agg_span_allocator.h"
#include "agg_span_pattern_rgba.h"
#include "agg_image_accessors.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_outline_aa.h"
#endif // USING_AGG_FOR_MAP

#ifdef USING_AGG_ARROW
#include "agg_arrowhead.h"
#include "agg_vcgen_markers_term.h"
#include "agg_conv_marker.h"
#endif // USING_AGG_ARROW

#if (defined(USING_AGG_FOR_MAP) || defined(USING_AGG_ARROW))
#include "xdraw.h"
#include "xbuffer.h"

typedef agg::pixfmt_rgb565 pixfmt;
typedef agg::pixfmt_rgb565_pre pixfmt_pre;
typedef agg::rgba8 color_type;

#define _GetRValue(pixel)      ((pixel & RED_MASK)>>8)
#define _GetGValue(pixel)      ((pixel & GREEN_MASK)>>3)
#define _GetBValue(pixel)      ((pixel & BLUE_MASK)<<3)
#endif

#ifdef USING_AGG_FOR_MAP

enum AGG_PATTERN_TYPE
{
	AGG_PATTERN_NULL = 0,	
	AGG_PATTERN_SOLID,
	AGG_PATTERN_FDIAGONAL,
	AGG_PATTERN_CROSS,
	AGG_PATTERN_DIAGCROSS,
	AGG_PATTERN_BDIAGONAL,
	AGG_PATTERN_HORIZONTAL,
	AGG_PATTERN_VERTICAL,
	AGG_PATTERN_NUM
};

typedef struct _AGG_PATTERN
{
	agg::int8u* m_pPattern;
	int m_nWidth;
	int m_nHeight;
	agg::rendering_buffer m_PatternRbuf;
	agg::pixfmt_rgba32 m_PatternPixf;
	agg::image_accessor_wrap<agg::pixfmt_rgba32, agg::wrap_mode_repeat, agg::wrap_mode_repeat> m_PatternSrc;
	agg::span_pattern_rgba<agg::image_accessor_wrap<agg::pixfmt_rgba32, agg::wrap_mode_repeat, agg::wrap_mode_repeat>> m_Sg;

	_AGG_PATTERN()
	{
		m_pPattern = NULL;
		m_nWidth = 0;
		m_nHeight = 0;
	}

	inline bool IsValid() const {return (m_pPattern!=NULL);}

	bool Create(int type, const COLORREF& color, uint8_t alpha=255);
	bool Create(PXSurface pxPattern, uint8_t alpha=255);
	void Delete();
} AGG_PATTERN;

void AGG_DrawLine(PXSurface xsurface, int x1, int y1, int x2, int y2, int w, const COLORREF& color, uint8_t alpha=255);
void AGG_DrawLineDash(PXSurface xsurface, int x1, int y1, int x2, int y2, int w, const COLORREF& color, const int pattern[], const int np, uint8_t alpha=255);
void AGG_DrawLineOutline(PXSurface xsurface, int x1, int y1, int x2, int y2, int w, const COLORREF& color, uint8_t alpha=255);
void AGG_DrawLineInline(PXSurface xsurface, int x1, int y1, int x2, int y2, int w, const COLORREF& color, uint8_t alpha=255);
void AGG_DrawPolyOutLine(PXSurface xsurface, POINT *pPts, int nPt, int w, const COLORREF& color, uint8_t alpha=255);
void AGG_DrawPolyOL(PXSurface xsurface, POINT *pPts, int nPt, int w, const COLORREF& color, uint8_t alpha=255);
void AGG_DrawPolyInLine(PXSurface xsurface, POINT *pPts, int nPt, int w, const COLORREF& color, uint8_t alpha=255);

void AGG_DrawPoly(PXSurface xsurface, POINT *pPts, int nPt, int w, const COLORREF& color, bool bClosed=false, uint8_t alpha=255);
void AGG_DrawPolyDash(PXSurface xsurface, POINT *pPts, int nPt, int w, const COLORREF& color, const int pattern[], const int np, bool bClosed=false, uint8_t alpha=255);
void AGG_FillPoly(PXSurface xsurface, POINT *pPts, int nPt, const COLORREF& color, uint8_t alpha=255);
void AGG_FillPolyEx(PXSurface xsurface, POINT *pPts, int nPt, const COLORREF& color, uint8_t alpha=255);
void AGG_FillPolyPattern(PXSurface xsurface, POINT *pPts, int nPt, const COLORREF& color, int type, uint8_t alpha=255);
void AGG_FillPolyPattern(PXSurface xsurface, POINT *pPts, int nPt, PXSurface pxPattern, uint8_t alpha=255);
void AGG_FillPolyPattern(PXSurface xsurface, POINT *pPts, int nPt, HBITMAP hBitmap, uint8_t alpha=255);

void AGG_DrawEllipse(PXSurface xsurface, int x, int y, int rx, int ry, int w, const COLORREF& color, uint8_t alpha=255);
void AGG_DrawEllipseDash(PXSurface xsurface, int x, int y, int rx, int ry, int w, const COLORREF& color, const int pattern[], const int np, uint8_t alpha=255);
void AGG_FillEllipse(PXSurface xsurface, int x, int y, int rx, int ry, const COLORREF& color, uint8_t alpha=255);
void AGG_FillEllipsePattern(PXSurface xsurface, int x, int y, int rx, int ry, const COLORREF& color, int type, uint8_t alpha=255);

#endif // USING_AGG_FOR_MAP

#ifdef USING_AGG_ARROW
void AGG_DrawArrow(PXSurface xsurface, POINT *pPts, int nPt, int nArrowWid, uint8_t alpha, const COLORREF& clrArrow, int nArrHeadLen, int nArrHeadWid, bool bHaveTail);
void AGG_DrawArrowOnLine(PXSurface xsurface, const POINT& pt1, const POINT& pt2, uint8_t alpha, const COLORREF& clrArrow, int nArrowLen, int nArrHeadLen, int nArrHeadWid);
void AGG_DrawArrowOnPoly(PXSurface xsurface, POINT *pPts, int nPt, bool bBack, uint8_t alpha, const COLORREF& clrArrow, int nArrowLen, int nArrowOff, int nArrHeadLen, int nArrHeadWid);
void AGG_DrawArrowOnRoad(PXSurface xsurface, POINT *pPts, int nPt, bool bBack, uint8_t alpha, const COLORREF& clrArrow, int nArrowLen, int nArrowOff, int nArrHeadLen, int nArrHeadWid);
#endif // USING_AGG_ARROW

#endif // __AGGUTIL_H__

