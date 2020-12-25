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
#ifndef _skPixel_h_
#define _skPixel_h_

#include "Utils/Config/skConfig.h"
struct FIBITMAP;

enum class skPixelFormat
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
};

enum skImageFormat
{
    IMF_TGA,
    IMF_BMP,
    IMF_JPEG,
    IMF_PNG,
    IMF_XPM,
};



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


struct skPixelLA
{
#if SK_ENDIAN == SK_ENDIAN_BIG
    SKubyte l, a;
#else
    SKubyte a, l;
#endif
};

struct skPixelRGB
{
#if SK_ENDIAN == SK_ENDIAN_BIG
    SKubyte r, g, b;
#else
    SKubyte b, g, r;
#endif
};

struct skPixelRGBA
{
#if SK_ENDIAN == SK_ENDIAN_BIG
    SKubyte r, g, b, a;
#else
    SKubyte b, g, r, a;
#endif
};

class skPixel
{
public:
    skPixel() :
        r(0),
        g(0),
        b(0),
        a(0)
    {
    }

    skPixel(const skPixel& rhs) :
        r(rhs.r),
        g(rhs.g),
        b(rhs.b),
        a(rhs.a)
    {
    }


    skPixel(const SKubyte r, const SKubyte g, const SKubyte b, const SKubyte a) :
        r(r),
        g(g),
        b(b),
        a(a)
    {
    }


    skPixel(const SKuint32& col);


    void add(const skPixel& px);
    void sub(const skPixel& px);
    void mul(const skPixel& px);
    void div(const skPixel& px);
    void mix(const skPixel& px, double f);

public:
    SKubyte r, g, b, a;
};



#endif  //_skPixel_h_
