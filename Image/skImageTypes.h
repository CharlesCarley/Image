/*
-------------------------------------------------------------------------------
    Copyright (c) Charles Carley.

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------
*/
#ifndef _skImageTypes_h_
#define _skImageTypes_h_

#include "Utils/Config/skConfig.h"
SK_ST_C;


typedef enum SKPixelFormat
{
    SK_ALPHA,
    SK_LUMINANCE,
    SK_LUMINANCE_ALPHA,
    SK_BGR,
    SK_RGB,
    SK_RGBA,
    SK_BGRA,
    SK_ARGB,
    SK_ABGR,
    SK_PF_MAX,
} skPixelFormat;


typedef union skColorUnion
{
    SKubyte  b[4];
    SKuint32 i;
} skColorUnion;

#if SK_ENDIAN == SK_ENDIAN_BIG
#define SK_rIdx 0
#define SK_gIdx 1
#define SK_bIdx 2
#define SK_aIdx 3
#else
#define SK_rIdx 3
#define SK_gIdx 2
#define SK_bIdx 1
#define SK_aIdx 0
#endif


typedef struct skPixelLA
{
#if SK_ENDIAN == SK_ENDIAN_BIG
    SKubyte l, a;
#else
    SKubyte a, l;
#endif
} skPixelLA;

typedef struct skPixelRGB
{
#if SK_ENDIAN == SK_ENDIAN_BIG
    SKubyte r, g, b;
#else
    SKubyte b, g, r;
#endif
} skPixelRGB;

typedef struct skPixelRGBA
{
#if SK_ENDIAN == SK_ENDIAN_BIG
    SKubyte r, g, b, a;
#else
    SKubyte b, g, r, a;
#endif
} skPixelRGBA;

SK_EN_C;

#endif  //_skImageTypes_h_
