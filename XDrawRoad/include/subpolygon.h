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
 * \brief A class for handling sub-polygons. Each sub-polygon is a continuos, closed set of edges.
 *
 *
 * $Id: $
 * $Date: $
 * $Revision: $
 */

#ifndef SUB_POLYGON_H_INCLUDED
#define SUB_POLYGON_H_INCLUDED

#define NUM_OF_POLYGON_VERTEX 500
#define NUM_OF_CAP_POINT 36
#define NUM_OF_JOIN_POINT 5
#define NUM_OF_EDGE 2000
#define SUBPOLYGON_EPSILON 2 * SUBPIXEL_COUNT * FLT_EPSILON

//#define USE_ASM_OPTIMIZATIONS
//#ifdef UNDER_CE
//#undef USE_ASM_OPTIMIZATIONS
//#endif // UNDER_CE

//! Converts rational to int using direct asm.
/*! Typical conversions using _ftol are slow on Pentium 4, as _ftol sets the control
 *  state of the fpu unit. Also, we need a special rounding operation that rounds down,
 *  not towards zero. Therefore we use asm here. This requires that the rounding mode
 *  is set to round down by calling _controlfp(_MCW_RC,_RC_DOWN). The state of the
 *  control mode is thread specific, so other applications won't mix with this.
 */
inline int rationalToIntRoundDown(RATIONAL a)
{
#ifdef USE_ASM_OPTIMIZATIONS
    int i;
    _asm
    {
        fld a
        fistp i
    }
    return i;
#else
    return ((int)((a) + INT_TO_RATIONAL(1)) - 1);
#endif
}


//! The rational to fixed conversion is implemented with asm for performance reasons.
inline int rationalToFixed(RATIONAL a)
{
#ifdef USE_ASM_OPTIMIZATIONS
    int i;
    static const double scale = (double)(1 << FIXED_POINT_SHIFT);
    _asm
    {
        fld a
        fmul scale
        fistp i
    }
    return i;
#else
    return FLOAT_TO_FIXED(a);
#endif
}


//! Sets the FPU round down mode to correct value for the asm conversion routines.
inline void setRoundDownMode()
{
#ifdef USE_ASM_OPTIMIZATIONS
    _controlfp(_MCW_RC,_RC_DOWN);//if used -> route problem
#endif
}


// #define SUBPOLYGON_DEBUG

enum POLYGON_CLIP_FLAGS
{
    POLYGON_CLIP_NONE = 0x00,
    POLYGON_CLIP_LEFT = 0x01,
    POLYGON_CLIP_RIGHT = 0x02,
    POLYGON_CLIP_TOP = 0x04,
    POLYGON_CLIP_BOTTOM = 0x08,
};


#define CLIP_SUM_SHIFT 8
#define CLIP_UNION_SHIFT 4

/*! Fixed point math inevitably introduces rounding error to the DDA. The error is
 *  fixed every now and then by a separate fix value. The defines below set these.
 */
#define SLOPE_FIX_SHIFT 8
#define SLOPE_FIX_STEP (1 << SLOPE_FIX_SHIFT)
#define SLOPE_FIX_MASK (SLOPE_FIX_STEP - 1)
#define SLOPE_FIX_SCANLINES (1 << (SLOPE_FIX_SHIFT - SUBPIXEL_SHIFT))
#define SLOPE_FIX_SCANLINE_MASK (SLOPE_FIX_SCANLINES - 1)

//! A polygon edge.
class PolygonEdge
{
public:
    int mFirstLine;
    int mLastLine;
    short mWinding;
    RATIONAL mX;
    RATIONAL mSlope;
};

//! A more optimized representation of a polygon edge.
class PolygonScanEdge
{
public:
    int mLastLine;
    int mFirstLine;
    short mWinding;
    FIXED_POINT mX;
    FIXED_POINT mSlope;
    FIXED_POINT mSlopeFix;
    class PolygonScanEdge *mNextEdge;
};

//! A class for holding processing data regarding the vertex.
class VertexData
{
public:
    VECTOR2D mPosition;
    int mClipFlags;
    int mLine;
};

//! A class for handling sub-polygons. Each sub-polygon is a continuos, closed set of edges.
class SubPolygon
{
public:
    //! Constructor.
    /*! Note that the ownership of the vertex array is transferred.
     */
    SubPolygon(VECTOR2D *aVertices, int aVertexCount);
	SubPolygon();	

    //! Initializer.
    //bool init();
	bool init_poly(VECTOR2D * aVertices, int aVertexCount);

    //! Destructor.
    ~SubPolygon();

    //! Returns the amount of vertices in the polygon.
    inline int getVertexCount() const
    {
        return mVertexCount;
    }

    //! Returns a vertex at given position.
    inline const VECTOR2D & getVertex(int aIndex) const
    {
        return mVertices[aIndex];
    }

    //! Calculates the edges of the polygon with transformation and clipping to aEdges array.
    /*! Note that this may return upto three times the amount of edges that the polygon has vertices,
     *  in the unlucky case where both left and right side get clipped for all edges.
     *  \param aEdges the array for result edges. This should be able to contain 2*aVertexCount edges.
     *  \param aTransformation the transformation matrix for the polygon.
     *  \param aClipRectangle the clip rectangle.
     *  \return the amount of edges in the result.
     */
    int getScanEdges(PolygonScanEdge *aEdges, const ClipRectangle &aClipRectangle) const;

protected:
    //! Calculates the clip flags for a point.
    inline int getClipFlags(const VECTOR2D &aPoint, const ClipRectangle &aClipRectangle) const;

    //! Creates a polygon edge between two vectors.
    /*! Clips the edge vertically to the clip rectangle. Returns true for edges that
     *  should be rendered, false for others.
     */
    bool getEdge(const VECTOR2D &aStart, const VECTOR2D &aEnd, PolygonEdge &aEdge, const ClipRectangle &aClipRectangle) const;

    //! Creates a vertical polygon edge between two y values.
    /*! Clips the edge vertically to the clip rectangle. Returns true for edges that
     *  should be rendered, false for others.
     */
    bool getVerticalEdge(RATIONAL aStartY, RATIONAL aEndY, RATIONAL aX, PolygonEdge &aEdge, const ClipRectangle &aClipRectangle) const;

    //! Returns the edge(s) between two vertices in aVertexData.
    inline int getScanEdge(VertexData *aVertexData, short aWinding, PolygonScanEdge *aEdges, const ClipRectangle &aClipRectangle) const;

#ifdef SUBPOLYGON_DEBUG
    //! A debug routine for checking that generated edge is valid.
    bool verifyEdge(const PolygonEdge &aEdge, const ClipRectangle &aClipRectangle) const;

    //! A debug routine for checking that generated edge is valid.
    bool verifyEdge(const PolygonScanEdge &aEdge, const ClipRectangle &aClipRectangle) const;
#endif

    int mVertexCount;
    VECTOR2D *mVertices;
    VertexData *mVertexData;
	int m_nVertextDataAlloc;
};

#endif // !SUB_POLYGON_H_INCLUDED
