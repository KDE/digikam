/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-18
 * Description : Shear tool threaded image filter.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

// Degrees to radian conversion coeff (PI/180). To optimize computation.
#define DEG2RAD 0.017453292519943

#include "shearfilter.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Local includes

#include "dimg.h"
#include "pixelsaliasfilter.h"

namespace Digikam
{

ShearFilter::ShearFilter(QObject* parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

ShearFilter::ShearFilter(DImg* orgImage, QObject* parent, float hAngle, float vAngle,
                         bool antialiasing, const QColor& backgroundColor, int orgW, int orgH)
    : DImgThreadedFilter(orgImage, parent, "sheartool")
{
    m_hAngle          = hAngle;
    m_vAngle          = vAngle;
    m_orgW            = orgW;
    m_orgH            = orgH;
    m_antiAlias       = antialiasing;
    m_backgroundColor = backgroundColor;

    initFilter();
}

ShearFilter::~ShearFilter()
{
    cancelFilter();
}

void ShearFilter::filterImage()
{
    int          progress;
    register int x, y, p = 0, pt;
    int          new_width, new_height;
    double       nx, ny, dx, dy;
    double       horz_factor, vert_factor;
    double       horz_add, vert_add;
    double       horz_beta_angle, vert_beta_angle;

    int nWidth  = m_orgImage.width();
    int nHeight = m_orgImage.height();

    uchar* pBits            = m_orgImage.bits();
    unsigned short* pBits16 = (unsigned short*)m_orgImage.bits();

    // get beta ( complementary ) angle for horizontal and vertical angles
    horz_beta_angle = ( ( ( m_hAngle < 0.0 ) ? 180.0 : 90.0 ) - m_hAngle ) * DEG2RAD;
    vert_beta_angle = ( ( ( m_vAngle < 0.0 ) ? 180.0 : 90.0 ) - m_vAngle ) * DEG2RAD;

    // get new distance for width and height values
    horz_add = nHeight * ( ( m_hAngle < 0.0 ) ? sin( horz_beta_angle ) : cos( horz_beta_angle ) );
    vert_add = nWidth  * ( ( m_vAngle < 0.0 ) ? sin( vert_beta_angle ) : cos( vert_beta_angle ) );

    // get absolute values for the distances
    horz_add = fabs( horz_add );
    vert_add = fabs( vert_add );

    // get new image size ( original size + distance )
    new_width  = (int)horz_add + nWidth;
    new_height = (int)vert_add + nHeight;

    // get scale factor for width and height
    horz_factor = horz_add / new_height;
    vert_factor = vert_add / new_width;

    // if horizontal angle is greater than zero...
    // else, initial distance is equal to maximum distance ( in negative form )
    if ( m_hAngle > 0.0 )
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
    if ( m_vAngle > 0.0 )
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
    m_destImage.fill( DColor(m_backgroundColor.rgb(), sixteenBit) );

    uchar* pResBits            = m_destImage.bits();
    unsigned short* pResBits16 = (unsigned short*)m_destImage.bits();

    PixelsAliasFilter alias;

    for ( y = 0; y < new_height; ++y)
    {
        for ( x = 0; x < new_width; ++x, p += 4 )
        {
            // get new positions
            nx = x + dx + y * horz_factor;
            ny = y + dy + x * vert_factor;

            // if is inside the source image
            if (isInside (nWidth, nHeight, lround( nx ), lround( ny )))
            {
                if ( m_antiAlias )
                {
                    if (!sixteenBit)
                        alias.pixelAntiAliasing(pBits, nWidth, nHeight, nx, ny,
                                                &pResBits[p+3], &pResBits[p+2],
                                                &pResBits[p+1], &pResBits[p]);
                    else
                        alias.pixelAntiAliasing16(pBits16, nWidth, nHeight, nx, ny,
                                                  &pResBits16[p+3], &pResBits16[p+2],
                                                  &pResBits16[p+1], &pResBits16[p]);
                }
                else
                {
                    pt = setPosition (nWidth, lround( nx ), lround( ny ));

                    for (int z = 0 ; z < 4 ; ++z)
                    {
                        if (!sixteenBit)
                        {
                            pResBits[p+z] = pBits[pt+z];
                        }
                        else
                        {
                            pResBits16[p+z] = pBits16[pt+z];
                        }
                    }
                }
            }
        }

        // Update the progress bar in dialog.
        progress = (int)(((double)y * 100.0) / new_height);

        if (progress%5 == 0)
        {
            postProgress( progress );
        }
    }

    // To compute the rotated destination image size using original image dimensions.
    int W = (int)(fabs(m_orgH * ( ( m_hAngle < 0.0 ) ? sin( horz_beta_angle ) : cos( horz_beta_angle )))) + m_orgW;
    int H = (int)(fabs(m_orgW * ( ( m_vAngle < 0.0 ) ? sin( vert_beta_angle ) : cos( vert_beta_angle )))) + m_orgH;

    m_newSize.setWidth(W);
    m_newSize.setHeight(H);
}

FilterAction ShearFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter("antiAlias", m_antiAlias);
    action.addParameter("hAngle", m_hAngle);
    action.addParameter("orgH", m_orgH);
    action.addParameter("orgW", m_orgW);
    action.addParameter("vAngle", m_vAngle);
    action.addParameter("backgroundColorR", m_backgroundColor.red());
    action.addParameter("backgroundColorG", m_backgroundColor.green());
    action.addParameter("backgroundColorB", m_backgroundColor.blue());
    action.addParameter("backgroundColorA", m_backgroundColor.alpha());

    return action;
}

void ShearFilter::readParameters(const FilterAction& action)
{
    m_antiAlias = action.parameter("antiAlias").toBool();
    m_hAngle = action.parameter("hAngle").toFloat();
    m_orgH = action.parameter("orgH").toInt();
    m_orgW = action.parameter("orgW").toInt();
    m_vAngle = action.parameter("vAngle").toFloat();
    m_backgroundColor.setRed(action.parameter("backgroundColorR").toInt());
    m_backgroundColor.setGreen(action.parameter("backgroundColorG").toInt());
    m_backgroundColor.setBlue(action.parameter("backgroundColorB").toInt());
    m_backgroundColor.setAlpha(action.parameter("backgroundColorA").toInt());
}


}  // namespace Digikam
