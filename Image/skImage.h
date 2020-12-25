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
#ifndef _skImage_h_
#define _skImage_h_

#include "Utils/Config/skConfig.h"
#include "skPixel.h"

class skImage
{
private:
    SKuint32      m_width;
    SKuint32      m_height;
    SKuint32      m_pitch;
    SKuint32      m_bpp;
    SKsize        m_size;
    SKubyte*      m_bytes;
    skPixelFormat m_format;
    FIBITMAP*     m_bitmap;

    void unloadAndReset();
    void allocateBytes();
    void calculateBitsPerPixel();
    void calculateFormat();

    void copy(SKubyte*       dst,
              const SKubyte* src,
              SKuint32       w,
              SKuint32       h,
              skPixelFormat  dstFmt,
              skPixelFormat  srcFmt);

    SKuint32      getSize(skPixelFormat fmt);
    skPixelFormat getFormat(SKuint32 bpp);

    void getPixel(skPixel& dest, SKubyte* src, skPixelFormat fmt);
    void setPixel(SKubyte* dst, const skPixel& src, skPixelFormat fmt);

public:
    skImage();
    skImage(SKuint32 w, SKuint32 h, skPixelFormat fmt);
    virtual ~skImage();

    SK_INLINE SKuint32 getWidth(void)
    {
        return m_width;
    }

    SK_INLINE SKuint32 getHeight(void)
    {
        return m_height;
    }

    SK_INLINE SKuint32 getBPP(void)
    {
        return m_bpp;
    }

    SK_INLINE SKubyte* getBytes(void)
    {
        return m_bytes;
    }

    SK_INLINE skPixelFormat getFormat(void)
    {
        return m_format;
    }

    void clear(const skPixel& pixel);
    void setPixel(SKuint32 x, SKuint32 y, const skPixel &pixel);

    void fillRect(SKuint32 x,
                  SKuint32 y,
                  SKuint32 width,
                  SKuint32 height,
                  const skPixel& col);

    void lineTo(SKint32 x1,
                SKint32 y1,
                SKint32 x2,
                SKint32 y2,
                const skPixel& col);


    void save(int fmt, const char* file);
    void load(int fmt, const char* file);


    static void initialize();
    static void finalize();
};

#endif  //_skImage_h_
