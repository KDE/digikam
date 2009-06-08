/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : border threaded image filter.
 *
 * Copyright 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright 2009      by Andi Clemens <andi dot clemens at gmx dot net>
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


#include "border.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Qt includes

#include <QPoint>
#include <QPolygon>
#include <QRegion>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"

namespace DigikamBorderImagesPlugin
{

class BorderPriv
{
public:

    BorderPriv()
    {
        preserveAspectRatio = false;
        orgWidth              = 0;
        orgHeight             = 0;
        borderType            = 0;
        borderWidth1          = 0;
        borderWidth2          = 0;
        borderWidth3          = 0;
        borderWidth4          = 0;
        borderMainWidth       = 0;
        border2ndWidth        = 0;
        orgRatio              = 0.0f;
    }

    bool            preserveAspectRatio;

    int             orgWidth;
    int             orgHeight;

    int             borderType;

    int             borderWidth1;
    int             borderWidth2;
    int             borderWidth3;
    int             borderWidth4;

    int             borderMainWidth;
    int             border2ndWidth;

    float           orgRatio;

    QString         borderPath;

    Digikam::DColor solidColor;
    Digikam::DColor niepceBorderColor;
    Digikam::DColor niepceLineColor;
    Digikam::DColor bevelUpperLeftColor;
    Digikam::DColor bevelLowerRightColor;
    Digikam::DColor decorativeFirstColor;
    Digikam::DColor decorativeSecondColor;
};

Border::Border(Digikam::DImg *image, QObject *parent, int orgWidth, int orgHeight,
               QString borderPath, int borderType, float borderPercent,
               Digikam::DColor solidColor,
               Digikam::DColor niepceBorderColor,
               Digikam::DColor niepceLineColor,
               Digikam::DColor bevelUpperLeftColor,
               Digikam::DColor bevelLowerRightColor,
               Digikam::DColor decorativeFirstColor,
               Digikam::DColor decorativeSecondColor)
      : Digikam::DImgThreadedFilter(image, parent, "Border"),
        d(new BorderPriv)
{
    d->orgWidth        = orgWidth;
    d->orgHeight       = orgHeight;
    d->orgRatio        = (float)d->orgWidth / (float)d->orgHeight;
    d->borderType      = borderType;
    d->borderPath      = borderPath;
    int size           = (image->width() > image->height()) ? image->height() : image->width();
    d->borderMainWidth = (int)(size * borderPercent);
    d->border2ndWidth  = (int)(size * 0.005);

    // Clamp internal border with to 1 pixel to be visible with small image.
    if (d->border2ndWidth < 1) d->border2ndWidth = 1;

    d->solidColor            = solidColor;
    d->niepceBorderColor     = niepceBorderColor;
    d->niepceLineColor       = niepceLineColor;
    d->bevelUpperLeftColor   = bevelUpperLeftColor;
    d->bevelLowerRightColor  = bevelLowerRightColor;
    d->decorativeFirstColor  = decorativeFirstColor;
    d->decorativeSecondColor = decorativeSecondColor;

    d->preserveAspectRatio   = true;

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
      : Digikam::DImgThreadedFilter(orgImage, parent, "Border"),
        d(new BorderPriv)
{
    d->orgWidth              = orgWidth;
    d->orgHeight             = orgHeight;

    d->borderType            = borderType;
    d->borderWidth1          = borderWidth1;
    d->borderWidth2          = borderWidth2;
    d->borderWidth3          = borderWidth3;
    d->borderWidth4          = borderWidth4;

    d->solidColor            = solidColor;
    d->niepceBorderColor     = niepceBorderColor;
    d->niepceLineColor       = niepceLineColor;
    d->bevelUpperLeftColor   = bevelUpperLeftColor;
    d->bevelLowerRightColor  = bevelLowerRightColor;
    d->decorativeFirstColor  = decorativeFirstColor;
    d->decorativeSecondColor = decorativeSecondColor;

    d->borderPath            = borderPath;

    d->preserveAspectRatio   = false;

    initFilter();
}

Border::~Border()
{
    delete d;
}

void Border::filterImage(void)
{
    switch (d->borderType)
    {
        case SolidBorder:
            if (d->preserveAspectRatio)
                solid(m_orgImage, m_destImage, d->solidColor, d->borderMainWidth);
            else
                solid2(m_orgImage, m_destImage, d->solidColor, d->borderWidth1);
            break;

        case NiepceBorder:
            if (d->preserveAspectRatio)
                niepce(m_orgImage, m_destImage, d->niepceBorderColor, d->borderMainWidth,
                       d->niepceLineColor, d->border2ndWidth);
            else
                niepce2(m_orgImage, m_destImage, d->niepceBorderColor, d->borderWidth1,
                        d->niepceLineColor, d->borderWidth4);
            break;

        case BeveledBorder:
            if (d->preserveAspectRatio)
                bevel(m_orgImage, m_destImage, d->bevelUpperLeftColor,
                      d->bevelLowerRightColor, d->borderMainWidth);
            else
                bevel2(m_orgImage, m_destImage, d->bevelUpperLeftColor,
                       d->bevelLowerRightColor, d->borderWidth1);
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
            if (d->preserveAspectRatio)
                pattern(m_orgImage, m_destImage, d->borderMainWidth,
                        d->decorativeFirstColor, d->decorativeSecondColor,
                        d->border2ndWidth, d->border2ndWidth);
            else
                pattern2(m_orgImage, m_destImage, d->borderWidth1,
                         d->decorativeFirstColor, d->decorativeSecondColor,
                         d->borderWidth2, d->borderWidth2);
            break;
    }
}

// -- Methods to preserve aspect ratio of image ------------------------------------------

void Border::solid(Digikam::DImg& src, Digikam::DImg& dest, const Digikam::DColor& fg, int borderWidth)
{
    if (d->orgWidth > d->orgHeight)
    {
        int height = src.height() + borderWidth*2;
        dest = Digikam::DImg((int)(height*d->orgRatio), height, src.sixteenBit(), src.hasAlpha());
        dest.fill(fg);
        dest.bitBltImage(&src, (dest.width()-src.width())/2, borderWidth);
    }
    else
    {
        int width = src.width() + borderWidth*2;
        dest = Digikam::DImg(width, (int)(width/d->orgRatio), src.sixteenBit(), src.hasAlpha());
        dest.fill(fg);
        dest.bitBltImage(&src, borderWidth, (dest.height()-src.height())/2);
    }
}

void Border::niepce(Digikam::DImg& src, Digikam::DImg& dest, const Digikam::DColor& fg,
                    int borderWidth, const Digikam::DColor& bg, int lineWidth)
{
    Digikam::DImg tmp;
    solid(src, tmp, bg, lineWidth);
    solid(tmp, dest, fg, borderWidth);
}

void Border::bevel(Digikam::DImg& src, Digikam::DImg& dest, const Digikam::DColor& topColor,
                   const Digikam::DColor& btmColor, int borderWidth)
{
    int width, height;

    if (d->orgWidth > d->orgHeight)
    {
        height = src.height() + borderWidth*2;
        width  = (int)(height*d->orgRatio);
    }
    else
    {
        width  = src.width() + borderWidth*2;
        height = (int)(width/d->orgRatio);
    }

    dest = Digikam::DImg(width, height, src.sixteenBit(), src.hasAlpha());
    dest.fill(topColor);

    QPolygon btTriangle(3);
    btTriangle.setPoint(0, width, 0);
    btTriangle.setPoint(1, 0,     height);
    btTriangle.setPoint(2, width, height);
    QRegion btRegion(btTriangle);

    // paint upper right corner
    QPoint upperRightCorner((width - ((width - src.width()) / 2) - 2),
                            ((0 + (height - src.height())) / 2 + 2)
    );

    for (int x = upperRightCorner.x(); x < width; ++x)
    {
        for (int y = 0; y < upperRightCorner.y(); ++y)
        {
            if (btRegion.contains(QPoint(x, y)))
                dest.setPixelColor(x, y, btmColor);
        }
    }

    // paint right border
    for (int x = upperRightCorner.x(); x < width; ++x)
    {
        for (int y = upperRightCorner.y(); y < height; ++y)
        {
            dest.setPixelColor(x, y, btmColor);
        }
    }

    // paint lower left corner
    QPoint lowerLeftCorner((0 + ((width - src.width()) / 2) + 2),
                           (height - ((height - src.height()) / 2) - 2)
    );

    for (int x = 0; x < lowerLeftCorner.x(); ++x)
    {
        for (int y = lowerLeftCorner.y(); y < height; ++y)
        {
            if (btRegion.contains(QPoint(x, y)))
                dest.setPixelColor(x, y, btmColor);
        }
    }

    // paint bottom border
    for (int x = lowerLeftCorner.x(); x < width; ++x)
    {
        for (int y = lowerLeftCorner.y(); y < height; ++y)
        {
            dest.setPixelColor(x, y, btmColor);
        }
    }

    if (d->orgWidth > d->orgHeight)
        dest.bitBltImage(&src, (dest.width() - src.width()) / 2, borderWidth);
    else
        dest.bitBltImage(&src, borderWidth, (dest.height() - src.height()) / 2);
}

void Border::pattern(Digikam::DImg& src, Digikam::DImg& dest, int borderWidth,
                     const Digikam::DColor& firstColor, const Digikam::DColor& secondColor,
                     int firstWidth, int secondWidth)
{
    // Original image with the first solid border around.
    Digikam::DImg tmp;
    solid(src, tmp, firstColor, firstWidth);

    // Border tiled image using pattern with second solid border around.
    int width, height;

    if (d->orgWidth > d->orgHeight)
    {
        height = tmp.height() + borderWidth*2;
        width  = (int)(height*d->orgRatio);
    }
    else
    {
        width  = tmp.width() + borderWidth*2;
        height = (int)(width/d->orgRatio);
    }

    Digikam::DImg tmp2(width, height, tmp.sixteenBit(), tmp.hasAlpha());
    kDebug(50006) << "Border File:" << d->borderPath << endl;
    Digikam::DImg border(d->borderPath);
    if ( border.isNull() )
        return;

    border.convertToDepthOfImage(&tmp2);

    for (int x = 0 ; x < width ; x+=border.width())
    {
        for (int y = 0 ; y < height ; y+=border.height())
        {
            tmp2.bitBltImage(&border, x, y);
        }
    }

    solid(tmp2, dest, secondColor, secondWidth);

    // Merge both images to one.
    if (d->orgWidth > d->orgHeight)
    {
        dest.bitBltImage(&tmp, (dest.width()-tmp.width())/2, borderWidth);
    }
    else
    {
        dest.bitBltImage(&tmp, borderWidth, (dest.height()-tmp.height())/2);
    }
}

// -- Methods to not-preserve aspect ratio of image ------------------------------------------


void Border::solid2(Digikam::DImg& src, Digikam::DImg& dest, const Digikam::DColor& fg, int borderWidth)
{
    dest = Digikam::DImg(src.width() + borderWidth*2, src.height() + borderWidth*2,
                         src.sixteenBit(), src.hasAlpha());
    dest.fill(fg);
    dest.bitBltImage(&src, borderWidth, borderWidth);
}

void Border::niepce2(Digikam::DImg& src, Digikam::DImg& dest, const Digikam::DColor& fg, int borderWidth,
                     const Digikam::DColor& bg, int lineWidth)
{
    Digikam::DImg tmp;
    solid2(src, tmp, bg, lineWidth);
    solid2(tmp, dest, fg, borderWidth);
}

void Border::bevel2(Digikam::DImg& src, Digikam::DImg& dest, const Digikam::DColor& topColor,
                    const Digikam::DColor& btmColor, int borderWidth)
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

void Border::pattern2(Digikam::DImg& src, Digikam::DImg& dest, int borderWidth,
                      const Digikam::DColor& firstColor, const Digikam::DColor& secondColor,
                      int firstWidth, int secondWidth)
{
    // Border tile.

    int w = d->orgWidth + borderWidth*2;
    int h = d->orgHeight + borderWidth*2;

    kDebug(50006) << "Border File:" << d->borderPath << endl;
    Digikam::DImg border(d->borderPath);
    if ( border.isNull() )
        return;

    Digikam::DImg borderImg(w, h, src.sixteenBit(), src.hasAlpha());
    border.convertToDepthOfImage(&borderImg);

    for (int x = 0 ; x < w ; x+=border.width())
    {
        for (int y = 0 ; y < h ; y+=border.height())
        {
            borderImg.bitBltImage(&border, x, y);
        }
    }

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

}  // namespace DigikamBorderImagesPlugin
