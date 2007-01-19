/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2005-05-25
 * Description : border threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
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

#include <qpoint.h>
#include <qregion.h>
#include <qpointarray.h>

// Local includes.

#include "border.h"

namespace DigikamBorderImagesPlugin
{

Border::Border(Digikam::DImg *image, QObject *parent, int orgWidth, int orgHeight,
               QString borderPath, int borderType, float borderPercent,
               Digikam::DColor solidColor, 
               Digikam::DColor niepceBorderColor,
               Digikam::DColor niepceLineColor, 
               Digikam::DColor bevelUpperLeftColor,
               Digikam::DColor bevelLowerRightColor, 
               Digikam::DColor decorativeFirstColor,
               Digikam::DColor decorativeSecondColor)
      : Digikam::DImgThreadedFilter(image, parent, "Border")
{ 
    m_orgWidth        = orgWidth;
    m_orgHeight       = orgHeight;
    m_orgRatio        = (float)m_orgWidth / (float)m_orgHeight;
    m_borderType      = borderType;
    m_borderPath      = borderPath;
    int size          = (image->width() > image->height()) ? image->height() : image->width();
    m_borderMainWidth = (int)(size * borderPercent);
    m_border2ndWidth  = (int)(size * 0.005);
    
    // Clamp internal border with to 1 pixel to be visible with small image.    
    if (m_border2ndWidth < 1) m_border2ndWidth = 1; 
    
    m_solidColor            = solidColor;
    m_niepceBorderColor     = niepceBorderColor;
    m_niepceLineColor       = niepceLineColor;
    m_bevelUpperLeftColor   = bevelUpperLeftColor;
    m_bevelLowerRightColor  = bevelLowerRightColor;
    m_decorativeFirstColor  = decorativeFirstColor;
    m_decorativeSecondColor = decorativeSecondColor;

    m_preserveAspectRatio   = true;    

    initFilter();
}

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
    m_orgWidth              = orgWidth;
    m_orgHeight             = orgHeight;

    m_borderType            = borderType;
    m_borderWidth1          = borderWidth1;
    m_borderWidth2          = borderWidth2;
    m_borderWidth3          = borderWidth3;
    m_borderWidth4          = borderWidth4;
    
    m_solidColor            = solidColor;
    m_niepceBorderColor     = niepceBorderColor;
    m_niepceLineColor       = niepceLineColor;
    m_bevelUpperLeftColor   = bevelUpperLeftColor;
    m_bevelLowerRightColor  = bevelLowerRightColor;
    m_decorativeFirstColor  = decorativeFirstColor;
    m_decorativeSecondColor = decorativeSecondColor;
    
    m_borderPath            = borderPath;

    m_preserveAspectRatio   = false;    
    
    initFilter();
}

void Border::filterImage(void)
{
    switch (m_borderType)
    {
        case SolidBorder:
            if (m_preserveAspectRatio)
                solid(m_orgImage, m_destImage, m_solidColor, m_borderMainWidth);
            else
                solid2(m_orgImage, m_destImage, m_solidColor, m_borderWidth1);
            break;
        
        case NiepceBorder:
            if (m_preserveAspectRatio)
                niepce(m_orgImage, m_destImage, m_niepceBorderColor, m_borderMainWidth, 
                       m_niepceLineColor, m_border2ndWidth);
            else
                niepce2(m_orgImage, m_destImage, m_niepceBorderColor, m_borderWidth1, 
                        m_niepceLineColor, m_borderWidth4);
            break;
    
        case BeveledBorder:
            if (m_preserveAspectRatio)
                bevel(m_orgImage, m_destImage, m_bevelUpperLeftColor,
                      m_bevelLowerRightColor, m_borderMainWidth);
            else
                bevel2(m_orgImage, m_destImage, m_bevelUpperLeftColor,
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
            if (m_preserveAspectRatio)
                pattern(m_orgImage, m_destImage, m_borderMainWidth, 
                        m_decorativeFirstColor, m_decorativeSecondColor,
                        m_border2ndWidth, m_border2ndWidth);
            else
                pattern2(m_orgImage, m_destImage, m_borderWidth1, 
                         m_decorativeFirstColor, m_decorativeSecondColor,
                         m_borderWidth2, m_borderWidth2);
            break;
    }
}

// -- Methods to preserve aspect ratio of image ------------------------------------------

void Border::solid(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &fg, int borderWidth)
{
    if (m_orgWidth > m_orgHeight)
    {
        int height = src.height() + borderWidth*2;
        dest = Digikam::DImg((int)(height*m_orgRatio), height, src.sixteenBit(), src.hasAlpha());
        dest.fill(fg);
        dest.bitBltImage(&src, (dest.width()-src.width())/2, borderWidth);
    }
    else
    {
        int width = src.width() + borderWidth*2;
        dest = Digikam::DImg(width, (int)(width/m_orgRatio), src.sixteenBit(), src.hasAlpha());
        dest.fill(fg);
        dest.bitBltImage(&src, borderWidth, (dest.height()-src.height())/2);
    }
}

void Border::niepce(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &fg, 
                    int borderWidth, const Digikam::DColor &bg, int lineWidth)
{
    Digikam::DImg tmp;
    solid(src, tmp, bg, lineWidth);
    solid(tmp, dest, fg, borderWidth);
}

void Border::bevel(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &topColor, 
                   const Digikam::DColor &btmColor, int borderWidth)
{
    int width, height;

    if (m_orgWidth > m_orgHeight)
    {
        height = src.height() + borderWidth*2;
        width  = (int)(height*m_orgRatio);
    }
    else
    {
        width  = src.width() + borderWidth*2;
        height = (int)(width/m_orgRatio);
    }

    dest = Digikam::DImg(width, height, src.sixteenBit(), src.hasAlpha());
    dest.fill(topColor);

    QPointArray btTriangle(3);
    btTriangle.setPoint(0, width, 0);
    btTriangle.setPoint(1, 0, height);
    btTriangle.setPoint(2, width, height);
    QRegion btRegion(btTriangle);

    for(int x=0 ; x < width ; x++)
    {
        for(int y=0 ; y < height ; y++)
        {
            if (btRegion.contains(QPoint(x, y)))
                dest.setPixelColor(x, y, btmColor);
        }
    }

    if (m_orgWidth > m_orgHeight)
    {
        dest.bitBltImage(&src, (dest.width()-src.width())/2, borderWidth);
    }
    else
    {
        dest.bitBltImage(&src, borderWidth, (dest.height()-src.height())/2);
    }
}

void Border::pattern(Digikam::DImg &src, Digikam::DImg &dest, int borderWidth,
                     const Digikam::DColor &firstColor, const Digikam::DColor &secondColor, 
                     int firstWidth, int secondWidth)
{
    // Original image with the first solid border around.
    Digikam::DImg tmp; 
    solid(src, tmp, firstColor, firstWidth);
  
    // Border tiled image using pattern with second solid border around.
    int width, height;

    if (m_orgWidth > m_orgHeight)
    {
        height = tmp.height() + borderWidth*2;
        width  = (int)(height*m_orgRatio);
    }
    else
    {
        width  = tmp.width() + borderWidth*2;
        height = (int)(width/m_orgRatio);
    }

    Digikam::DImg tmp2(width, height, tmp.sixteenBit(), tmp.hasAlpha());
    DDebug() << "Border File:" << m_borderPath << endl;
    Digikam::DImg border(m_borderPath);
    if ( border.isNull() )
        return;
    
    border.convertToDepthOfImage(&tmp2);

    for (int x = 0 ; x < width ; x+=border.width())
        for (int y = 0 ; y < height ; y+=border.height())
            tmp2.bitBltImage(&border, x, y);
      
    solid(tmp2, dest, secondColor, secondWidth);

    // Merge both images to one.
    if (m_orgWidth > m_orgHeight)
    {
        dest.bitBltImage(&tmp, (dest.width()-tmp.width())/2, borderWidth);
    }
    else
    {
        dest.bitBltImage(&tmp, borderWidth, (dest.height()-tmp.height())/2);
    }
}

// -- Methods to not-preserve aspect ratio of image ------------------------------------------


void Border::solid2(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &fg, int borderWidth)
{
    dest = Digikam::DImg(src.width() + borderWidth*2, src.height() + borderWidth*2, 
                         src.sixteenBit(), src.hasAlpha());
    dest.fill(fg);
    dest.bitBltImage(&src, borderWidth, borderWidth);
}

void Border::niepce2(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &fg, int borderWidth, 
                     const Digikam::DColor &bg, int lineWidth)
{
    Digikam::DImg tmp;
    solid2(src, tmp, bg, lineWidth);
    solid2(tmp, dest, fg, borderWidth);
}

void Border::bevel2(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &topColor, 
                    const Digikam::DColor &btmColor, int borderWidth)
{
    int x, y;
    int wc;

    dest = Digikam::DImg(src.width() + borderWidth*2, 
                         src.height() + borderWidth*2, 
                         src.sixteenBit(), src.hasAlpha());

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

void Border::pattern2(Digikam::DImg &src, Digikam::DImg &dest, int borderWidth,
                      const Digikam::DColor &firstColor, const Digikam::DColor &secondColor, 
                      int firstWidth, int secondWidth)
{
    // Border tile.

    int w = m_orgWidth + borderWidth*2;
    int h = m_orgHeight + borderWidth*2;

    DDebug() << "Border File:" << m_borderPath << endl;
    Digikam::DImg border(m_borderPath);
    if ( border.isNull() )
        return;

    Digikam::DImg borderImg(w, h, src.sixteenBit(), src.hasAlpha());
    border.convertToDepthOfImage(&borderImg);

    for (int x = 0 ; x < w ; x+=border.width())
        for (int y = 0 ; y < h ; y+=border.height())
            borderImg.bitBltImage(&border, x, y);

    // First line around the pattern tile.
    Digikam::DImg tmp = borderImg.smoothScale(src.width() + borderWidth*2,
                                              src.height() + borderWidth*2 );

    solid2(tmp, dest, firstColor, firstWidth);

    // Second line around original image.
    tmp.reset();
    solid2(src, tmp, secondColor, secondWidth);

    // Copy original image.
    dest.bitBltImage(&tmp, borderWidth, borderWidth);
}

}  // NameSpace DigikamBorderImagesPlugin
