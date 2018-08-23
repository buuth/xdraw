/*!
 * \file
 * Scanline edge-flag algorithm for antialiasing <br>
 * Copyright (c) 2005-2007 Kiia Kallio
 *
 * http://mlab.uiah.fi/~kkallio/antialiasing/
 * 
 * This code is distributed under the three-clause BSD license.
 * Read the LICENSE file or visit the URL above for details.
 *
 * \brief PolygonFiller implementation version F. Similar to E, but does the rendering one scanline at a time.
 *
 *
 * $Id: $
 * $Date: $
 * $Revision: $
 */
#include "PolygonUnrollB.h"
#include "PolygonVersionF.h"


//! Constructor.
PolygonVersionF::PolygonVersionF()
{
    mMaskBuffer = NULL;
    mWindingBuffer = NULL;
    mEdgeTable = NULL;
	mEdgeStorage = NULL;
    mEdgeCount = 0;
    mCurrentEdge = 0;
	subpoly = NULL;
}


//! Initializer.
/*! aWidth and aHeight define the maximum output size for the filler.
 *  The filler will output to larger bitmaps as well, but the output will
 *  be cropped. aEdgeCount defines the initial amount of edges available for
 *  rendering.
 */
bool PolygonVersionF::init(unsigned int aWidth, unsigned int aHeight, unsigned int aEdgeCount)
{
    // The buffer used for filling needs to be three pixels wider than the bitmap.
    // This is because of three reasons: first, the buffer used for filling needs
    // to be one pixel wider because the end flag that turns the fill off is the
    // first pixel after the actually drawn edge. Second, the edge tracking needs
    // to add the maximum sample offset (i.e. 1) to the end of the edge. This
    // requires one pixel more in the buffer to avoid reading of data from the
    // left edge. Third, the rendering is stopped with an edge marker that is placed
    // after the data, thus requiring one pixel more for the maximum case.
    unsigned int bufferWidth = aWidth + 3;

    mMaskBuffer = new SUBPIXEL_DATA[bufferWidth];
    if (mMaskBuffer == NULL)
        return false;
    MEMSET(mMaskBuffer,0,bufferWidth*sizeof(SUBPIXEL_DATA));

    mWindingBuffer = new NonZeroMask[bufferWidth];
    if (mWindingBuffer == NULL)
        return false;
    MEMSET(mWindingBuffer,0,bufferWidth*sizeof(NonZeroMask));

    mEdgeTable = new PolygonScanEdge*[aHeight];
    if (mEdgeTable == NULL)
        return false;
    MEMSET(mEdgeTable,0,aHeight*sizeof(PolygonScanEdge *));

    mEdgeStorage = new PolygonScanEdge[aEdgeCount];
    if (mEdgeStorage == NULL)
        return false;
    mEdgeCount = aEdgeCount;

    mWidth = aWidth;
    mBufferWidth = bufferWidth;
    mHeight = aHeight;

    mClipRect.setClip(0,0,mWidth,mHeight,SUBPIXEL_COUNT);
	subpoly = new SubPolygon();

    return true;
}


//! Virtual destructor.
PolygonVersionF::~PolygonVersionF()
{	
    if (mMaskBuffer)
		delete [] mMaskBuffer;
	if (mWindingBuffer)
		delete [] mWindingBuffer;
	if (mEdgeTable)
		delete[] mEdgeTable;
	if (mEdgeStorage)
		delete [] mEdgeStorage;	
	if (subpoly)
		delete subpoly;
}


//! Renders the polygon with even-odd fill.
/*! \param aTarget the target bitmap.
 *  \param aPolygon the polygon to render.
 *  \param aColor the color to be used for rendering.
 *  \param aTransformation the transformation matrix.
 */
void PolygonVersionF::renderEvenOdd(BYTE* pBuffer, int nPitch, Vector2d* pts, int nPts, Pixel color)
{    
    // Sets the round down mode in case it has been modified.
    setRoundDownMode();

    mVerticalExtents.reset();
    mCurrentEdge = 0;
    
    bool success = true;
	subpoly->init_poly(pts, nPts);		
    // The maximum amount of edges is 3 x the vertices.
	int freeCount = getFreeEdgeCount() - nPts * 3;
    if (freeCount < 0 && !resizeEdgeStorage(-freeCount))
    {
        success = false;
    }
    else
    {
        PolygonScanEdge *edges = &mEdgeStorage[mCurrentEdge];
		int edgeCount = subpoly->getScanEdges(edges,mClipRect);
        
        int p;
        for (p = 0; p < edgeCount; p++)
        {
            int firstLine = edges[p].mFirstLine >> SUBPIXEL_SHIFT;
            int lastLine = edges[p].mLastLine >> SUBPIXEL_SHIFT;

            edges[p].mNextEdge = mEdgeTable[firstLine];
            mEdgeTable[firstLine] = &edges[p];

            mVerticalExtents.mark(firstLine,lastLine);
        }
        mCurrentEdge += edgeCount;
    }

    if (success)
		fillEvenOdd(pBuffer, nPitch, color);
    else
    {
        unsigned int y;
        for (y = 0; y < mHeight; y++)
            mEdgeTable[y] = NULL;
    }
}

void PolygonVersionF::renderEvenOddEx(BYTE* pBuffer, int nPitch, Vector2d* pPts, int* pVertextIdx, int nVertexCnt, Pixel color)
{    
	// Sets the round down mode in case it has been modified.
	setRoundDownMode();

	mVerticalExtents.reset();
	mCurrentEdge = 0;

	int nPt;
	bool success = true;
	for (int iv=0; iv<nVertexCnt-1; iv++)
	{
		nPt = pVertextIdx[iv+1] - pVertextIdx[iv];
		subpoly->init_poly(pPts + pVertextIdx[iv], nPt);		
		// The maximum amount of edges is 3 x the vertices.
		int freeCount = getFreeEdgeCount() - nPt * 3;
		if (freeCount < 0 && !resizeEdgeStorage(-freeCount))
		{
			success = false;
			break;
		}
		else
		{
			PolygonScanEdge *edges = &mEdgeStorage[mCurrentEdge];
			int edgeCount = subpoly->getScanEdges(edges,mClipRect);

			int p;
			for (p = 0; p < edgeCount; p++)
			{
				int firstLine = edges[p].mFirstLine >> SUBPIXEL_SHIFT;
				int lastLine = edges[p].mLastLine >> SUBPIXEL_SHIFT;

				edges[p].mNextEdge = mEdgeTable[firstLine];
				mEdgeTable[firstLine] = &edges[p];

				mVerticalExtents.mark(firstLine,lastLine);
			}
			mCurrentEdge += edgeCount;
		}
	}

	if (success)
		fillEvenOdd(pBuffer, nPitch, color);
	else
	{
		unsigned int y;
		for (y = 0; y < mHeight; y++)
			mEdgeTable[y] = NULL;
	}
}

//! Renders the polygon with non-zero winding fill.
/*! \param aTarget the target bitmap.
 *  \param aPolygon the polygon to render.
 *  \param aColor the color to be used for rendering.
 *  \param aTransformation the transformation matrix.
 */
void PolygonVersionF::renderNonZeroWinding(BYTE* pBuffer, int nPitch, Vector2d* pts, int nPts, Pixel color)
{   
    // Sets the round down mode in case it has been modified.
    setRoundDownMode();

    mVerticalExtents.reset();
    mCurrentEdge = 0;        
		
    bool success = true;
	subpoly->init_poly(pts, nPts);			
    // The maximum amount of edges is 3 x the vertices.
	int freeCount = getFreeEdgeCount() - nPts * 3;
    if (freeCount < 0 && 
        !resizeEdgeStorage(-freeCount))
    {
        success = false;
    }
    else
    {
        PolygonScanEdge *edges = &mEdgeStorage[mCurrentEdge];		
		int edgeCount = subpoly->getScanEdges(edges,mClipRect);
        
        int p;
        for (p = 0; p < edgeCount; p++)
        {
            int firstLine = edges[p].mFirstLine >> SUBPIXEL_SHIFT;
            int lastLine = edges[p].mLastLine >> SUBPIXEL_SHIFT;

            edges[p].mNextEdge = mEdgeTable[firstLine];
            mEdgeTable[firstLine] = &edges[p];

            mVerticalExtents.mark(firstLine,lastLine);
        }
        mCurrentEdge += edgeCount;
    }

    if (success)
		fillNonZero(pBuffer, nPitch , color, mClipRect);
    else
    {
        unsigned int y;
        for (y = 0; y < mHeight; y++)
            mEdgeTable[y] = NULL;
    }
}


//! Sets the clip rectangle for the polygon filler.
void PolygonVersionF::setClipRect(unsigned int aX, unsigned int aY, unsigned int aWidth, unsigned int aHeight)
{
    unsigned int x = aX;
    unsigned int y = aY;

    if (x > mWidth)
        x = mWidth;

    if (y > mHeight)
        y = mHeight;

    unsigned int maxX = x + aWidth;
    unsigned int maxY = y + aHeight;

    if (maxX > mWidth)
        maxX = mWidth;

    if (maxY > mHeight)
        maxY = mHeight;

    mClipRect.setClip(x, y, maxX - x, maxY - y, SUBPIXEL_COUNT);
}

//! Renders the edges from the current vertical index using even-odd fill.
inline void PolygonVersionF::renderEvenOddEdges(PolygonScanEdge * &aActiveEdgeTable, SpanExtents &aEdgeExtents, int aCurrentLine)
{
    static const FIXED_POINT offsets[SUBPIXEL_COUNT] = SUBPIXEL_OFFSETS_FIXED;

    // First, process edges in the active edge table. These are either full height edges
    // or edges that end before the last line. Remove those that end within the scan line.

    // Then, fetch the edges from the edge table. These are either such that the edge is
    // fully within this scanline or the edge starts from within the scanline and continues
    // to the next scanline. Add those that continue to the next scanline to the active
    // edge table. Clear the edge table.

    PolygonScanEdge *prevEdge = NULL;
    PolygonScanEdge *currentEdge = aActiveEdgeTable;

    while (currentEdge)
    {
        // The plotting of all edges in the AET starts from the top of the scanline.
        // The plotting is divided to two stages: those that end on this scanline and
        // those that span over the full scanline.

        int lastLine = currentEdge->mLastLine >> SUBPIXEL_SHIFT;

        if (lastLine == aCurrentLine)
        {
            FIXED_POINT x = currentEdge->mX;
            FIXED_POINT slope = currentEdge->mSlope;

            // First x value rounded down.
            int xs = FIXED_TO_INT(x);
            int ye = currentEdge->mLastLine & (SUBPIXEL_COUNT - 1);

            SUBPIXEL_DATA mask = 1;
            int ySub;
            for (ySub = 0; ySub <= ye; ySub++)
            {
                int xp = FIXED_TO_INT(x + offsets[ySub]);
                mMaskBuffer[xp] ^= mask;
                mask <<= 1;
                x += slope;
            }

            // Last x-value rounded down.
            int xe = FIXED_TO_INT(x - slope);

            // Mark the span.
            aEdgeExtents.markWithSort(xs,xe);

            // Remove the edge from the active edge table
            currentEdge = currentEdge->mNextEdge;
            if (prevEdge)
                prevEdge->mNextEdge = currentEdge;
            else
                aActiveEdgeTable = currentEdge;
        }
        else
        {
            FIXED_POINT x = currentEdge->mX;
            FIXED_POINT slope = currentEdge->mSlope;

            // First x value rounded down.
            int xs = FIXED_TO_INT(x);
            int xe;

            EVENODD_LINE_UNROLL_INIT();

            EVENODD_LINE_UNROLL_0(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_1(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_2(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_3(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_4(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_5(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_6(mMaskBuffer,x,slope,offsets);
#if SUBPIXEL_COUNT == 8
            xe = FIXED_TO_INT(x);
#endif
            EVENODD_LINE_UNROLL_7(mMaskBuffer,x,slope,offsets);

#if SUBPIXEL_COUNT > 8
            EVENODD_LINE_UNROLL_8(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_9(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_10(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_11(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_12(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_13(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_14(mMaskBuffer,x,slope,offsets);
#if SUBPIXEL_COUNT == 16
            xe = FIXED_TO_INT(x);
#endif
            EVENODD_LINE_UNROLL_15(mMaskBuffer,x,slope,offsets);
#endif

#if SUBPIXEL_COUNT > 16
            EVENODD_LINE_UNROLL_16(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_17(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_18(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_19(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_20(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_21(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_22(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_23(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_24(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_25(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_26(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_27(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_28(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_29(mMaskBuffer,x,slope,offsets);
            EVENODD_LINE_UNROLL_30(mMaskBuffer,x,slope,offsets);
            xe = FIXED_TO_INT(x);
            EVENODD_LINE_UNROLL_31(mMaskBuffer,x,slope,offsets);
#endif

            // Mark the span.
            aEdgeExtents.markWithSort(xs,xe);

            // Update the edge
            if ((aCurrentLine & SLOPE_FIX_SCANLINE_MASK) == 0)
                currentEdge->mX = x + currentEdge->mSlopeFix;
            else
                currentEdge->mX = x;

            // Proceed forward in the AET.
            prevEdge = currentEdge;
            currentEdge = currentEdge->mNextEdge;
        }
    }

    // Fetch edges from the edge table.
    currentEdge = mEdgeTable[aCurrentLine];

    if (currentEdge)
    {
        // Clear the edge table for this line.
        mEdgeTable[aCurrentLine] = NULL;

        do
        {
            // The plotting of all edges in the edge table starts somewhere from the middle
            // of the scanline, and ends either at the end or at the last scanline. This leads
            // to two cases: either the scanning starts and stops within this scanline, or
            // it continues to the next as well.

            int lastLine = currentEdge->mLastLine >> SUBPIXEL_SHIFT;

            if (lastLine == aCurrentLine)
            {
                FIXED_POINT x = currentEdge->mX;
                FIXED_POINT slope = currentEdge->mSlope;

                // First x value rounded down.
                int xs = FIXED_TO_INT(x);

                int ys = currentEdge->mFirstLine & (SUBPIXEL_COUNT - 1);
                int ye = currentEdge->mLastLine & (SUBPIXEL_COUNT - 1);

                SUBPIXEL_DATA mask = 1 << ys;
                int ySub;
                for (ySub = ys; ySub <= ye; ySub++)
                {
                    int xp = FIXED_TO_INT(x + offsets[ySub]);
                    mMaskBuffer[xp] ^= mask;
                    mask <<= 1;
                    x += slope;
                }

                // Last x-value rounded down.
                int xe = FIXED_TO_INT(x - slope);

                // Mark the span.
                aEdgeExtents.markWithSort(xs,xe);

                // Ignore the edge (don't add to AET)
            }
            else
            {
                FIXED_POINT x = currentEdge->mX;
                FIXED_POINT slope = currentEdge->mSlope;

                // First x value rounded down.
                int xs = FIXED_TO_INT(x);

                int ys = currentEdge->mFirstLine & (SUBPIXEL_COUNT - 1);

                SUBPIXEL_DATA mask = 1 << ys;
                int ySub;
                for (ySub = ys; ySub < SUBPIXEL_COUNT; ySub++)
                {
                    int xp = FIXED_TO_INT(x + offsets[ySub]);
                    mMaskBuffer[xp] ^= mask;
                    mask <<= 1;
                    x += slope;
                }

                // Last x-value rounded down.
                int xe = FIXED_TO_INT(x - slope);

                // Mark the span.
                aEdgeExtents.markWithSort(xs,xe);

                // Update the edge
                currentEdge->mX = x;

                // Add the edge to AET
                if (prevEdge)
                    prevEdge->mNextEdge = currentEdge;
                else
                    aActiveEdgeTable = currentEdge;

                prevEdge = currentEdge;
            }
        
            currentEdge = currentEdge->mNextEdge;
        }
        while (currentEdge);
    }

    if (prevEdge)
        prevEdge->mNextEdge = NULL;
}

inline Pixel *GetPixelsF(BYTE* pBuffer, int nPitch, int x, int y)
{
	return (Pixel*)(pBuffer + (x<<1) + y*nPitch);
}


//! Renders the mask to the canvas with even-odd fill.
void PolygonVersionF::fillEvenOdd(BYTE* pBuffer, int nPitch, Pixel color)
{
    int y;

    int minY = mVerticalExtents.mMinimum;
    int maxY = mVerticalExtents.mMaximum;

	unsigned long aColor = RGB(_GetRValue(color), _GetGValue(color), _GetBValue(color));
    unsigned long cs1 = aColor & 0xff00ff;
    unsigned long cs2 = (aColor >> 8) & 0xff00ff;    

    PolygonScanEdge *activeEdges = NULL;
    SpanExtents edgeExtents;

    for (y = minY; y <= maxY; y++)
    {
        edgeExtents.reset();

        renderEvenOddEdges(activeEdges,edgeExtents,y);

        int minX = edgeExtents.mMinimum;
        // Offset values are not taken into account when calculating the extents, so add
        // one to the maximum. This makes sure that full spans are included, as offset
        // values are in the range of 0 to 1.
        int maxX = edgeExtents.mMaximum + 1;

        if (minX < maxX)
        {
			Pixel *pixel= GetPixelsF(pBuffer, nPitch, minX, y);
            SUBPIXEL_DATA *mb = &mMaskBuffer[minX];
            SUBPIXEL_DATA *end = &mMaskBuffer[maxX + 1];

            // Place a marker data at the end position.
            // Since marker data is placed after the last entry to be rendered (thus mask is 0),
            // it forces a state change. This drops the control to the outer while loop, which
            // terminates because of the compare.
            *end = SUBPIXEL_FULL_COVERAGE;

            SUBPIXEL_DATA mask = *mb;
            *mb++ = 0;

            while (mb <= end)
            {
                // The straightforward implementation reads in the value, modifies the mask with it,
                // calculates the coverage from the mask and renders the result with it.

                // Here the mask value is fetched in the previous round. Therefore the write operation
                // needs to be done before the new mask value is changed. This also means that the
                // end marker will never be rendered, as the loop terminates before that.
                if (mask == 0)
                {
                    // Mask is empty, scan forward until mask changes.
                    SUBPIXEL_DATA *sb = mb;
                    do
                    {
                        mask = *mb++;
                    }
                    while (mask == 0); // && mb <= end);
                    mb[-1] = 0;
					pixel += mb - sb;
                }
                else if (mask == SUBPIXEL_FULL_COVERAGE)
                {
                    // Mask has full coverage, fill with aColor until mask changes.
                    SUBPIXEL_DATA temp;
                    do
                    {
#ifdef GATHER_STATISTICS
                        if (*tp != STATISTICS_BG_COLOR)
                            mStatistics.mOverdrawPixels++;
                        mStatistics.mFilledPixels++;
#endif
						*pixel++ = color;
                        temp = *mb++;
                    }
                    while (temp == 0); // && mb <= end);
                    mb[-1] = 0;
                    mask ^= temp;
                }
                else
                {
                    // Mask is semitransparent.
                    do
                    {
#ifdef GATHER_STATISTICS
                        if (*tp != STATISTICS_BG_COLOR)
                            mStatistics.mOverdrawPixels++;
                        mStatistics.mAntialiasPixels++;
#endif
						aColor = RGB(_GetRValue(*pixel), _GetGValue(*pixel), _GetBValue(*pixel));
                        unsigned long alpha = SUBPIXEL_COVERAGE(mask);

                        // alpha is in range of 0 to SUBPIXEL_COUNT
                        unsigned long invAlpha = SUBPIXEL_COUNT - alpha;

                        unsigned long ct1 = (aColor & 0xff00ff) * invAlpha;
                        unsigned long ct2 = ((aColor >> 8) & 0xff00ff) * invAlpha;

                        ct1 = ((ct1 + cs1 * alpha) >> SUBPIXEL_SHIFT) & 0xff00ff;
                        ct2 = ((ct2 + cs2 * alpha) << (8 - SUBPIXEL_SHIFT)) & 0xff00ff00;
						aColor = ct1 + ct2;
                        *pixel++ = Color( GetRValue(aColor), GetGValue(aColor), GetBValue(aColor));	

                        mask ^= *mb;
                        *mb++ = 0;
                    }
                    while (!(mask == 0 || mask == SUBPIXEL_FULL_COVERAGE)); // && mb <= end);
                }
            }
        }      
    }
}


//! Renders the edges from the current vertical index using non-zero winding fill.
inline void PolygonVersionF::renderNonZeroEdges(PolygonScanEdge * &aActiveEdgeTable, SpanExtents &aEdgeExtents, int aCurrentLine)
{
    static const FIXED_POINT offsets[SUBPIXEL_COUNT] = SUBPIXEL_OFFSETS_FIXED;

    // First, process edges in the active edge table. These are either full height edges
    // or edges that end before the last line. Remove those that end within the scan line.

    // Then, fetch the edges from the edge table. These are either such that the edge is
    // fully within this scanline or the edge starts from within the scanline and continues
    // to the next scanline. Add those that continue to the next scanline to the active
    // edge table. Clear the edge table.

    PolygonScanEdge *prevEdge = NULL;
    PolygonScanEdge *currentEdge = aActiveEdgeTable;

    while (currentEdge)
    {
        // The plotting of all edges in the AET starts from the top of the scanline.
        // The plotting is divided to two stages: those that end on this scanline and
        // those that span over the full scanline.

        int lastLine = currentEdge->mLastLine >> SUBPIXEL_SHIFT;

        if (lastLine == aCurrentLine)
        {
            FIXED_POINT x = currentEdge->mX;
            FIXED_POINT slope = currentEdge->mSlope;
            NON_ZERO_MASK_DATA_UNIT winding = (NON_ZERO_MASK_DATA_UNIT)currentEdge->mWinding;

            // First x value rounded down.
            int xs = FIXED_TO_INT(x);
            int ye = currentEdge->mLastLine & (SUBPIXEL_COUNT - 1);

            SUBPIXEL_DATA mask = 1;
            int ySub;
            for (ySub = 0; ySub <= ye; ySub++)
            {
                int xp = FIXED_TO_INT(x + offsets[ySub]);
                mWindingBuffer[xp].mMask |= mask;
                mWindingBuffer[xp].mBuffer[ySub] += winding;
                mask <<= 1;
                x += slope;
            }

            // Last x-value rounded down.
            int xe = FIXED_TO_INT(x - slope);

            // Mark the span.
            aEdgeExtents.markWithSort(xs,xe);

            // Remove the edge from the active edge table
            currentEdge = currentEdge->mNextEdge;
            if (prevEdge)
                prevEdge->mNextEdge = currentEdge;
            else
                aActiveEdgeTable = currentEdge;
        }
        else
        {
            FIXED_POINT x = currentEdge->mX;
            FIXED_POINT slope = currentEdge->mSlope;
            NON_ZERO_MASK_DATA_UNIT winding = (NON_ZERO_MASK_DATA_UNIT)currentEdge->mWinding;

            // First x value rounded down.
            int xs = FIXED_TO_INT(x);
            int xe;

            NONZERO_LINE_UNROLL_INIT();

            NONZERO_LINE_UNROLL_0(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_1(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_2(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_3(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_4(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_5(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_6(mWindingBuffer,x,slope,offsets,winding);
#if SUBPIXEL_COUNT == 8
            xe = FIXED_TO_INT(x);
#endif
            NONZERO_LINE_UNROLL_7(mWindingBuffer,x,slope,offsets,winding);

#if SUBPIXEL_COUNT > 8
            NONZERO_LINE_UNROLL_8(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_9(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_10(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_11(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_12(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_13(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_14(mWindingBuffer,x,slope,offsets,winding);
#if SUBPIXEL_COUNT == 16
            xe = FIXED_TO_INT(x);
#endif
            NONZERO_LINE_UNROLL_15(mWindingBuffer,x,slope,offsets,winding);
#endif

#if SUBPIXEL_COUNT > 16
            NONZERO_LINE_UNROLL_16(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_17(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_18(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_19(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_20(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_21(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_22(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_23(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_24(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_25(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_26(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_27(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_28(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_29(mWindingBuffer,x,slope,offsets,winding);
            NONZERO_LINE_UNROLL_30(mWindingBuffer,x,slope,offsets,winding);
            xe = FIXED_TO_INT(x);
            NONZERO_LINE_UNROLL_31(mWindingBuffer,x,slope,offsets,winding);
#endif
            // Mark the span.
            aEdgeExtents.markWithSort(xs,xe);

            // Update the edge
            if ((aCurrentLine & SLOPE_FIX_SCANLINE_MASK) == 0)
                currentEdge->mX = x + currentEdge->mSlopeFix;
            else
                currentEdge->mX = x;

            // Proceed forward in the AET.
            prevEdge = currentEdge;
            currentEdge = currentEdge->mNextEdge;
        }
    }

    // Fetch edges from the edge table.
    currentEdge = mEdgeTable[aCurrentLine];

    if (currentEdge)
    {
        // Clear the edge table for this line.
        mEdgeTable[aCurrentLine] = NULL;

        do
        {
            // The plotting of all edges in the edge table starts somewhere from the middle
            // of the scanline, and ends either at the end or at the last scanline. This leads
            // to two cases: either the scanning starts and stops within this scanline, or
            // it continues to the next as well.

            int lastLine = currentEdge->mLastLine >> SUBPIXEL_SHIFT;

            if (lastLine == aCurrentLine)
            {
                FIXED_POINT x = currentEdge->mX;
                FIXED_POINT slope = currentEdge->mSlope;
                NON_ZERO_MASK_DATA_UNIT winding = (NON_ZERO_MASK_DATA_UNIT)currentEdge->mWinding;

                // First x value rounded down.
                int xs = FIXED_TO_INT(x);

                int ys = currentEdge->mFirstLine & (SUBPIXEL_COUNT - 1);
                int ye = currentEdge->mLastLine & (SUBPIXEL_COUNT - 1);

                SUBPIXEL_DATA mask = 1 << ys;
                int ySub;
                for (ySub = ys; ySub <= ye; ySub++)
                {
                    int xp = FIXED_TO_INT(x + offsets[ySub]);
                    mWindingBuffer[xp].mMask |= mask;
                    mWindingBuffer[xp].mBuffer[ySub] += winding;
                    mask <<= 1;
                    x += slope;
                }

                // Last x-value rounded down.
                int xe = FIXED_TO_INT(x - slope);

                // Mark the span.
                aEdgeExtents.markWithSort(xs,xe);

                // Ignore the edge (don't add to AET)
            }
            else
            {
                FIXED_POINT x = currentEdge->mX;
                FIXED_POINT slope = currentEdge->mSlope;
                NON_ZERO_MASK_DATA_UNIT winding = (NON_ZERO_MASK_DATA_UNIT)currentEdge->mWinding;

                // First x value rounded down.
                int xs = FIXED_TO_INT(x);

                int ys = currentEdge->mFirstLine & (SUBPIXEL_COUNT - 1);

                SUBPIXEL_DATA mask = 1 << ys;
                int ySub;
                for (ySub = ys; ySub < SUBPIXEL_COUNT; ySub++)
                {
                    int xp = FIXED_TO_INT(x + offsets[ySub]);
                    mWindingBuffer[xp].mMask |= mask;
                    mWindingBuffer[xp].mBuffer[ySub] += winding;
                    mask <<= 1;
                    x += slope;
                }

                // Last x-value rounded down.
                int xe = FIXED_TO_INT(x - slope);

                // Mark the span.
                aEdgeExtents.markWithSort(xs,xe);

                // Update the edge
                currentEdge->mX = x;

                // Add the edge to AET
                if (prevEdge)
                    prevEdge->mNextEdge = currentEdge;
                else
                    aActiveEdgeTable = currentEdge;

                prevEdge = currentEdge;
            }
        
            currentEdge = currentEdge->mNextEdge;
        }
        while (currentEdge);

    }

    if (prevEdge)
        prevEdge->mNextEdge = NULL;
}


//! Renders the mask to the canvas with non-zero winding fill.
void PolygonVersionF::fillNonZero(BYTE* pBuffer, int nPitch, Pixel color, const ClipRectangle &aClipRect)
{
    static const FIXED_POINT offsets[SUBPIXEL_COUNT] = SUBPIXEL_OFFSETS_FIXED;
    int y;

    int minY = mVerticalExtents.mMinimum;
    int maxY = mVerticalExtents.mMaximum;
	unsigned long aColor = RGB(_GetRValue(color), _GetGValue(color), _GetBValue(color));	
    unsigned long cs1 = aColor & 0xff00ff;
    unsigned long cs2 = (aColor >> 8) & 0xff00ff;    

    NonZeroMask values;

    PolygonScanEdge *activeEdges = NULL;
    SpanExtents edgeExtents;

    for (y = minY; y <= maxY; y++)
    {
        edgeExtents.reset();

        renderNonZeroEdges(activeEdges,edgeExtents,y);

        int minX = edgeExtents.mMinimum;
        // Offset values are not taken into account when calculating the extents, so add
        // one to the maximum. This makes sure that full spans are included, as offset
        // values are in the range of 0 to 1.
        int maxX = edgeExtents.mMaximum + 1;

        if (minX < maxX)
        {
			Pixel *pixel= GetPixelsF(pBuffer, nPitch, minX, y);
            NonZeroMask *wb = &mWindingBuffer[minX];
            NonZeroMask *end = &mWindingBuffer[maxX + 1];

            // Place a marker data at the end position.
            // Since marker data is placed after the last entry to be rendered (thus mask is 0),
            // it forces a state change. This drops the control to the outer while loop, which
            // terminates because of the compare.
            MEMSET(end,0xff,sizeof(NonZeroMask));

            NonZeroMask *temp = wb++;
            NonZeroMaskC::reset(*temp,values);

            while (wb <= end)
            {
                // The straightforward implementation reads in the value, modifies the mask with it,
                // calculates the coverage from the mask and renders the result with it.

                // Here the mask value is fetched in the previous round. Therefore the write operation
                // needs to be done before the new mask value is changed. This also means that the
                // end marker will never be rendered, as the loop terminates before that.
                if (values.mMask == 0)
                {
                    do
                    {
                        // Mask is empty, scan forward until mask changes.
                        NonZeroMask *sb = wb;
                        do
                        {
                            temp = wb++;
                        }
                        while (temp->mMask == 0 && wb <= end);

                        int count = wb - sb;
                        pixel += count;

                        NonZeroMaskC::init(*temp,values);
                    }
                    while (values.mMask == 0); // && wb <= end);
                }
                else if (values.mMask == SUBPIXEL_FULL_COVERAGE)
                {
                    do
                    {
#ifdef GATHER_STATISTICS
                        if (*tp != STATISTICS_BG_COLOR)
                            mStatistics.mOverdrawPixels++;
                        mStatistics.mFilledPixels++;
#endif

                        // Mask has full coverage, fill with aColor until mask changes.
						*pixel++ = color;
                        temp = wb++;

                        if (temp->mMask)
                        {
                            NonZeroMaskC::apply(*temp,values);

                            // A safeguard is needed if the winding value overflows and end marker is not
                            // detected correctly
                            if (wb > end)
                                break;
                        }
                    }
                    while (values.mMask == SUBPIXEL_FULL_COVERAGE); // && wb <= end);
                }
                else
                {
                    // Mask is semitransparent.
                    unsigned long alpha = SUBPIXEL_COVERAGE(values.mMask);

                    // alpha is in range of 0 to SUBPIXEL_COUNT
                    unsigned long invAlpha = SUBPIXEL_COUNT - alpha;
                    unsigned long cs1a = cs1 * alpha;
                    unsigned long cs2a = cs2 * alpha;

                    do
                    {
#ifdef GATHER_STATISTICS
                        if (*tp != STATISTICS_BG_COLOR)
                            mStatistics.mOverdrawPixels++;
                        mStatistics.mAntialiasPixels++;
#endif

						aColor = RGB(_GetRValue(*pixel), _GetGValue(*pixel), _GetBValue(*pixel));
                        unsigned long ct1 = (aColor & 0xff00ff) * invAlpha;
                        unsigned long ct2 = ((aColor >> 8) & 0xff00ff) * invAlpha;

                        ct1 = ((ct1 + cs1a) >> SUBPIXEL_SHIFT) & 0xff00ff;
                        ct2 = ((ct2 + cs2a) << (8 - SUBPIXEL_SHIFT)) & 0xff00ff00;
						
						aColor = ct1 + ct2;
                        *pixel++ = Color( GetRValue(aColor), GetGValue(aColor), GetBValue(aColor));

                        temp = wb++;
                        if (temp->mMask)
                        {							
                            NonZeroMaskC::apply(*temp,values);

                            alpha = SUBPIXEL_COVERAGE(values.mMask);
                            invAlpha = SUBPIXEL_COUNT - alpha;
                            cs1a = cs1 * alpha;
                            cs2a = cs2 * alpha;

                            // A safeguard is needed if the winding value overflows and end marker is not
                            // detected correctly
                            if (wb > end)
                                break;
                        }
                    }
                    while (values.mMask != 0 && values.mMask != SUBPIXEL_FULL_COVERAGE); // && wb <= end);
                }
            }
        }        
    }

}


//! Resizes the edge storage.
bool PolygonVersionF::resizeEdgeStorage(int aIncrement)
{
    unsigned int newCount = mEdgeCount + aIncrement;

    PolygonScanEdge *newStorage = new PolygonScanEdge[newCount];
    if (newStorage == NULL)
        return false;

    // Some pointer arithmetic to keep the linked lists in sync.
    // Make this 64-bit just for future compatibilty
    INTEGER64 diff = (INTEGER64)newStorage - (INTEGER64)mEdgeStorage;

    unsigned int n;
    for (n = 0; n < mHeight; n++)
    {
        if (mEdgeTable[n] != NULL)
            mEdgeTable[n] = (PolygonScanEdge *)((INTEGER64)mEdgeTable[n] + diff);
    }

    // Copy edge storage...
    MEMCPY(newStorage, mEdgeStorage,sizeof(PolygonScanEdge)*mEdgeCount);

    // ...and fix the pointers.
    for (n = 0; n < mEdgeCount; n++)
    {
        PolygonScanEdge *edge = &newStorage[n];
        if (edge->mNextEdge != NULL)
            edge->mNextEdge = (PolygonScanEdge *)((INTEGER64)edge->mNextEdge + diff);
    }

    delete[] mEdgeStorage;
    mEdgeStorage = newStorage;
    mEdgeCount = newCount;

    return true;
}
