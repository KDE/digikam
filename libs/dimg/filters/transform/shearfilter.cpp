/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-18
 * Description : Shear tool threaded image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * Original Shear algorithms copyrighted 2005 by
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
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

#include "shearfilter.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Local includes

#include "digikam_globals.h"
#include "dimg.h"
#include "pixelsaliasfilter.h"

namespace Digikam
{

class ShearFilter::Private
{
public:

    Private()
    {
        antiAlias       = true;
        orgW            = 0;
        orgH            = 0;
        hAngle          = 0;
        vAngle          = 0;
        backgroundColor = Qt::black;
    }

    bool   antiAlias;

    int    orgW;
    int    orgH;

    float  hAngle;
    float  vAngle;

    QColor backgroundColor;

    QSize  newSize;
};

ShearFilter::ShearFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

ShearFilter::ShearFilter(DImg* const orgImage, QObject* const parent, float hAngle, float vAngle,
                         bool antialiasing, const QColor& backgroundColor, int orgW, int orgH)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("sheartool")),
      d(new Private)
{
    d->hAngle          = hAngle;
    d->vAngle          = vAngle;
    d->orgW            = orgW;
    d->orgH            = orgH;
    d->antiAlias       = antialiasing;
    d->backgroundColor = backgroundColor;

    initFilter();
}

ShearFilter::~ShearFilter()
{
    cancelFilter();
}

QSize ShearFilter::getNewSize() const
{
    return d->newSize;
}

void ShearFilter::filterImage()
{
    int          progress;
   int x, y, p = 0, pt;
    int          new_width, new_height;
    double       nx, ny, dx, dy;
    double       horz_factor, vert_factor;
    double       horz_add, vert_add;
    double       horz_beta_angle, vert_beta_angle;

    int nWidth              = m_orgImage.width();
    int nHeight             = m_orgImage.height();
    uchar* pBits            = m_orgImage.bits();
    unsigned short* pBits16 = reinterpret_cast<unsigned short*>(m_orgImage.bits());

    // get beta ( complementary ) angle for horizontal and vertical angles
    horz_beta_angle = (((d->hAngle < 0.0) ? 180.0 : 90.0) - d->hAngle) * DEG2RAD;
    vert_beta_angle = (((d->vAngle < 0.0) ? 180.0 : 90.0) - d->vAngle) * DEG2RAD;

    // get new distance for width and height values
    horz_add = nHeight * ((d->hAngle < 0.0) ? sin(horz_beta_angle) : cos(horz_beta_angle));
    vert_add = nWidth  * ((d->vAngle < 0.0) ? sin(vert_beta_angle) : cos(vert_beta_angle));

    // get absolute values for the distances
    horz_add = fabs(horz_add);
    vert_add = fabs(vert_add);

    // get new image size ( original size + distance )
    new_width  = (int)horz_add + nWidth;
    new_height = (int)vert_add + nHeight;

    // get scale factor for width and height
    horz_factor = horz_add / new_height;
    vert_factor = vert_add / new_width;

    // if horizontal angle is greater than zero...
    // else, initial distance is equal to maximum distance ( in negative form )
    if (d->hAngle > 0.0)
    {
        // initial distance is zero and scale is negative ( to decrease )
        dx = 0;
        horz_factor *= -1.0;
    }
    else
    {
        dx = -horz_add;
    }

    // if vertical angle is greater than zero...
    // else, initial distance is equal to maximum distance ( in negative form )
    if (d->vAngle > 0.0)
    {
        // initial distance is zero and scale is negative ( to decrease )
        dy = 0;
        vert_factor *= -1.0;
    }
    else
    {
        dy = -vert_add;
    }

    // allocates a new image with the new size

    bool sixteenBit = m_orgImage.sixteenBit();
    m_destImage     = DImg(new_width, new_height, sixteenBit, m_orgImage.hasAlpha());
    m_destImage.fill(DColor(d->backgroundColor.rgb(), sixteenBit));

    uchar* pResBits            = m_destImage.bits();
    unsigned short* pResBits16 = reinterpret_cast<unsigned short*>(m_destImage.bits());

    PixelsAliasFilter alias;

    for (y = 0; y < new_height; ++y)
    {
        for (x = 0; x < new_width; ++x, p += 4)
        {
            // get new positions
            nx = x + dx + y * horz_factor;
            ny = y + dy + x * vert_factor;

            // if is inside the source image
            if (isInside(nWidth, nHeight, lround(nx), lround(ny)))
            {
                if (d->antiAlias)
                {
                    if (!sixteenBit)
                        alias.pixelAntiAliasing(pBits, nWidth, nHeight, nx, ny,
                                                &pResBits[p + 3], &pResBits[p + 2],
                                                &pResBits[p + 1], &pResBits[p]);
                    else
                        alias.pixelAntiAliasing16(pBits16, nWidth, nHeight, nx, ny,
                                                  &pResBits16[p + 3], &pResBits16[p + 2],
                                                  &pResBits16[p + 1], &pResBits16[p]);
                }
                else
                {
                    pt = setPosition(nWidth, lround(nx), lround(ny));

                    for (int z = 0 ; z < 4 ; ++z)
                    {
                        if (!sixteenBit)
                        {
                            pResBits[p + z] = pBits[pt + z];
                        }
                        else
                        {
                            pResBits16[p + z] = pBits16[pt + z];
                        }
                    }
                }
            }
        }

        // Update the progress bar in dialog.
        progress = (int)(((double)y * 100.0) / new_height);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }

    // To compute the rotated destination image size using original image dimensions.
    int W = (int)(fabs(d->orgH * ((d->hAngle < 0.0) ? sin(horz_beta_angle) : cos(horz_beta_angle)))) + d->orgW;
    int H = (int)(fabs(d->orgW * ((d->vAngle < 0.0) ? sin(vert_beta_angle) : cos(vert_beta_angle)))) + d->orgH;

    d->newSize.setWidth(W);
    d->newSize.setHeight(H);
}

FilterAction ShearFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("antiAlias"),        d->antiAlias);
    action.addParameter(QLatin1String("hAngle"),           d->hAngle);
    action.addParameter(QLatin1String("orgH"),             d->orgH);
    action.addParameter(QLatin1String("orgW"),             d->orgW);
    action.addParameter(QLatin1String("vAngle"),           d->vAngle);
    action.addParameter(QLatin1String("backgroundColorR"), d->backgroundColor.red());
    action.addParameter(QLatin1String("backgroundColorG"), d->backgroundColor.green());
    action.addParameter(QLatin1String("backgroundColorB"), d->backgroundColor.blue());
    action.addParameter(QLatin1String("backgroundColorA"), d->backgroundColor.alpha());

    return action;
}

void ShearFilter::readParameters(const FilterAction& action)
{
    d->antiAlias = action.parameter(QLatin1String("antiAlias")).toBool();
    d->hAngle = action.parameter(QLatin1String("hAngle")).toFloat();
    d->orgH = action.parameter(QLatin1String("orgH")).toInt();
    d->orgW = action.parameter(QLatin1String("orgW")).toInt();
    d->vAngle = action.parameter(QLatin1String("vAngle")).toFloat();
    d->backgroundColor.setRed(action.parameter(QLatin1String("backgroundColorR")).toInt());
    d->backgroundColor.setGreen(action.parameter(QLatin1String("backgroundColorG")).toInt());
    d->backgroundColor.setBlue(action.parameter(QLatin1String("backgroundColorB")).toInt());
    d->backgroundColor.setAlpha(action.parameter(QLatin1String("backgroundColorA")).toInt());
}

}  // namespace Digikam
