/* ============================================================
 * File  : border.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2005-05-25
 * Description : border threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006 by Gilles Caulier and Marcel Wiesweg
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

// C++ includes. 
 
#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qpixmap.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qpen.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "border.h"

namespace DigikamBorderImagesPlugin
{

Border::Border(Digikam::DImg *orgImage, QObject *parent, int orgWidth, int orgHeight,
               QString borderPath, int borderType, 
               int borderWidth1, int borderWidth2, int borderWidth3, int borderWidth4,
               Digikam::DColor solidColor, 
               Digikam::DColor niepceBorderColor,
               Digikam::DColor niepceLineColor, 
               Digikam::DColor bevelUpperLeftColor,
               Digikam::DColor bevelLowerRightColor, 
               Digikam::DColor decorativeFirstColor,
               Digikam::DColor decorativeSecondColor)
      : Digikam::DImgThreadedFilter(orgImage, parent, "Border")
{ 
    m_orgWidth     = orgWidth;
    m_orgHeight    = orgHeight;

    m_borderType   = borderType;
    m_borderWidth1 = borderWidth1;
    m_borderWidth2 = borderWidth2;
    m_borderWidth3 = borderWidth3;
    m_borderWidth4 = borderWidth4;
    
    m_solidColor            = solidColor;
    m_niepceBorderColor     = niepceBorderColor;
    m_niepceLineColor       = niepceLineColor;
    m_bevelUpperLeftColor   = bevelUpperLeftColor;
    m_bevelLowerRightColor  = bevelLowerRightColor;
    m_decorativeFirstColor  = decorativeFirstColor;
    m_decorativeSecondColor = decorativeSecondColor;
    
    m_borderPath = borderPath;
    
    initFilter();
}

void Border::filterImage(void)
{
    switch (m_borderType)
    {
       case SolidBorder:
          solid(m_orgImage, m_destImage, m_solidColor, m_borderWidth1);
          break;
       
       case NiepceBorder:
          niepce(m_orgImage, m_destImage, m_niepceBorderColor, m_borderWidth1, 
                 m_niepceLineColor, m_borderWidth4);
          break;

       case BeveledBorder:
          bevel(m_orgImage, m_destImage, m_bevelUpperLeftColor,
                m_bevelLowerRightColor, m_borderWidth1);
          break;

       case PineBorder:
       case WoodBorder: 
       case PaperBorder: 
       case ParqueBorder: 
       case IceBorder:
       case LeafBorder: 
       case MarbleBorder:
       case RainBorder:
       case CratersBorder:
       case DriedBorder:
       case PinkBorder:
       case StoneBorder:
       case ChalkBorder:
       case GraniteBorder:
       case RockBorder:
       case WallBorder:
          pattern( m_orgImage, m_destImage, m_borderWidth1, 
                   m_decorativeFirstColor, m_decorativeSecondColor,
                   m_borderWidth2, m_borderWidth2 );
          break;
    }
}

void Border::solid(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &fg, int borderWidth)
{
    dest = Digikam::DImg(src.width() + borderWidth*2, src.height() + borderWidth*2, src.sixteenBit(), src.hasAlpha());
    dest.fill(fg);
    dest.bitBltImage(&src, borderWidth, borderWidth);
}

void Border::niepce(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &fg, int borderWidth, 
                                const Digikam::DColor &bg, int lineWidth)
{
    Digikam::DImg tmp;
    solid(src, tmp, bg, lineWidth);
    solid(tmp, dest, fg, borderWidth);
}

void Border::bevel(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &topColor, 
                               const Digikam::DColor &btmColor, int borderWidth)
{
    int x, y;
    int wc;

    dest = Digikam::DImg(src.width() + borderWidth*2, src.height() + borderWidth*2, src.sixteenBit(), src.hasAlpha());

    // top

    for(y=0, wc = (int)dest.width()-1; y < borderWidth; ++y, --wc)
    {
        for(x=0; x < wc; ++x)
            dest.setPixelColor(x, y, topColor);

        for(;x < (int)dest.width(); ++x)
            dest.setPixelColor(x, y, btmColor);
    }

    // left and right

    for(; y < (int)dest.height()-borderWidth; ++y)
    {
       for(x=0; x < borderWidth; ++x)
           dest.setPixelColor(x, y, topColor);

       for(x = (int)dest.width()-1; x > (int)dest.width()-borderWidth-1; --x)
           dest.setPixelColor(x, y, btmColor);
    }

    // bottom

    for(wc = borderWidth; y < (int)dest.height(); ++y, --wc)
    {
       for(x=0; x < wc; ++x)
           dest.setPixelColor(x, y, topColor);

       for(; x < (int)dest.width(); ++x)
           dest.setPixelColor(x, y, btmColor);
    }

    dest.bitBltImage(&src, borderWidth, borderWidth);
}

void Border::pattern(Digikam::DImg &src, Digikam::DImg &dest, int borderWidth,
                     const Digikam::DColor &firstColor, const Digikam::DColor &secondColor, 
                     int firstWidth, int secondWidth)
{
    // Border tile.

    int w = m_orgWidth + borderWidth*2;
    int h = m_orgHeight + borderWidth*2;
    kdDebug() << "Border File:" << m_borderPath << endl;
    Digikam::DImg border(m_borderPath);
    if ( border.isNull() )
        return;

    Digikam::DImg borderImg(w, h, src.sixteenBit(), src.hasAlpha());
    border.convertToDepthOfImage(&borderImg);

    for (int x = 0 ; x < w ; x+=border.width())
        for (int y = 0 ; y < h ; y+=border.height())
            borderImg.bitBltImage(&border, x, y);

    // First line around the pattern tile.
    Digikam::DImg tmp = borderImg.smoothScale( src.width() + borderWidth*2,
                                                src.height() + borderWidth*2 );

    solid(tmp, dest, firstColor, firstWidth);

    // Second line around original image.
    tmp.reset();
    solid(src, tmp, secondColor, secondWidth);

    // Copy original image.
    dest.bitBltImage(&tmp, borderWidth, borderWidth);
}

}  // NameSpace DigikamBorderImagesPlugin
