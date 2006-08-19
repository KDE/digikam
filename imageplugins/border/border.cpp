/* ============================================================
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

#include <qpoint.h>
#include <qregion.h>
#include <qpointarray.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "border.h"

namespace DigikamBorderImagesPlugin
{

Border::Border(Digikam::DImg *image, QObject *parent, int orgWidth, int orgHeight,
               QString borderPath, int borderType, float borderRatio,
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
    m_borderMainWidth = (int)(size * borderRatio);
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
    
    initFilter();
}

void Border::filterImage(void)
{
    switch (m_borderType)
    {
       case SolidBorder:
          solid(m_orgImage, m_destImage, m_solidColor, m_borderMainWidth);
          break;
       
       case NiepceBorder:
          niepce(m_orgImage, m_destImage, m_niepceBorderColor, m_borderMainWidth, 
                 m_niepceLineColor, m_border2ndWidth);
          break;

       case BeveledBorder:
          bevel(m_orgImage, m_destImage, m_bevelUpperLeftColor,
                m_bevelLowerRightColor, m_borderMainWidth);
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
          pattern( m_orgImage, m_destImage, m_borderMainWidth, 
                   m_decorativeFirstColor, m_decorativeSecondColor,
                   m_border2ndWidth, m_border2ndWidth );
          break;
    }
}

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
    kdDebug() << "Border File:" << m_borderPath << endl;
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

}  // NameSpace DigikamBorderImagesPlugin
