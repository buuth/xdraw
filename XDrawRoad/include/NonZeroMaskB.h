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
 * \brief NonZeroMaskB is an implementation for tracking the mask data of non-zero winding buffer. It
 *        doesn't use compares for building the masks.
 *
 *
 * $Id: $
 * $Date: $
 * $Revision: $
 */


#ifndef NON_ZERO_MASK_B_H_INCLUDED
#define NON_ZERO_MASK_B_H_INCLUDED

#ifdef NON_ZERO_MASK_USE_SHORT_DATA

#define NON_ZERO_MASK_B_FILL 0xffff
#define NON_ZERO_MASK_B_MASK 0x10000

#if SUBPIXEL_COUNT == 8
#define NON_ZERO_MASK_B_FIRST_RANGE 8
#endif
#if SUBPIXEL_COUNT == 16
#define NON_ZERO_MASK_B_FIRST_RANGE 16
#endif
#if SUBPIXEL_COUNT == 32
#define NON_ZERO_MASK_B_FIRST_RANGE 16
#define NON_ZERO_MASK_B_SECOND_RANGE 32
#endif

#else

#define NON_ZERO_MASK_B_FILL 0xff
#define NON_ZERO_MASK_B_MASK 0x100

#if SUBPIXEL_COUNT == 8
#define NON_ZERO_MASK_B_FIRST_RANGE 8
#endif
#if SUBPIXEL_COUNT == 16
#define NON_ZERO_MASK_B_FIRST_RANGE 8
#define NON_ZERO_MASK_B_SECOND_RANGE 16
#endif
#if SUBPIXEL_COUNT == 32
#define NON_ZERO_MASK_B_FIRST_RANGE 8
#define NON_ZERO_MASK_B_SECOND_RANGE 32
#endif

#endif

//! An implementation for tracking the mask data of non-zero winding buffer.
/*! It doesn't use compares for building the masks.
 */
class NonZeroMaskB
{
public:
    static inline void reset(NonZeroMask &aSource, NonZeroMask &aDestination)
    {
        int n;
        aDestination.mMask = 0;
        for (n = 0; n < NON_ZERO_MASK_B_FIRST_RANGE; n++)
        {
            aDestination.mBuffer[n] = aSource.mBuffer[n];
            aSource.mBuffer[n] = 0;
            unsigned long t = (unsigned long)((unsigned NON_ZERO_MASK_DATA_UNIT)aDestination.mBuffer[n]) + NON_ZERO_MASK_B_FILL;
            aDestination.mMask |= (SUBPIXEL_DATA)((t & NON_ZERO_MASK_B_MASK) >> (NON_ZERO_MASK_DATA_BITS - n));
        }

#if NON_ZERO_MASK_B_SECOND_RANGE
        for (n = NON_ZERO_MASK_B_FIRST_RANGE; n < NON_ZERO_MASK_B_SECOND_RANGE; n++)
        {
            aDestination.mBuffer[n] = aSource.mBuffer[n];
            aSource.mBuffer[n] = 0;
            unsigned long t = (unsigned long)((unsigned NON_ZERO_MASK_DATA_UNIT)aDestination.mBuffer[n]) + NON_ZERO_MASK_B_FILL;
            aDestination.mMask |= (SUBPIXEL_DATA)((t & NON_ZERO_MASK_B_MASK) << (n - NON_ZERO_MASK_DATA_BITS));
        }
#endif
        aSource.mMask = 0;
    }

    static inline void init(NonZeroMask &aSource, NonZeroMask &aDestination)
    {
        int n;
        for (n = 0; n < NON_ZERO_MASK_B_FIRST_RANGE; n++)
        {
            aDestination.mBuffer[n] = aSource.mBuffer[n];
            aSource.mBuffer[n] = 0;
            unsigned long t = (unsigned long)((unsigned NON_ZERO_MASK_DATA_UNIT)aDestination.mBuffer[n]) + NON_ZERO_MASK_B_FILL;
            aDestination.mMask |= (SUBPIXEL_DATA)((t & NON_ZERO_MASK_B_MASK) >> (NON_ZERO_MASK_DATA_BITS - n));
        }

#if NON_ZERO_MASK_B_SECOND_RANGE
        for (n = NON_ZERO_MASK_B_FIRST_RANGE; n < NON_ZERO_MASK_B_SECOND_RANGE; n++)
        {
            aDestination.mBuffer[n] = aSource.mBuffer[n];
            aSource.mBuffer[n] = 0;
            unsigned long t = (unsigned long)((unsigned NON_ZERO_MASK_DATA_UNIT)aDestination.mBuffer[n]) + NON_ZERO_MASK_B_FILL;
            aDestination.mMask |= (SUBPIXEL_DATA)((t & NON_ZERO_MASK_B_MASK) << (n - NON_ZERO_MASK_DATA_BITS));
        }
#endif
        aSource.mMask = 0;
    }

    static inline void apply(NonZeroMask &aSource, NonZeroMask &aDestination)
    {
        int n;
        for (n = 0; n < NON_ZERO_MASK_B_FIRST_RANGE; n++)
        {
            unsigned long t = (unsigned long)((unsigned NON_ZERO_MASK_DATA_UNIT)aDestination.mBuffer[n]) + NON_ZERO_MASK_B_FILL;
            aDestination.mBuffer[n] += aSource.mBuffer[n];
            aSource.mBuffer[n] = 0;
            t ^= (unsigned long)((unsigned NON_ZERO_MASK_DATA_UNIT)aDestination.mBuffer[n]) + NON_ZERO_MASK_B_FILL;
            aDestination.mMask ^= (SUBPIXEL_DATA)((t & NON_ZERO_MASK_B_MASK) >> (NON_ZERO_MASK_DATA_BITS - n));
        }

#if NON_ZERO_MASK_B_SECOND_RANGE
        for (n = NON_ZERO_MASK_B_FIRST_RANGE; n < NON_ZERO_MASK_B_SECOND_RANGE; n++)
        {
            unsigned long t = (unsigned long)((unsigned NON_ZERO_MASK_DATA_UNIT)aDestination.mBuffer[n]) + NON_ZERO_MASK_B_FILL;
            aDestination.mBuffer[n] += aSource.mBuffer[n];
            aSource.mBuffer[n] = 0;
            t ^= (unsigned long)((unsigned NON_ZERO_MASK_DATA_UNIT)aDestination.mBuffer[n]) + NON_ZERO_MASK_B_FILL;
            aDestination.mMask ^= (SUBPIXEL_DATA)((t & NON_ZERO_MASK_B_MASK) << (n - NON_ZERO_MASK_DATA_BITS));
        }
#endif
        aSource.mMask = 0;
    }

};

#endif // !NON_ZERO_MASK_B_H_INCLUDED
