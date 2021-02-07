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
#include "Image/skImage.h"
#include "Image/skPalette.h"


int main(int argc, char** argv)
{
    skImage::initialize();
    {
        skImage ima(140, 40, skPixelFormat::SK_RGB);
        ima.clear(skPalette::Grey02);

        ima.fillRect(0, 0, 20, 20, skPalette::Red);
        ima.fillRect(20, 0, 20, 20, skPalette::Orange);
        ima.fillRect(40, 0, 20, 20, skPalette::Yellow);
        ima.fillRect(60, 0, 20, 20, skPalette::Cyan);
        ima.fillRect(80, 0, 20, 20, skPalette::Blue);
        ima.fillRect(100, 0, 20, 20, skPalette::Green);
        ima.fillRect(120, 0, 20, 20, skPalette::Magenta);

        ima.fillRect(0, 20, 10, 20, skPalette::White);
        ima.fillRect(10, 20, 10, 20, skPalette::Grey09);
        ima.fillRect(20, 20, 10, 20, skPalette::Grey08);
        ima.fillRect(30, 20, 10, 20, skPalette::Grey07);
        ima.fillRect(40, 20, 10, 20, skPalette::Grey06);
        ima.fillRect(50, 20, 10, 20, skPalette::Grey05);
        ima.fillRect(60, 20, 10, 20, skPalette::Grey04);
        ima.fillRect(70, 20, 10, 20, skPalette::Grey03);
        ima.fillRect(80, 20, 10, 20, skPalette::Grey02);
        ima.fillRect(90, 20, 10, 20, skPalette::Grey01);

        ima.save("test.bmp");
    }
    {
        skImage ima(512, 512, skPixelFormat::SK_RGBA);
        ima.clear(skPalette::Grey02);


        int pointsY[16] = {15, 5, 17, 56, 32, 77, 200, 63, 99, 45, 88, 123, 500, 300, 100, 7};

        int stp = 512 / 15;
        int j   = 0;
        int fx = 0, fy = 0, tx, ty;
        for (int i = 0; i < 16; ++i, j += stp)
        {
            if (i == 0)
            {
                fx = j;
                fy = 512 - pointsY[i];
                ima.lineTo(0, 512, fx, fy, skPalette::Blue);
            }
            else
            {
                tx = j;
                ty = 512 - pointsY[i];

                ima.lineTo(fx, fy, tx, ty, skPalette::Green);

                fx = tx;
                fy = ty;
            }
        }
        ima.lineTo(0, 0, ima.getWidth(), 0, skPalette::Red);
        ima.lineTo(0, 0, 0, ima.getHeight(), skPalette::Orange);
        ima.lineTo(ima.getWidth() - 1, 0, ima.getWidth() - 1, ima.getHeight(), skPalette::Yellow);
        ima.lineTo(0, ima.getHeight() - 1, ima.getWidth(), ima.getHeight() - 1, skPalette::Green);

        ima.save("test1.png");
    }
    {
        skImage ima(512, 512, skPixelFormat::SK_RGB);
        ima.clear(skPalette::Grey09);

        ima.strokeRect(20,20,200,200, skPalette::Black);
        ima.save("test2.jpg");
    }
    skImage::finalize();
    return 0;
}
