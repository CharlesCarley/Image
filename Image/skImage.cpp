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
#include "skImage.h"
#include "FreeImage.h"
#include "Utils/skLogger.h"
#include "Utils/skMemoryUtils.h"
#include "Utils/skMinMax.h"
#include "Utils/skPlatformHeaders.h"


class ImageUtils
{
public:
    static int getFormat(const int format)
    {
        int out = (int)FIF_UNKNOWN;
        switch (format)
        {
        case FIF_BMP:
            out = FIF_BMP;
            break;
        case FIF_JPEG:
            out = FIF_JPEG;
            break;
        case FIF_J2K:
            out = FIF_J2K;
            break;
        case FIF_PNG:
            out = FIF_PNG;
            break;
        case FIF_PSD:
            out = FIF_PSD;
            break;
        case FIF_TARGA:
            out = FIF_TARGA;
            break;
        case FIF_XPM:
            out = FIF_XPM;
            break;
        default:
            break;
        }
        return out;
    }

    static void clearA(SKubyte* mem, const SKsize max, const skPixel& p)
    {
        skMemset(mem, p.a, max);
    }

    static void clearLa(SKubyte* mem, const SKsize max, const skPixel& p)
    {
        if (mem && max > 2)
        {
            SKubyte cp[2] = {
                (SKubyte)(((int)p.r + (int)p.g + (int)p.b) / 3),
                p.a,
            };

            SKsize i;
            for (i = 0; i < max - 1; i += 2)
            {
#if SK_ENDIAN == SK_ENDIAN_BIG
                mem[i]     = cp[0];
                mem[i + 1] = cp[1];
#else
                mem[i + 1] = cp[0];
                mem[i]     = cp[1];
#endif
            }
        }
    }

    static void clearRgb(SKubyte* mem, const SKsize max, const skPixel& p)
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

    static void clearRgba(SKubyte* mem, const SKsize max, const skPixel& p)
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
                mem[i]     = p.b;
                mem[i + 1] = p.g;
                mem[i + 2] = p.r;
                mem[i + 3] = p.a;
#endif
            }
        }
    }
};


skImage::skImage() :
    m_width(0),
    m_height(0),
    m_pitch(0),
    m_bpp(0),
    m_size(0),
    m_bytes(nullptr),
    m_format(skPixelFormat::SK_ALPHA),
    m_bitmap(nullptr)
{
}

skImage::skImage(const SKuint32      width,
                 const SKuint32      height,
                 const skPixelFormat format) :
    m_width(width),
    m_height(height),
    m_pitch(0),
    m_bpp(0),
    m_size(0),
    m_bytes(nullptr),
    m_format(format),
    m_bitmap(nullptr)
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
    case skPixelFormat::SK_PF_MAX:
        m_bpp = 0;
        break;
    }
}

void skImage::save(const char* file) const
{
    int       fmt = FreeImage_GetFIFFromFilename(file);
    const int out = ImageUtils::getFormat(fmt);

    if (m_bitmap != nullptr && out != FIF_UNKNOWN && file != nullptr)
        FreeImage_Save((FREE_IMAGE_FORMAT)out, m_bitmap, file);
}


bool skImage::load(const char* file)
{
    int       fmt = FreeImage_GetFIFFromFilename(file);
    const int out = ImageUtils::getFormat(fmt);

    if (out != FIF_UNKNOWN && file != nullptr)
    {
        unloadAndReset();

        m_bitmap = FreeImage_Load((FREE_IMAGE_FORMAT)out, file);
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
            return true;
        }
    }
    return false;
}

void skImage::allocateBytes()
{
    if (!(m_width > 0 && m_width < SK_NPOS32) ||
        !(m_height > 0 && m_height < SK_NPOS32))
    {
        skLogf(LD_ERROR, "The specified image dimensions exceed the limit of the data type.\n");
        return;
    }

    if (!(m_bpp > 0 && m_bpp < SK_NPOS32))
    {
        skLogf(LD_ERROR, "Unknown number of bits per pixel.\n");
        return;
    }

    if (m_bitmap != nullptr)
        FreeImage_Unload(m_bitmap);

    m_bitmap = FreeImage_Allocate(m_width, m_height, 8 * (int)m_bpp);
    m_size   = (SKsize)m_width * (SKsize)m_height * (SKsize)m_bpp;
    m_bpp    = FreeImage_GetBPP(m_bitmap) / 8;
    m_bytes  = FreeImage_GetBits(m_bitmap);
    m_pitch  = FreeImage_GetPitch(m_bitmap);
}

void skImage::clear(const skPixel& pixel) const
{
    switch (m_format)
    {
    case skPixelFormat::SK_LUMINANCE:
    case skPixelFormat::SK_ALPHA:
        ImageUtils::clearA(m_bytes, m_size, pixel);
        break;
    case skPixelFormat::SK_LUMINANCE_ALPHA:
        ImageUtils::clearLa(m_bytes, m_size, pixel);
        break;
    case skPixelFormat::SK_RGB:
    case skPixelFormat::SK_BGR:
        ImageUtils::clearRgb(m_bytes, m_size, pixel);
        break;
    case skPixelFormat::SK_RGBA:
    case skPixelFormat::SK_BGRA:
    case skPixelFormat::SK_ARGB:
    case skPixelFormat::SK_ABGR:
        ImageUtils::clearRgba(m_bytes, m_size, pixel);
        break;
    case skPixelFormat::SK_PF_MAX:
        break;
    }
}


void skImage::getPixel(skPixel& dest, const SKubyte* src, const skPixelFormat format)
{
    skPixel* rs = &dest;

    switch (format)
    {
    case skPixelFormat::SK_BGR:
    {
        const skPixelRGB* value = (const skPixelRGB*)src;

        rs->r = value->b;
        rs->g = value->g;
        rs->b = value->r;
        rs->a = 255;
        break;
    }
    case skPixelFormat::SK_RGB:
    {
        const skPixelRGB* value = (const skPixelRGB*)src;

        rs->r = value->r;
        rs->g = value->g;
        rs->b = value->b;
        rs->a = 255;
        break;
    }
    case skPixelFormat::SK_RGBA:
    {
        const skPixelRGBA* value = (const skPixelRGBA*)src;
        rs->r                    = value->r;
        rs->g                    = value->g;
        rs->b                    = value->b;
        rs->a                    = value->a;
        break;
    }
    case skPixelFormat::SK_BGRA:
    {
        const skPixelRGBA* value = (const skPixelRGBA*)src;
        rs->r                    = value->b;
        rs->g                    = value->g;
        rs->b                    = value->r;
        rs->a                    = value->a;
        break;
    }
    case skPixelFormat::SK_ARGB:
    {
        const skPixelRGBA* value = (const skPixelRGBA*)src;
        rs->r                    = value->a;
        rs->g                    = value->r;
        rs->b                    = value->g;
        rs->a                    = value->b;
        break;
    }
    case skPixelFormat::SK_ABGR:
    {
        const skPixelRGBA* value = (const skPixelRGBA*)src;
        rs->r                    = value->a;
        rs->g                    = value->b;
        rs->b                    = value->g;
        rs->a                    = value->r;
        break;
    }
    case skPixelFormat::SK_LUMINANCE_ALPHA:
    {
        const skPixelLA* la = (const skPixelLA*)src;
        rs->r               = la->l;
        rs->g               = la->l;
        rs->b               = la->l;
        rs->a               = la->a;
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
    case skPixelFormat::SK_PF_MAX:
        break;
    }
}

void skImage::setPixel(SKubyte* dst, const skPixel& src, const skPixelFormat format)
{
    switch (format)
    {
    case skPixelFormat::SK_BGR:
    {
        skPixelRGB* p = (skPixelRGB*)dst;
        p->b          = src.r;
        p->g          = src.g;
        p->r          = src.b;
        break;
    }
    case skPixelFormat::SK_RGB:
    {
        skPixelRGB* p = (skPixelRGB*)dst;
        p->r          = src.r;
        p->g          = src.g;
        p->b          = src.b;
        break;
    }
    case skPixelFormat::SK_RGBA:
    {
        skPixelRGBA* p = (skPixelRGBA*)dst;
        p->r           = src.r;
        p->g           = src.g;
        p->b           = src.b;
        p->a           = src.a;
        break;
    }
    case skPixelFormat::SK_BGRA:
    {
        skPixelRGBA* p = (skPixelRGBA*)dst;
        p->b           = src.r;
        p->g           = src.g;
        p->r           = src.b;
        p->a           = src.a;
        break;
    }
    case skPixelFormat::SK_ARGB:
    {
        skPixelRGBA* p = (skPixelRGBA*)dst;
        p->a           = src.r;
        p->r           = src.g;
        p->g           = src.b;
        p->b           = src.a;
        break;
    }
    case skPixelFormat::SK_ABGR:
    {
        skPixelRGBA* p = (skPixelRGBA*)dst;
        p->a           = src.r;
        p->b           = src.g;
        p->g           = src.b;
        p->r           = src.a;
        break;
    }
    case skPixelFormat::SK_LUMINANCE_ALPHA:
    {
        skPixelLA* p = (skPixelLA*)dst;

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
    case skPixelFormat::SK_PF_MAX:
        break;
    }
}


void skImage::setPixel(const SKuint32& x, const SKuint32& y, const skPixel& pixel) const
{
    const SKuint32 loc = getBufferPos(x, y);
    if (loc < m_size)
        setPixel(&m_bytes[loc], pixel, m_format);
}


void skImage::getPixel(const SKuint32& x, const SKuint32& y, skPixel& pixel) const
{
    const SKuint32 loc = getBufferPos(x, y);
    if (loc < m_size)
        getPixel(pixel, &m_bytes[loc], m_format);
}

void skImage::fillRect(const SKuint32 x,
                       const SKuint32 y,
                       const SKuint32 width,
                       const SKuint32 height,
                       const skPixel& col) const
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
            const SKuint32 loc = getBufferPos(ix, iy);
            if (loc <= m_size)
                setPixel(&m_bytes[loc], col, m_format);
        }
    }
}


void skImage::strokeRect(SKuint32       x,
                         SKuint32       y,
                         SKuint32       width,
                         SKuint32       height,
                         const skPixel& col) const
{
    SKuint32 x1, y1, x2, y2;
    x1 = x;
    y1 = y;
    x2 = x1 + width;
    y2 = y1 + height;

    lineTo(x1, y1, x2, y1, col);
    lineTo(x2, y1, x2, y2, col);
    lineTo(x2, y2, x1, y2, col);
    lineTo(x1, y2, x1, y1, col);
}

void skImage::lineTo(SKint32        x1,
                     SKint32        y1,
                     SKint32        x2,
                     SKint32        y2,
                     const skPixel& col) const
{
    if (x2 == x1)
    {
        if (y2 < y1)
            skSwap(y2, y1);

        for (SKint32 iy = y1; iy <= y2; ++iy)
        {
            const SKuint32 loc = getBufferPos(x1, iy);
            if (loc <= m_size)
                setPixel(&m_bytes[loc], col, m_format);
        }
    }
    else if (y2 == y1)
    {
        if (x2 < x1)
            skSwap(x2, x1);
        for (SKint32 ix = x1; ix <= x2; ++ix)
        {
            const SKuint32 loc = getBufferPos(ix, y1);
            if (loc <= m_size)
                setPixel(&m_bytes[loc], col, m_format);
        }
    }
    else if (skABS(y2 - y1) > skABS(x2 - x1))
    {
        skSwap(x1, y1);
        skSwap(x2, y2);

        if (x1 > x2)
        {
            skSwap(x1, x2);
            skSwap(y1, y2);
        }

        const SKint32 de = skABS(y2 - y1);
        const SKint32 sy = y1 > y2 ? -1 : 1;
        const SKint32 dx = x2 - x1;

        SKint32 e = -(dx >> 1);
        SKint32 iy, ix;
        iy = y1;
        for (ix = x1; ix <= x2; ++ix)
        {
            const SKuint32 loc = getBufferPos(iy, ix);
            if (loc <= m_size)
                setPixel(&m_bytes[loc], col, m_format);

            e += de;
            if (e > 0)
            {
                iy += sy;
                e -= dx;
            }
        }
    }
    else
    {
        if (x1 > x2)
        {
            skSwap(x1, x2);
            skSwap(y1, y2);
        }

        const SKint32 de = skABS(y2 - y1);
        const SKint32 sy = y1 > y2 ? -1 : 1;
        const SKint32 dx = x2 - x1;

        SKint32 e = -(dx >> 1);
        SKint32 iy, ix;
        iy = y1;
        for (ix = x1; ix <= x2; ++ix)
        {
            const SKuint32 loc = getBufferPos(ix, iy);
            if (loc <= m_size)
                setPixel(&m_bytes[loc], col, m_format);

            e += de;
            if (e > 0)
            {
                iy += sy;
                e -= dx;
            }
        }
    }
}


void skImage::copy(SKubyte*            dst,
                   const SKubyte*      src,
                   const SKuint32      w,
                   const SKuint32      h,
                   const skPixelFormat dstFmt,
                   const skPixelFormat srcFmt) const
{
    if (!dst || !src)
        return;

    if (dstFmt == srcFmt)
    {
        skMemcpy(dst, src, (SKsize)w * (SKsize)h * (SKsize)getSize(dstFmt));
        return;
    }

    const SKuint32 srcBpp = getSize(srcFmt);
    const SKuint32 dstBpp = getSize(dstFmt);

    SKubyte* dp;
    for (SKuint32 y = 0; y < h; y++)
    {
        for (SKuint32 x = 0; x < w; x++)
        {
            const SKubyte* sp = (const SKubyte*)&src[(y * w + x) * srcBpp];

            dp = (SKubyte*)&dst[(y * w + x) * dstBpp];

            skPixel rs(0, 0, 0, 255);
            getPixel(rs, sp, srcFmt);
            setPixel(dp, rs, dstFmt);
        }
    }
}

SKuint32 skImage::getSize(const skPixelFormat& format)
{
    switch (format)
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
    case skPixelFormat::SK_PF_MAX:
        return 0;
    }
    return 0;
}

skPixelFormat skImage::getFormat(const SKuint32 bpp)
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

void FreeImage_MessageProc(int, const char* msg)
{
    if (msg)
    {
        if (skLogger::getSingletonPtr())
            skLogd(LD_ERROR, msg);
        else
            printf("-- %s\n", msg);
    }
}

void skImage::initialize()
{
    FreeImage_SetOutputMessage((FreeImage_OutputMessageFunction)FreeImage_MessageProc);
    FreeImage_Initialise(true);
}

void skImage::finalize()
{
    // Metadata/FreeImageTag will show up as memory leaks because 
    // TagLib::instance is defined as a static instance on
    // the stack which then needs an atexit ~TagLib call. 
    FreeImage_DeInitialise();
}
