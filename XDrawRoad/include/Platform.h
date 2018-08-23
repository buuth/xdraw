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
 * \brief A header file for defining platform related class formats and functions.
 *
 *
 * $Id: $
 * $Date: $
 * $Revision: $
 */


#ifndef PLATFORM_H_INCLUDED
#define PLATFORM_H_INCLUDED

// Required includes for the platform.
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <assert.h>

// General defines
#ifndef NULL
#define NULL 0
#endif
#define MEMSET(aAddress,aData,aLength) memset(aAddress,aData,aLength)
#define MEMCPY(aTarget,aSource,aLength) memcpy(aTarget,aSource,aLength)
#define STRDUP(aString) strdup(aString)
#define STRCAT(aTgt,aSrc) strcat(aTgt,aSrc)
#define STRLEN(aString) strlen(aString)
#define STREQ(aString1,aString2) (strcmp(aString1,aString2) == 0)
#define SSCANF(aString,aFormat,aParameter) sscanf(aString,aFormat,aParameter)

// Defines for the rational number format.
#define RATIONAL float
#define INT_TO_RATIONAL(a) ((float)(a))
#define FLOAT_TO_RATIONAL(a) (a)
#define RATIONAL_SIN(a) (float)sin((double)(a))
#define RATIONAL_COS(a) (float)cos((double)(a))
#define MIN_RATIONAL ((float)-1000000000000000)
#define MAX_RATIONAL ((float)1000000000000000)

/*
#define RATIONAL double
#define INT_TO_RATIONAL(a) ((double)(a))
#define FLOAT_TO_RATIONAL(a) ((double)(a))
#define RATIONAL_SIN(a) sin((a))
#define RATIONAL_COS(a) cos((a))
#define MIN_RATIONAL ((double)-1000000000000000)
#define MAX_RATIONAL ((double)1000000000000000)
*/

// Defines the fixed point conversions
#define FIXED_POINT int
#define FIXED_POINT_SHIFT 16
#define INT_TO_FIXED(a) ((a) << FIXED_POINT_SHIFT)
#define FLOAT_TO_FIXED(a) (int)((a)*((float)(1 << FIXED_POINT_SHIFT)))

#define FIXED_TO_INT(a) ((a) >> FIXED_POINT_SHIFT) 
#define FIXED_TO_FLOAT(a) ((float)(a)/((float)(1 << FIXED_POINT_SHIFT)))
#define RATIONAL_TO_FIXED(a) FLOAT_TO_FIXED(a)
#define FIXED_TO_RATIONAL(a) FIXED_TO_FLOAT(a)

// Define for 64-bit integer number format
#define INTEGER64 __int64

// Defines for the vector and matrix classes
#include "Vector2d.h"

#define VECTOR2D Vector2d
#define VECTOR2D_SETX(aVector,aX) aVector.mX = (aX)
#define VECTOR2D_SETY(aVector,aY) aVector.mY = (aY)
#define VECTOR2D_GETX(aVector) aVector.mX
#define VECTOR2D_GETY(aVector) aVector.mY

#define MATRIX2D Matrix2d
#define MATRIX2D_MAKEROTATION(aMatrix,aAngle) aMatrix.makeRotation(aAngle)
#define MATRIX2D_MAKETRANSLATION(aMatrix,aTranslation) aMatrix.makeTranslation(aTranslation)
#define MATRIX2D_MAKESCALING(aMatrix,aScale) aMatrix.makeScaling(aScale)
#define MATRIX2D_MAKEIDENTITY(aMatrix) aMatrix.makeIdentity()
#define MATRIX2D_ROTATE(aMatrix,aAngle) aMatrix.rotate(aAngle)
#define MATRIX2D_TRANSLATE(aMatrix,aTranslation) aMatrix.translate(aTranslation)
#define MATRIX2D_SCALE(aMatrix,aScale) aMatrix.scale(aScale)
#define MATRIX2D_MULTIPLY(aMatrix,aMultiplier) aMatrix.multiply(aMultiplier)
#define MATRIX2D_TRANSFORM(aMatrix,aSource,aResult) aMatrix.transform(aSource,aResult)
#define MATRIX2D_GET_M11(aMatrix) aMatrix.mMatrix[0][0]
#define MATRIX2D_GET_M12(aMatrix) aMatrix.mMatrix[0][1]
#define MATRIX2D_GET_M21(aMatrix) aMatrix.mMatrix[1][0]
#define MATRIX2D_GET_M22(aMatrix) aMatrix.mMatrix[1][1]
#define MATRIX2D_GET_DX(aMatrix) aMatrix.mMatrix[2][0]
#define MATRIX2D_GET_DY(aMatrix) aMatrix.mMatrix[2][1]

// Defines the file IO functions for writing text
#define TEXTFILE_OBJECT FILE
#define TEXTFILE_OPEN_FOR_WRITE(aName) fopen(aName,"wb")
#define TEXTFILE_WRITE_STRING(aFile,aString) fprintf(aFile,"%s",aString)
#define TEXTFILE_WRITE_RATIONAL(aFile,aRational) fprintf(aFile,"%f",(float)aRational)
#define TEXTFILE_WRITE_INT(aFile,aInt) fprintf(aFile,"%d",aInt)
#define TEXTFILE_CLOSE(aObject) fclose(aObject)

// Defines the file IO functions for writing binary
#define BINARYFILE_OBJECT FILE
#define BINARYFILE_OPEN_FOR_WRITE(aName) fopen(aName,"wb")
#define BINARYFILE_WRITE_DATA(aDataPtr,aItemSize,aItemCount,aFile) fwrite(aDataPtr,aItemSize,aItemCount,aFile)
#define BINARYFILE_CLOSE(aObject) fclose(aObject)

// Defines the debug assertion
#define DEBUG_ASSERT(aExp) assert(aExp)

#endif // !PLATFORM_H_INCLUDED
