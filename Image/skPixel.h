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

#include "Image/skImageTypes.h"
#include "Utils/Config/skConfig.h"
struct FIBITMAP;

class skPixel
{
public:
    skPixel() :
        r(0),
        g(0),
        b(0),
        a(255)
    {
    }

    skPixel(const skPixel& rhs) = default;

    skPixel(const SKubyte pr, const SKubyte pg, const SKubyte pb, const SKubyte pa) :
        r(pr),
        g(pg),
        b(pb),
        a(pa)
    {
    }

    explicit skPixel(const SKuint32& col);

    double lum() const;

    void set(const skPixel& px);
    void add(const skPixel& px);
    void sub(const skPixel& px);
    void mul(const skPixel& px);
    void div(const skPixel& px);
    void mix(const skPixel& px, double f);

public:
    SKubyte r, g, b, a;
};


#endif  //_skPixel_h_
