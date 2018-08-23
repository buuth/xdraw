#include "AggUtil.h"

using namespace agg;
#ifdef USING_AGG_FOR_MAP

bool _AGG_PATTERN::Create(int type, const COLORREF& color, uint8_t alpha)
{
	if (type<=0 || type>=AGG_PATTERN_NUM)
		return false;

	m_nWidth = 8;
	m_nHeight = 8;
	color_type agg_bgcolor = agg::rgba8(0, 0, 0, 0);
	color_type agg_color = agg::rgba8(GetRValue(color), GetGValue(color), GetBValue(color), alpha);
	m_pPattern = new agg::int8u[m_nWidth * m_nHeight * 4];
	if (m_pPattern==NULL)
	{
		m_nWidth = 0;
		m_nHeight = 0;
		return false;
	}

	// Clear
	for (int nRow = 0; nRow<m_nHeight; nRow++)
	{
		agg::int8u* pRow = m_pPattern + nRow*m_nWidth*4;
		for (int nCol=0; nCol<m_nWidth; nCol++,pRow+=4)
			*((color_type*)pRow) = agg_bgcolor;
	}

	// Create pattern template
	switch (type)
	{
	case AGG_PATTERN_SOLID:
		{
			for (int nRow = 0; nRow<m_nHeight; nRow++)
			{
				agg::int8u* pRow = m_pPattern + nRow*m_nWidth*4;
				for (int nCol=0; nCol<m_nWidth; nCol++, pRow+=4)
					*((color_type*)pRow) = agg_color;
			}
		}
		break;

	case AGG_PATTERN_FDIAGONAL:
		{
			for (int nRow = 0; nRow<m_nHeight; nRow++)
			{
				agg::int8u* pRow = m_pPattern + nRow*m_nWidth*4;
				pRow = pRow + (m_nWidth-nRow-1)*4;
				*((color_type*)pRow) = agg_color;
			}
		}
		break;

	case AGG_PATTERN_CROSS:
		{
			int nHalf = m_nHeight/2;
			agg::int8u* pRow = m_pPattern + nHalf*m_nWidth*4;
			for (int nCol = 0; nCol<m_nWidth; nCol++, pRow+=4)
				*((color_type*)pRow) = agg_color;
			
			pRow = m_pPattern + nHalf*4;
			for (int nRow=0; nRow<m_nHeight; nRow++,pRow+=m_nWidth*4)
				if (nRow!=nHalf)
					*((color_type*)pRow) = agg_color;
		}
		break;

	case AGG_PATTERN_DIAGCROSS:
		{
			for (int nRow = 0; nRow<m_nHeight; nRow++)
			{
				agg::int8u* pRow = m_pPattern + nRow*m_nWidth*4;
				pRow = pRow + (m_nHeight-nRow-1)*4;
				*((color_type*)pRow) = agg_color;
			}

			for (int nRow = 0; nRow<m_nHeight; nRow++)
			{
				agg::int8u* pRow = m_pPattern + nRow*m_nWidth*4;
				pRow = pRow + nRow*4;
				*((color_type*)pRow) = agg_color;
			}
		}
		break;
	
	case AGG_PATTERN_BDIAGONAL:
		{
			for (int nRow = 0; nRow<m_nHeight; nRow++)
			{
				agg::int8u* pRow = m_pPattern + nRow*m_nWidth*4;
				pRow = pRow + nRow*4;
				*((color_type*)pRow) = agg_color;
			}
		}
		break;

	case AGG_PATTERN_HORIZONTAL:
		{
			agg::int8u* pRow = m_pPattern + (m_nHeight/2)*m_nWidth*4;
			for (int nCol=0; nCol<m_nWidth; nCol++, pRow+=4)
				*((color_type*)pRow) = agg_color;
		}
		break;

	case AGG_PATTERN_VERTICAL:
		{
				agg::int8u* pRow = m_pPattern + (m_nWidth/2)*4;
				for (int nRow=0; nRow<m_nHeight; nRow++, pRow+=m_nWidth*4)
					*((color_type*)pRow) = agg_color;
		}
		break;
	}

	// Attach to agg variables
	m_PatternRbuf.attach(m_pPattern, m_nWidth, m_nHeight, m_nWidth*4);
	m_PatternPixf.attach(m_PatternRbuf);
	m_PatternSrc.attach(m_PatternPixf);
	m_Sg.attach(m_PatternSrc);
	m_Sg.offset_x(m_nWidth);
	m_Sg.offset_y(m_nHeight);

	return true;
}

bool _AGG_PATTERN::Create(PXSurface pxPattern, uint8_t alpha)
{
	if (!pxPattern)
		return false;

	m_nWidth = ::GetXSurfaceWidth(pxPattern);
	m_nHeight = ::GetXSurfaceHeight(pxPattern);
	int nPitch = GetXSurfacePitch(pxPattern);

	m_pPattern = new agg::int8u[m_nWidth * m_nHeight * 4];
	if (!m_pPattern)
	{
		m_nWidth = 0;
		m_nHeight = 0;
		return false;
	}

	BYTE* pSrcRow = (BYTE*)GetXSurfacebuffer(pxPattern);
	color_type* pPtRow = (color_type*)m_pPattern;
	for (int nRow=0; nRow<m_nHeight; nRow++)
	{
		Pixel* pTemp = (Pixel*)pSrcRow;
		int nTemp = m_nWidth;
		while (nTemp)
		{
			*pPtRow = agg::rgba8(_GetRValue(*pTemp), _GetGValue(*pTemp), _GetBValue(*pTemp), alpha);
			pTemp++;
			pPtRow++;
			nTemp--;
		}
		pSrcRow += nPitch;
	}

	// Attach to agg variables
	m_PatternRbuf.attach(m_pPattern, m_nWidth, m_nHeight, m_nWidth*4);
	m_PatternPixf.attach(m_PatternRbuf);
	m_PatternSrc.attach(m_PatternPixf);
	m_Sg.attach(m_PatternSrc);
	m_Sg.offset_x(m_nWidth);
	m_Sg.offset_y(m_nHeight);

	return true;
}

void _AGG_PATTERN::Delete()
{
	if (m_pPattern)
	{
		delete [] m_pPattern;
		m_pPattern = NULL;
	}
	m_nWidth = 0;
	m_nHeight = 0;
}

void AGG_DrawLine(PXSurface xsurface, int x1, int y1, int x2, int y2, int w, const COLORREF& color, uint8_t alpha)
{
	agg::path_storage path;
	path.move_to(x1, y1);	
	path.line_to(x2, y2);
	agg::conv_stroke<agg::path_storage> stroke(path);
	stroke.width((double)w);

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(stroke);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::scanline_p8 sl;
	agg::render_scanlines_aa_solid(ras, sl, renb, agg::rgba8(GetRValue(color), GetGValue(color), GetBValue(color), alpha));
}

void AGG_DrawLineDash(PXSurface xsurface, int x1, int y1, int x2, int y2, int w, const COLORREF& color, const int pattern[], const int np, uint8_t alpha)
{
	agg::path_storage path;
	path.move_to(x1, y1);	
	path.line_to(x2, y2);
	agg::conv_dash<agg::path_storage> dash(path);

	agg::conv_stroke<agg::conv_dash<agg::path_storage>> stroke(dash);
	stroke.width((double)w);

	for (int i=0; i<np/2; i++)
		dash.add_dash(pattern[2*i], pattern[2*i+1]);
	if (np%2)
		dash.add_dash(pattern[np-1], 0);

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(stroke);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::renderer_scanline_aa_solid<agg::renderer_base<pixfmt>> ren_sl(renb);
	ren_sl.color(agg::rgba8(GetRValue(color), GetGValue(color), GetBValue(color), alpha));
	agg::scanline_p8 sl;
	agg::render_scanlines(ras, sl, ren_sl);
}

void AGG_DrawLineOutline(PXSurface xsurface, int x1, int y1, int x2, int y2, int w, const COLORREF& color, uint8_t alpha)
{
	agg::path_storage path;
	path.move_to(x1, y1);
	path.line_to(x2, y2);

	agg::conv_stroke<agg::path_storage> stroke(path);
	stroke.miter_limit(4.0);
	stroke.line_join(agg::round_join);
	stroke.line_cap(agg::round_cap);
	stroke.width((double)w);

	agg::conv_stroke<agg::conv_stroke<agg::path_storage>> poly_outline(stroke);
	poly_outline.line_join(agg::round_join);
	poly_outline.line_cap(agg::round_cap);
	poly_outline.width(1.4);

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(poly_outline);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::scanline_p8 sl;
	agg::render_scanlines_aa_solid(ras, sl, renb, agg::rgba8(GetRValue(color), GetGValue(color), GetBValue(color), alpha));
}

void AGG_DrawLineInline(PXSurface xsurface, int x1, int y1, int x2, int y2, int w, const COLORREF& color, uint8_t alpha)
{
	agg::path_storage path;
	path.move_to(x1, y1);
	path.line_to(x2, y2);

	agg::conv_stroke<agg::path_storage> stroke(path);
	stroke.line_join(round_join);
	stroke.line_cap(round_cap);
	stroke.width((double)(w-1));

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(stroke);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::scanline_p8 sl;
	agg::render_scanlines_aa_solid(ras, sl, renb, agg::rgba8(GetRValue(color), GetGValue(color), GetBValue(color), alpha));
}

void AGG_DrawPoly(PXSurface xsurface, POINT *pPts, int nPt, int w, const COLORREF& color, bool bClosed, uint8_t alpha)
{
	agg::path_storage path;
	path.move_to(pPts[0].x, pPts[0].y);
	for (int i=1; i<nPt; i++)
		path.line_to(pPts[i].x, pPts[i].y);
	if (bClosed)
		path.close_polygon();

	agg::conv_stroke<agg::path_storage> stroke(path);
	stroke.line_join(round_join);
	stroke.line_cap(round_cap);
	stroke.width((double)w);

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(stroke);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::scanline_p8 sl;
	agg::render_scanlines_aa_solid(ras, sl, renb, agg::rgba8(GetRValue(color), GetGValue(color), GetBValue(color), alpha));
}

void AGG_DrawPolyDash(PXSurface xsurface, POINT *pPts, int nPt, int w, const COLORREF& color, const int pattern[], const int np, bool bClosed, uint8_t alpha)
{
	agg::path_storage path;
	path.move_to(pPts[0].x, pPts[0].y);	
	for (int i=1; i<nPt; i++)
		path.line_to(pPts[i].x, pPts[i].y);
	if (bClosed)
		path.close_polygon();
	agg::conv_dash<agg::path_storage> dash(path);

	agg::conv_stroke<agg::conv_dash<agg::path_storage>> stroke(dash);
	stroke.width((double)w);

	for (int i=0; i<np/2; i++)
		dash.add_dash(pattern[2*i], pattern[2*i+1]);
	if (np%2)
		dash.add_dash(pattern[np-1], 0);

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(stroke);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::renderer_scanline_aa_solid<agg::renderer_base<pixfmt>> ren_sl(renb);
	ren_sl.color(agg::rgba8(GetRValue(color), GetGValue(color), GetBValue(color), alpha));
	agg::scanline_p8 sl;
	agg::render_scanlines(ras, sl, ren_sl);
}

void AGG_FillPoly(PXSurface xsurface, POINT *pPts, int nPt, const COLORREF& color, uint8_t alpha)
{
	agg::path_storage path;
	path.move_to(pPts[0].x, pPts[0].y);
	for (int i=1; i<nPt; i++)
		path.line_to(pPts[i].x, pPts[i].y);
	path.close_polygon();

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(path);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::renderer_scanline_bin_solid<agg::renderer_base<pixfmt>> renderer_bin(renb);
	renderer_bin.color(agg::rgba8(GetRValue(color), GetGValue(color), GetBValue(color), alpha));
	agg::scanline_bin sl_bin;
	agg::render_scanlines(ras, sl_bin, renderer_bin);
}

void AGG_FillPolyEx(PXSurface xsurface, POINT *pPts, int nPt, const COLORREF& color, uint8_t alpha)
{
	agg::path_storage path;
	path.move_to(pPts[0].x, pPts[0].y);
	for (int i=1; i<nPt; i++)
		path.line_to(pPts[i].x, pPts[i].y);
	path.close_polygon();

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(path);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::renderer_scanline_aa_solid<agg::renderer_base<pixfmt>> renderer_aa(renb);
	renderer_aa.color(agg::rgba8(GetRValue(color), GetGValue(color), GetBValue(color), alpha));
	agg::scanline_p8 sl_p8;
	agg::render_scanlines(ras, sl_p8, renderer_aa);
}

void AGG_FillPolyPattern(PXSurface xsurface, POINT *pPts, int nPt, const COLORREF& color, int type, uint8_t alpha)
{
	if (type==AGG_PATTERN_SOLID)
	{
		AGG_FillPoly(xsurface, pPts, nPt, color, alpha);
		return;
	}

	AGG_PATTERN aggPattern;
	aggPattern.Create(type, color, alpha);
	if (!aggPattern.IsValid())
		return;

	agg::path_storage path;
	path.move_to(pPts[0].x, pPts[0].y);
	for (int i=1; i<nPt; i++)
		path.line_to(pPts[i].x, pPts[i].y);
	path.close_polygon();
	
	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(path);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::scanline_p8 sl;
	agg::span_allocator<color_type> sa;	

	agg::render_scanlines_aa(ras, sl, renb, sa, aggPattern.m_Sg);

	aggPattern.Delete();
}

void AGG_FillPolyPattern(PXSurface xsurface, POINT *pPts, int nPt, PXSurface pxPattern, uint8_t alpha)
{
	AGG_PATTERN aggPattern;
	aggPattern.Create(pxPattern, alpha);
	if (!aggPattern.IsValid())
		return;

	agg::path_storage path;
	path.move_to(pPts[0].x, pPts[0].y);
	for (int i=1; i<nPt; i++)
		path.line_to(pPts[i].x, pPts[i].y);
	path.close_polygon();

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(path);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::scanline_p8 sl;
	agg::span_allocator<color_type> sa;	

	agg::render_scanlines_aa(ras, sl, renb, sa, aggPattern.m_Sg);

	aggPattern.Delete();
}

void AGG_FillPolyPattern(PXSurface xsurface, POINT *pPts, int nPt, HBITMAP hBitmap, uint8_t alpha)
{
	if (!hBitmap)
		return;
	PXSurface pxPattern = ::CreateXSurface(hBitmap);
	if (pxPattern)
	{
		AGG_FillPolyPattern(xsurface, pPts, nPt, pxPattern, alpha);
		::CloseXSurface(pxPattern);
	}
}

void AGG_DrawPolyOutLine(PXSurface xsurface, POINT *pPts, int nPt, int w, const COLORREF& color, uint8_t alpha)
{
	agg::path_storage path;
	path.move_to(pPts[0].x, pPts[0].y);
	for (int i=1; i<nPt; i++)
		path.line_to(pPts[i].x, pPts[i].y);

	agg::conv_stroke<agg::path_storage> stroke(path);
	stroke.miter_limit(4.0);
	stroke.line_join(agg::round_join);
	stroke.line_cap(agg::round_cap);//
	stroke.width((double)w);

	agg::conv_stroke<agg::conv_stroke<agg::path_storage>> poly_outline(stroke);
	poly_outline.line_join(agg::round_join);
	poly_outline.line_cap(agg::round_cap);//
	poly_outline.width(1.4);

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(poly_outline);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::scanline_p8 sl;
	agg::render_scanlines_aa_solid(ras, sl, renb, agg::rgba8(GetRValue(color), GetGValue(color), GetBValue(color), alpha));
}

void AGG_DrawPolyOL(PXSurface xsurface, POINT *pPts, int nPt, int w, const COLORREF& color, uint8_t alpha)
{
	agg::path_storage path;
	path.move_to(pPts[0].x, pPts[0].y);
	for (int i=1; i<nPt; i++)
		path.line_to(pPts[i].x, pPts[i].y);

	agg::conv_stroke<agg::path_storage> stroke(path);
	stroke.miter_limit(4.0);
	stroke.line_join(agg::round_join);
	stroke.line_cap(agg::butt_cap);//
	stroke.width((double)w);

	agg::conv_stroke<agg::conv_stroke<agg::path_storage>> poly_outline(stroke);
	poly_outline.line_join(agg::round_join);
	poly_outline.line_cap(agg::butt_cap);//
	poly_outline.width(1.4);

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(poly_outline);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::scanline_p8 sl;
	agg::render_scanlines_aa_solid(ras, sl, renb, agg::rgba8(GetRValue(color), GetGValue(color), GetBValue(color), alpha));
}


void AGG_DrawPolyInLine(PXSurface xsurface, POINT *pPts, int nPt, int w, const COLORREF& color, uint8_t alpha)
{
	agg::path_storage path;
	path.move_to(pPts[0].x, pPts[0].y);
	for (int i=1; i<nPt; i++)
		path.line_to(pPts[i].x, pPts[i].y);

	agg::conv_stroke<agg::path_storage> stroke(path);
	stroke.line_join(round_join);
	stroke.line_cap(round_cap);
	stroke.width((double)(w-1));

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(stroke);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::scanline_p8 sl;
	agg::render_scanlines_aa_solid(ras, sl, renb, agg::rgba8(GetRValue(color), GetGValue(color), GetBValue(color), alpha));
}

void AGG_DrawEllipse(PXSurface xsurface, int x, int y, int rx, int ry, int w, const COLORREF& color, uint8_t alpha)
{
	agg::ellipse e(x, y, rx, ry);
	agg::conv_stroke<agg::ellipse> p(e);
	p.width(w);

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(p);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::scanline_p8 sl;
	agg::render_scanlines_aa_solid(ras, sl, renb, agg::rgba8(GetRValue(color), GetGValue(color), GetBValue(color), alpha));
}

void AGG_DrawEllipseDash(PXSurface xsurface, int x, int y, int rx, int ry, int w, const COLORREF& color, const int pattern[], const int np, uint8_t alpha)
{
	agg::ellipse e(x, y, rx, ry);
	agg::conv_dash<agg::ellipse> dash(e);

	agg::conv_stroke<agg::conv_dash<agg::ellipse>> stroke(dash);
	stroke.width((double)w);

	for (int i=0; i<np/2; i++)
		dash.add_dash(pattern[2*i], pattern[2*i+1]);
	if (np%2)
		dash.add_dash(pattern[np-1], 0);

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(stroke);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::renderer_scanline_aa_solid<agg::renderer_base<pixfmt>> ren_sl(renb);
	ren_sl.color(agg::rgba8(GetRValue(color), GetGValue(color), GetBValue(color), alpha));
	agg::scanline_p8 sl;
	agg::render_scanlines(ras, sl, ren_sl);
}

void AGG_FillEllipse(PXSurface xsurface, int x, int y, int rx, int ry, const COLORREF& color, uint8_t alpha)
{
	agg::ellipse e(x, y, rx, ry);

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(e);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::renderer_scanline_bin_solid<agg::renderer_base<pixfmt>> renderer_bin(renb);
	renderer_bin.color(agg::rgba8(GetRValue(color), GetGValue(color), GetBValue(color), alpha));
	agg::scanline_bin sl_bin;
	agg::render_scanlines(ras, sl_bin, renderer_bin);
}

void AGG_FillEllipsePattern(PXSurface xsurface, int x, int y, int rx, int ry, const COLORREF& color, int type, uint8_t alpha)
{
	if (type==AGG_PATTERN_SOLID)
	{
		AGG_FillEllipse(xsurface, x, y, rx, ry, color, alpha);
		return;
	}

	AGG_PATTERN aggPattern;
	aggPattern.Create(type, color, alpha);
	if (!aggPattern.IsValid())
		return;

	agg::ellipse e(x, y, rx, ry);
	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(e);

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::scanline_p8 sl;
	agg::span_allocator<color_type> sa;	

	agg::render_scanlines_aa(ras, sl, renb, sa, aggPattern.m_Sg);

	aggPattern.Delete();
}
#endif // USE_AGG_TO_DRAWROAD

#ifdef USING_AGG_ARROW
void AGG_DrawArrow(PXSurface xsurface, POINT *pPts, int nPt, int nArrowWid, uint8_t alpha, const COLORREF& clrArrow, int nArrHeadLen, int nArrHeadWid, bool bHaveTail)
{
	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::rgba8 aggColor = agg::rgba8(GetRValue(clrArrow), GetGValue(clrArrow), GetBValue(clrArrow), alpha);

	agg::path_storage path;
	path.move_to(pPts[0].x, pPts[0].y);
	for (int i=1; i<nPt; i++)
		path.line_to(pPts[i].x, pPts[i].y);

	agg::conv_stroke<agg::path_storage, agg::vcgen_markers_term> stroke(path);
	stroke.line_join(agg::round_join);
	stroke.width((double)nArrowWid);

	agg::arrowhead ah;
	ah.head(nArrHeadLen, nArrHeadLen, nArrHeadWid, 0);
	if (bHaveTail)
	{
		double dTailWid = ((double)nArrowWid)/2;
		ah.tail(dTailWid, dTailWid, dTailWid, dTailWid);
	}
	agg::conv_marker<agg::vcgen_markers_term, agg::arrowhead> arrow(stroke.markers(), ah);

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(stroke);
	ras.add_path(arrow);

	agg::scanline_p8 sl;
	agg::render_scanlines_aa_solid(ras, sl, renb, aggColor);
}

void AGG_DrawArrowOnLine(PXSurface xsurface, const POINT& pt1, const POINT& pt2, uint8_t alpha, const COLORREF& clrArrow, int nArrowLen, int nArrHeadLen, int nArrHeadWid)
{
	int ux = pt2.x - pt1.x;
	int uy = pt2.y - pt1.y;
	int nLineLen = julery_isqrt(ux*ux+uy*uy);
	if (!nLineLen)
		return;

	POINT ptStart, ptEnd;
	int nTempLen = (nLineLen-nArrowLen)/2;
	ptStart.x =pt1.x + nTempLen*ux/nLineLen;
	ptStart.y = pt1.y + nTempLen*uy/nLineLen;
	nTempLen += nArrowLen;
	ptEnd.x = pt1.x + nTempLen*ux/nLineLen;
	ptEnd.y = pt1.y + nTempLen*uy/nLineLen;

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::rgba8 aggColor = agg::rgba8(GetRValue(clrArrow), GetGValue(clrArrow), GetBValue(clrArrow), alpha);

	agg::path_storage path;
	path.move_to(ptStart.x, ptStart.y);
	path.line_to(ptEnd.x, ptEnd.y);

	agg::conv_stroke<agg::path_storage, agg::vcgen_markers_term> stroke(path);
	stroke.width((double)2);

	agg::arrowhead ah;
	ah.head(nArrHeadLen, nArrHeadLen, nArrHeadWid+1, nArrHeadWid);
	agg::conv_marker<agg::vcgen_markers_term, agg::arrowhead> arrow(stroke.markers(), ah);

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP
	ras.add_path(stroke);
	ras.add_path(arrow);

	agg::scanline_p8 sl;
	agg::render_scanlines_aa_solid(ras, sl, renb, aggColor);
}

void AGG_DrawArrowOnPoly(PXSurface xsurface, POINT *pPts, int nPt, bool bBack, uint8_t alpha, const COLORREF& clrArrow, int nArrowLen, int nArrowOff, int nArrHeadLen, int nArrHeadWid)
{
	int i;

	int nIdx = 0;
	int nIdxOff = 1;
	if (bBack)
	{
		nIdx = nPt - 1;
		nIdxOff = -1;
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
	if (!nArrowNum)
		return;

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::rgba8 aggColor = agg::rgba8(GetRValue(clrArrow), GetGValue(clrArrow), GetBValue(clrArrow), alpha);

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP

	int nArrLen, nRemainLen;
	int nLen = 0;
	POINT pt1, pt2;
	int nArrIdx = 0;
	int nPntIdx = nIdx;
	int nPntNextId;
	bool bContinue = (nArrIdx<nArrowNum && ((bBack && nPntIdx>0) || (!bBack && nPntIdx<nPt-1)) && nCurLen>0);
	while (bContinue)
	{
		nPntNextId = nPntIdx + nIdxOff;
		ux = pPts[nPntNextId].x - pPts[nPntIdx].x;
		uy = pPts[nPntNextId].y - pPts[nPntIdx].y;
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

		agg::path_storage path;
		path.move_to(pt1.x, pt1.y);
		while (nRemainLen<nArrLen && ((bBack && nPntIdx>1) || (!bBack && nPntIdx<nPt-2)))
		{
			path.line_to(pPts[nPntNextId].x, pPts[nPntNextId].y);

			nPntIdx = nPntNextId;
			nPntNextId = nPntIdx + nIdxOff;
			pt1 = pPts[nPntIdx];
			ux = pPts[nPntNextId].x - pPts[nPntIdx].x;
			uy = pPts[nPntNextId].y - pPts[nPntIdx].y;
			nLen = julery_isqrt(ux*ux+uy*uy);
			nArrLen -= nRemainLen;
			nRemainLen = nLen;
			nCurLen = 0;
		}
		if (nLen==0)
			break;
		pt2.x = pt1.x + nArrLen*ux/nLen;
		pt2.y = pt1.y + nArrLen*uy/nLen;
		path.line_to(pt2.x, pt2.y);

		agg::conv_stroke<agg::path_storage, agg::vcgen_markers_term> stroke(path);
		stroke.line_join(agg::round_join);
		stroke.line_cap(agg::round_cap);
		stroke.width((double)2);

		agg::arrowhead ah;
		// 		double k = ::pow(2, 0.7);
		// 		//ah.head(4*k, 4*k, 3*k, 2*k);
		// 		ah.head((nArrHeadLen-1)*k, (nArrHeadLen-1)*k, nArrHeadWid*k, k);
		ah.head(nArrHeadLen, nArrHeadLen, nArrHeadWid+1, nArrHeadWid);
		agg::conv_marker<agg::vcgen_markers_term, agg::arrowhead> arrow(stroke.markers(), ah);

		ras.reset();
		ras.add_path(stroke);
		ras.add_path(arrow);

		agg::scanline_p8 sl;
		agg::render_scanlines_aa_solid(ras, sl, renb, aggColor);

		nArrIdx++;
		nCurLen += nArrowLen + nArrowOff;
		bContinue = (nArrIdx<nArrowNum && ((bBack && nPntIdx>0) || (!bBack && nPntIdx<nPt-1)) && nCurLen>0);
	}
}

void AGG_DrawArrowOnRoad(PXSurface xsurface, POINT *pPts, int nPt, bool bBack, uint8_t alpha, const COLORREF& clrArrow, int nArrowLen, int nArrowOff, int nArrHeadLen, int nArrHeadWid)
{
	int nIdx = 0;
	int nIdxOff = 1;
	if (bBack)
	{
		nIdx = nPt - 1;
		nIdxOff = -1;
	}
	bool bArrowDraw = false;

	agg::rendering_buffer rendbuff((unsigned char*)::GetXSurfacebuffer(xsurface), (unsigned)GetXSurfaceWidth(xsurface), (unsigned)GetXSurfaceHeight(xsurface), (unsigned)GetXSurfacePitch(xsurface));
	pixfmt pixf(rendbuff);
	agg::renderer_base<pixfmt> renb(pixf);
	agg::rgba8 aggColor = agg::rgba8(GetRValue(clrArrow), GetGValue(clrArrow), GetBValue(clrArrow), alpha);

	agg::rasterizer_scanline_aa<> ras;
#ifdef AGG_AUTO_CLIP
	ras.clip_box(0, 0, GetXSurfaceWidth(xsurface), GetXSurfaceHeight(xsurface));
#endif // AGG_AUTO_CLIP

	agg::arrowhead ah;
	ah.head(nArrHeadLen, nArrHeadLen, nArrHeadWid+1, nArrHeadWid);

	int nPolyLen = 0;
	int nSecLen = 0;
	int ux, uy;
	int nArrowTotalLen = nArrowLen + nArrowOff;
	int nCheckMinLen = nArrowLen*2;
	int nCheckMaxLen = nArrowLen*5;

	POINT pt1, pt2;
	int nPntIdx = nIdx;
	int nPntNextId;
	int nLenOff;
	int nPreLen =  0;
	bool bContinue = ((bBack && nPntIdx>0) || (!bBack && nPntIdx<nPt-1));
	while (bContinue)
	{
		nPntNextId = nPntIdx + nIdxOff;
		ux = pPts[nPntNextId].x - pPts[nPntIdx].x;
		uy = pPts[nPntNextId].y - pPts[nPntIdx].y;
		nSecLen = julery_isqrt(ux*ux+uy*uy);
		nPolyLen += nSecLen;

		if (nSecLen>nCheckMinLen)
		{
			int nArrNum = 1;
			if (nSecLen>2*nArrowTotalLen)
				nArrNum = nSecLen/nArrowTotalLen;
			else if (bArrowDraw && nPreLen+nSecLen<nArrowOff)
				nArrNum = 0;

			nLenOff = (nSecLen - nArrNum*nArrowLen - (nArrNum-1)*nArrowOff)/2;
			if (nArrNum)
			{
				bArrowDraw = true;
				nPreLen = nLenOff;
			}
			else
				nPreLen += nSecLen;
			for (int i=0; i<nArrNum; i++)
			{
				pt1.x = pPts[nPntIdx].x + nLenOff*ux/nSecLen;
				pt1.y = pPts[nPntIdx].y + nLenOff*uy/nSecLen;
				nLenOff += nArrowLen;
				pt2.x = pPts[nPntIdx].x + nLenOff*ux/nSecLen;
				pt2.y = pPts[nPntIdx].y + nLenOff*uy/nSecLen;
				nLenOff += nArrowOff;

				agg::path_storage path;
				path.move_to(pt1.x, pt1.y);
				path.line_to(pt2.x, pt2.y);
				agg::conv_stroke<agg::path_storage, agg::vcgen_markers_term> stroke(path);
				stroke.width((double)2);

				agg::conv_marker<agg::vcgen_markers_term, agg::arrowhead> arrow(stroke.markers(), ah);

				ras.reset();
				ras.add_path(stroke);
				ras.add_path(arrow);

				agg::scanline_p8 sl;
				agg::render_scanlines_aa_solid(ras, sl, renb, aggColor);
			}
		}
		else
			nPreLen += nSecLen;

		nPntIdx = nPntNextId;
		bContinue = ((bBack && nPntIdx>0) || (!bBack && nPntIdx<nPt-1));
	}

	if (!bArrowDraw && nPolyLen>nCheckMinLen && nPolyLen>nCheckMinLen)
	{
		// Ve mui ten o giua
		agg::path_storage path;
		int nStartLen = (nPolyLen - nArrowLen)>>1;
		int nEndLen = (nPolyLen + nArrowLen)>>1;
		int nCountLen = 0;
		int nCountNextLen = 0;
		for (nPntIdx = 0; nPntIdx<nPt-1; nPntIdx++)
		{
			nPntNextId = nPntIdx + nIdxOff;
			ux = pPts[nPntNextId].x - pPts[nPntIdx].x;
			uy = pPts[nPntNextId].y - pPts[nPntIdx].y;
			nSecLen = julery_isqrt(ux*ux+uy*uy);
			nCountNextLen = nCountLen+nSecLen;
			if (nCountLen<=nStartLen && nCountNextLen>nStartLen)
			{
				nLenOff = nStartLen-nCountLen;
				pt1.x = pPts[nPntIdx].x + nLenOff*ux/nSecLen;
				pt1.y = pPts[nPntIdx].y + nLenOff*uy/nSecLen;
				path.move_to(pt1.x, pt1.y);
			}
			else if (nCountLen>nStartLen && nCountNextLen<nEndLen)
			{
				path.line_to(pPts[nPntIdx].x, pPts[nPntIdx].y);
			}
			else if (nCountLen>=nEndLen && nCountNextLen<nEndLen)
			{
				nLenOff = nEndLen-nCountLen;
				pt1.x = pPts[nPntIdx].x + nLenOff*ux/nSecLen;
				pt1.y = pPts[nPntIdx].y + nLenOff*uy/nSecLen;
				path.move_to(pt1.x, pt1.y);

				agg::conv_stroke<agg::path_storage, agg::vcgen_markers_term> stroke(path);
				stroke.width((double)2);

				agg::conv_marker<agg::vcgen_markers_term, agg::arrowhead> arrow(stroke.markers(), ah);

				ras.reset();
				ras.add_path(stroke);
				ras.add_path(arrow);

				agg::scanline_p8 sl;
				agg::render_scanlines_aa_solid(ras, sl, renb, aggColor);

				break;
			}
			nCountLen = nCountNextLen;
		}
	}
}
#endif // USING_AGG_ARROW

#ifdef USING_AGG_FOR_MAP
#include "./agg_2.5/src/agg_arc.cpp"
#include "./agg_2.5/src/agg_curves.cpp"
#include "./agg_2.5/src/agg_trans_affine.cpp"
#include "./agg_2.5/src/agg_vcgen_dash.cpp"
//#include "./agg_2.5/src/agg_sqrt_tables.cpp"
#endif // USING_AGG_FOR_MAP

#if (defined(USING_AGG_FOR_MAP) || defined(USING_AGG_FOR_ROAD))
#include "./agg_2.5/src/agg_line_aa_basics.cpp"
#include "./agg_2.5/src/agg_line_profile_aa.cpp"
#endif // #if (defined(USING_AGG_FOR_MAP) || defined(USING_AGG_FOR_ROAD))

#if (defined(USING_AGG_FOR_MAP) || defined(USING_AGG_ARROW) || defined(USING_AGG_FOR_ROAD))
#include "./agg_2.5/src/agg_vcgen_stroke.cpp"
#endif // USING_AGG_FOR_MAP, USING_AGG_ARROW

#ifdef USING_AGG_ARROW
#include "./agg_2.5/src/agg_arrowhead.cpp"
#include "./agg_2.5/src/agg_vcgen_markers_term.cpp"
#endif // USING_AGG_ARROW