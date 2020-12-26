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
#include "skPixel.h"
#include "Utils/skMinMax.h"


skPixel::skPixel(const SKuint32& col)
{
    skColorUnion c{};
    c.i = col;
    r   = c.b[SK_rIdx];
    g   = c.b[SK_gIdx];
    b   = c.b[SK_bIdx];
    a   = c.b[SK_aIdx];
}


void skPixel::add(const skPixel& px)
{
    SKint16 tr = r, tg = g, tb = b, ta = a;
    tr += px.r;
    tg += px.g;
    tb += px.b;
    ta += px.a;
    r = (SKuint8)skMin<SKint16>(tr, 255);
    g = (SKuint8)skMin<SKint16>(tg, 255);
    b = (SKuint8)skMin<SKint16>(tb, 255);
    a = (SKuint8)skMin<SKint16>(ta, 255);
}

void skPixel::sub(const skPixel& px)
{
    SKint16 tr = r, tg = g, tb = b, ta = a;
    tr -= px.r;
    tg -= px.g;
    tb -= px.b;
    ta -= px.a;
    r = (SKuint8)skMax<SKint16>(tr, 0);
    g = (SKuint8)skMax<SKint16>(tg, 0);
    b = (SKuint8)skMax<SKint16>(tb, 0);
    a = (SKuint8)skMax<SKint16>(ta, 0);
}

void skPixel::mul(const skPixel& px)
{
    SKint16 tr = r, tg = g, tb = b, ta = a;
    tr *= px.r;
    tg *= px.g;
    tb *= px.b;
    ta *= px.a;
    r = (SKuint8)skClamp<SKint16>(tr, 0, 255);
    g = (SKuint8)skClamp<SKint16>(tg, 0, 255);
    b = (SKuint8)skClamp<SKint16>(tb, 0, 255);
    a = (SKuint8)skClamp<SKint16>(ta, 0, 255);
}

void skPixel::div(const skPixel& px)
{
    if (px.r > 0)
        r /= px.r;
    if (px.g > 0)
        g /= px.g;
    if (px.b > 0)
        b /= px.b;
    if (px.a > 0)
        a /= px.a;
}

void skPixel::mix(const skPixel& px, double f)
{
    double d = (double)f, d1, d2;
    if (d < 0)
        d = 0;
    if (d > 1)
        d = 1;

    d1 = d;
    d2 = 1.0 - d;
    r  = (SKuint8)((d2 * (double)r) + (double)px.r * d1);
    g  = (SKuint8)((d2 * (double)g) + (double)px.g * d1);
    b  = (SKuint8)((d2 * (double)b) + (double)px.b * d1);
    a  = (SKuint8)((d2 * (double)a) + (double)px.a * d1);
}
