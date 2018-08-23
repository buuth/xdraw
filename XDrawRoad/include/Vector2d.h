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
 * \brief A simple 2d vector class.
 *
 *
 * $Id: $
 * $Date: $
 * $Revision: $
 */


#ifndef VECTOR2D_H_INCLUDED
#define VECTOR2D_H_INCLUDED

//! A simple 2d vector class.
class Vector2d
{
public:
    //! Constructor.
    inline Vector2d() {}

    //! Constructor.
    inline Vector2d(RATIONAL aX, RATIONAL aY)
    {
        mX = aX;
        mY = aY;
    }

    RATIONAL mX;
    RATIONAL mY;
};

#endif // !VECTOR2D_H_INCLUDED
