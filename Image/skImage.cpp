/*
-------------------------------------------------------------------------------
    Copyright (c) 2019 Charles Carley.

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
#include "skImage.h"
#include <cmath>
#include "FreeImage.h"
#include "Utils/skLogger.h"
#include "Utils/skMemoryUtils.h"
#include "Utils/skMinMax.h"
#include "Utils/skPlatformHeaders.h"

#define UINTV32(x) ((x) > 0 && (x) < SK_NPOS32)
#define GETBUFFERPOS(x, y) (SKuint32)((SKsize)m_height - (y)) * m_pitch + (x)*m_bpp


class ImageUtils
{
public:
    static FREE_IMAGE_FORMAT getFormat(int format)
    {
        int out = (int)FREE_IMAGE_FORMAT::FIF_UNKNOWN;
        switch (format)
        {
        case skImageFormat::IMF_BMP:
            out = FREE_IMAGE_FORMAT::FIF_BMP;
            break;
        case skImageFormat::IMF_JPEG:
            out = FREE_IMAGE_FORMAT::FIF_JPEG;
            break;
        case skImageFormat::IMF_PNG:
            out = FREE_IMAGE_FORMAT::FIF_PNG;
            break;
        case skImageFormat::IMF_TGA:
            out = FREE_IMAGE_FORMAT::FIF_TARGA;
            break;
        case skImageFormat::IMF_XPM:
            out = FREE_IMAGE_FORMAT::FIF_XPM;
            break;
        default:
            break;
        }
        return (FREE_IMAGE_FORMAT)out;
    }


    static void clearA(SKubyte* mem, SKsize max, const skPixel& p)
    {
        skMemset(mem, p.a, max);
    }

    static void clearLA(SKubyte* mem, SKsize max, const skPixel& p)
    {
        if (mem && max > 2)
        {
            union cpack
            {
                SKubyte  cp[2];
                SKuint16 i;
            } pak{};

            pak.cp[0] = (SKubyte)(((int)p.r + (int)p.g + (int)p.b) / 3);
            pak.cp[1] = p.a;

            auto*        ptr = (SKuint16*)&mem[0];
            const SKsize sz  = max / 2;
            SKsize       i;
            for (i = 0; i < sz; ++i)
                *ptr++ = pak.i;
        }
    }

    static void clearRGB(SKubyte* mem, SKsize max, const skPixel& p)
    {
        if (mem && max > 3)
        {
            SKsize i;
            for (i = 0; i < max - 3; i += 3)
            {
#if SK_ENDIAN == SK_ENDIAN_BIG
                mem[i]     = p.r;
                mem[i + 1] = p.g;
                mem[i + 2] = p.b;
#else
                mem[i]     = p.b;
                mem[i + 1] = p.g;
                mem[i + 2] = p.r;
#endif
            }
        }
    }

    static void clearRGBA(SKubyte* mem, SKsize max, const skPixel& p)
    {
        if (mem && max > 4)
        {
            SKsize i;
            for (i = 0; i < max - 4; i += 4)
            {
#if SK_ENDIAN == SK_ENDIAN_BIG
                mem[i]     = p.r;
                mem[i + 1] = p.g;
                mem[i + 2] = p.b;
                mem[i + 3] = p.a;
#else
                mem[i]     = p.a;
                mem[i + 1] = p.r;
                mem[i + 2] = p.g;
                mem[i + 3] = p.b;
#endif
            }
        }
    }
};



void skImage::initialize()
{
    FreeImage_Initialise(true);
}

void skImage::finalize()
{
    FreeImage_DeInitialise();
}

skImage::skImage() :
    m_width(0),
    m_height(0),
    m_pitch(0),
    m_bpp(0),
    m_size(0),
    m_bytes(nullptr),
    m_bitmap(nullptr),
    m_format(skPixelFormat::SK_ALPHA)
{
}

skImage::skImage(SKuint32 w, SKuint32 h, skPixelFormat fmt) :
    m_width(w),
    m_height(h),
    m_pitch(0),
    m_bpp(0),
    m_size(0),
    m_bytes(nullptr),
    m_bitmap(nullptr),
    m_format(fmt)
{
    calculateBitsPerPixel();
    allocateBytes();
}

skImage::~skImage()
{
    if (m_bitmap)
        FreeImage_Unload(m_bitmap);
}

void skImage::unloadAndReset()
{
    if (m_bitmap)
        FreeImage_Unload(m_bitmap);
    m_width  = 0;
    m_height = 0;
    m_pitch  = 0;
    m_bpp    = 0;
    m_size   = 0;
    m_bytes  = nullptr;
    m_bitmap = nullptr;
    m_format = skPixelFormat::SK_ALPHA;
}

void skImage::calculateFormat()
{
    switch (m_bpp)
    {
    case 8:
    case 1:
    default:
        m_format = skPixelFormat::SK_ALPHA;
        break;
    case 16:
    case 2:
        m_format = skPixelFormat::SK_LUMINANCE_ALPHA;
        break;
    case 24:
    case 3:
        m_format = skPixelFormat::SK_RGB;
        break;
    case 4:
    case 32:
        m_format = skPixelFormat::SK_RGBA;
        break;
    }
}

void skImage::calculateBitsPerPixel()
{
    switch (m_format)
    {
    case skPixelFormat::SK_LUMINANCE:
    case skPixelFormat::SK_ALPHA:
        m_bpp = 1;
        break;
    case skPixelFormat::SK_LUMINANCE_ALPHA:
        m_bpp = 2;
        break;
    case skPixelFormat::SK_RGB:
    case skPixelFormat::SK_BGR:
        m_bpp = 3;
        break;
    case skPixelFormat::SK_RGBA:
    case skPixelFormat::SK_BGRA:
    case skPixelFormat::SK_ARGB:
    case skPixelFormat::SK_ABGR:
        m_bpp = 4;
        break;
    default:
        break;
    }
}

void skImage::save(int fmt, const char* file)
{
    FREE_IMAGE_FORMAT out = ImageUtils::getFormat(fmt);
    if (m_bitmap != nullptr && out != FIF_UNKNOWN && file != nullptr)
        FreeImage_Save(out, m_bitmap, file);
}


void skImage::load(int fmt, const char* file)
{
    FREE_IMAGE_FORMAT out = ImageUtils::getFormat(fmt);
    if (out != FIF_UNKNOWN && file != nullptr)
    {
        unloadAndReset();

        m_bitmap = FreeImage_Load(out, file);
        if (m_bitmap != nullptr)
        {
            m_bytes = FreeImage_GetBits(m_bitmap);
            m_bpp   = FreeImage_GetBPP(m_bitmap);
            m_bpp /= 8;
            calculateFormat();

            m_width  = FreeImage_GetWidth(m_bitmap);
            m_height = FreeImage_GetHeight(m_bitmap);
            m_pitch  = FreeImage_GetPitch(m_bitmap);

            m_size = (SKsize)m_width * (SKsize)m_height * (SKsize)m_bpp;
        }
    }
}

void skImage::allocateBytes()
{
    if (!UINTV32(m_width) || !UINTV32(m_height))
    {
        skLogf(LD_ERROR, "Cannot allocate an image with a width or height of 0.\n");
        return;
    }

    if (!UINTV32(m_bpp))
    {
        skLogf(LD_ERROR, "Unknown number of bits per pixel.\n");
        return;
    }

    if (m_bitmap != nullptr)
        FreeImage_Unload(m_bitmap);

    m_bitmap = FreeImage_Allocate(m_width, m_height, 8 * m_bpp);
    m_size   = (SKsize)m_width * (SKsize)m_height * (SKsize)m_bpp;
    m_bpp    = FreeImage_GetBPP(m_bitmap);
    m_bpp /= 8;
    m_bytes = FreeImage_GetBits(m_bitmap);
    m_pitch  = FreeImage_GetPitch(m_bitmap);
}

void skImage::clear(const skPixel& px)
{
    switch (m_format)
    {
    case skPixelFormat::SK_LUMINANCE:
    case skPixelFormat::SK_ALPHA:
        ImageUtils::clearA(m_bytes, m_size, px);
        break;
    case skPixelFormat::SK_LUMINANCE_ALPHA:
        ImageUtils::clearLA(m_bytes, m_size, px);
        break;
    case skPixelFormat::SK_RGB:
    case skPixelFormat::SK_BGR:
        ImageUtils::clearRGB(m_bytes, m_size, px);
        break;
    case skPixelFormat::SK_RGBA:
    case skPixelFormat::SK_BGRA:
    case skPixelFormat::SK_ARGB:
    case skPixelFormat::SK_ABGR:
        ImageUtils::clearRGBA(m_bytes, m_size, px);
        break;
    default:
        break;
    }
}


void skImage::getPixel(skPixel& dest, SKubyte* src, skPixelFormat fmt)
{
    skPixel* rs = &dest;
    switch (fmt)
    {
    case skPixelFormat::SK_BGR:
    {
        auto* value = (skPixelRGB*)src;

        rs->r = value->b;
        rs->g = value->g;
        rs->b = value->r;
        rs->a = 255;
        break;
    }
    case skPixelFormat::SK_RGB:
    {
        auto* value = (skPixelRGB*)src;

        rs->r = value->r;
        rs->g = value->g;
        rs->b = value->b;
        rs->a = 255;
        break;
    }
    case skPixelFormat::SK_RGBA:
    {
        auto* value = (skPixelRGBA*)src;
        rs->r       = value->r;
        rs->g       = value->g;
        rs->b       = value->b;
        rs->a       = value->a;
        break;
    }
    case skPixelFormat::SK_BGRA:
    {
        auto* value = (skPixelRGBA*)src;
        rs->r       = value->b;
        rs->g       = value->g;
        rs->b       = value->r;
        rs->a       = value->a;
        break;
    }
    case skPixelFormat::SK_ARGB:
    {
        auto* value = (skPixelRGBA*)src;
        rs->r       = value->a;
        rs->g       = value->r;
        rs->b       = value->g;
        rs->a       = value->b;
        break;
    }
    case skPixelFormat::SK_ABGR:
    {
        auto* value = (skPixelRGBA*)src;
        rs->r       = value->a;
        rs->g       = value->b;
        rs->b       = value->g;
        rs->a       = value->r;
        break;
    }
    case skPixelFormat::SK_LUMINANCE_ALPHA:
    {
        auto* la = (skPixelLA*)src;
        rs->r    = la->l;
        rs->g    = la->l;
        rs->b    = la->l;
        rs->a    = la->a;
        break;
    }
    case skPixelFormat::SK_LUMINANCE:
    case skPixelFormat::SK_ALPHA:
    {
        rs->r = src[0];
        rs->g = rs->r;
        rs->b = rs->r;
        rs->a = rs->r;
        break;
    }
    default:
        break;
    }
}

void skImage::setPixel(SKubyte* dst, const skPixel& src, skPixelFormat fmt)
{
    switch (fmt)
    {
    case skPixelFormat::SK_BGR:
    {
        auto* p = (skPixelRGB*)dst;
        p->b    = src.r;
        p->g    = src.g;
        p->r    = src.b;
        break;
    }
    case skPixelFormat::SK_RGB:
    {
        auto* p = (skPixelRGB*)dst;
        p->r    = src.r;
        p->g    = src.g;
        p->b    = src.b;
        break;
    }
    case skPixelFormat::SK_RGBA:
    {
        auto* p = (skPixelRGBA*)dst;
        p->r    = src.r;
        p->g    = src.g;
        p->b    = src.b;
        p->a    = src.a;
        break;
    }
    case skPixelFormat::SK_BGRA:
    {
        auto* p = (skPixelRGBA*)dst;
        p->b    = src.r;
        p->g    = src.g;
        p->r    = src.b;
        p->a    = src.a;
        break;
    }
    case skPixelFormat::SK_ARGB:
    {
        auto* p = (skPixelRGBA*)dst;
        p->a    = src.r;
        p->r    = src.g;
        p->g    = src.b;
        p->b    = src.a;
        break;
    }
    case skPixelFormat::SK_ABGR:
    {
        auto* p = (skPixelRGBA*)dst;
        p->a    = src.r;
        p->b    = src.g;
        p->g    = src.b;
        p->r    = src.a;
        break;
    }
    case skPixelFormat::SK_LUMINANCE_ALPHA:
    {
        auto* p = (skPixelLA*)dst;

        int lc = src.r;
        lc += src.g;
        lc += src.b;
        lc /= 3;
        p->l = (SKubyte)lc;
        p->a = src.a;
        break;
    }
    case skPixelFormat::SK_LUMINANCE:
        dst[0] = src.r;
        break;
    case skPixelFormat::SK_ALPHA:
        dst[0] = src.a;
        break;
    }
}


void skImage::setPixel(SKuint32 x, SKuint32 y, const skPixel& pixel)
{
    const SKuint32 loc = GETBUFFERPOS(x, y);
    if (loc < m_size)
        setPixel(&m_bytes[loc], pixel, m_format);
}


void skImage::fillRect(SKuint32       x,
                       SKuint32       y,
                       SKuint32       width,
                       SKuint32       height,
                       const skPixel& col)
{
    SKuint32 x0, x1, y0, y1;
    x0 = x;
    x1 = x + width;
    y0 = y;
    y1 = y + height;


    for (SKuint32 ix = x0; ix < x1; ++ix)
    {
        for (SKuint32 iy = y0; iy < y1; ++iy)
        {
            const SKuint32 loc = GETBUFFERPOS(ix, iy);
            if (loc < m_size)
                setPixel(&m_bytes[loc], col, m_format);
        }
    }
}


void skImage::lineTo(SKint32        x1,
                     SKint32        y1,
                     SKint32        x2,
                     SKint32        y2,
                     const skPixel& col)
{
    if (x2 == x1)
    {
        if (y2 < y1)
            skSwap(y2, y1);

        for (SKint32 iy = y1; iy < y2; ++iy)
        {
            const SKuint32 loc = GETBUFFERPOS(x1, iy);
            if (loc < m_size)
                setPixel(&m_bytes[loc], col, m_format);
        }
    }
    else if (y2 == y1)
    {
        if (x2 < x1)
            skSwap(x2, x1);
        for (SKint32 ix = x1; ix < x2; ++ix)
        {
            const SKuint32 loc = GETBUFFERPOS(ix, y1);
            if (loc < m_size)
                setPixel(&m_bytes[loc], col, m_format);
        }
    }
}



void skImage::copy(SKubyte*       dst,
                   const SKubyte* src,
                   SKuint32       w,
                   SKuint32       h,
                   skPixelFormat  dstFmt,
                   skPixelFormat  srcFmt)
{
    if (!dst || !src)
        return;

    if (dstFmt == srcFmt)
    {
        skMemcpy(dst, src, (SKsize)w * (SKsize)h * (SKsize)getSize(dstFmt));
        return;
    }

    const SKuint32 sbpp = getSize(srcFmt);
    const SKuint32 dbpp = getSize(dstFmt);

    SKubyte *dp, *sp;
    for (SKuint32 y = 0; y < h; y++)
    {
        for (SKuint32 x = 0; x < w; x++)
        {
            sp = (SKubyte*)&src[(y * w + x) * sbpp];
            dp = (SKubyte*)&dst[(y * w + x) * dbpp];

            skPixel rs(0, 0, 0, 255);
            getPixel(rs, sp, srcFmt);
            setPixel(dp, rs, dstFmt);
        }
    }
}

SKuint32 skImage::getSize(skPixelFormat fmt)
{
    switch (fmt)
    {
    case skPixelFormat::SK_BGR:
    case skPixelFormat::SK_RGB:
        return 3;
    case skPixelFormat::SK_ABGR:
    case skPixelFormat::SK_ARGB:
    case skPixelFormat::SK_BGRA:
    case skPixelFormat::SK_RGBA:
        return 4;
    case skPixelFormat::SK_LUMINANCE_ALPHA:
        return 2;
    case skPixelFormat::SK_LUMINANCE:
    case skPixelFormat::SK_ALPHA:
        return 1;
    default:
        return 0;
    }
}

skPixelFormat skImage::getFormat(SKuint32 bpp)
{
    switch (bpp)
    {
    case 3:
        return skPixelFormat::SK_RGB;
    case 4:
        return skPixelFormat::SK_RGBA;
    case 2:
        return skPixelFormat::SK_LUMINANCE_ALPHA;
    case 1:
    default:
        return skPixelFormat::SK_ALPHA;
    }
}
