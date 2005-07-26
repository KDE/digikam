/* ============================================================
 * File  : sheartool.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-18
 * Description : Shear tool threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Original Shear algorithms copyrighted 2005 by 
 * Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
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
 
// Degrees to radian convertion coeff (PI/180). To optimize computation.
#define DEG2RAD 0.017453292519943
 
// C++ includes. 
 
#include <cmath>
#include <cstdlib>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "sheartool.h"

namespace DigikamShearToolImagesPlugin
{

ShearTool::ShearTool(QImage *orgImage, QObject *parent, float hAngle, float vAngle,
                     bool antialiasing, QColor backgroundColor, int orgW, int orgH)
            : Digikam::ThreadedFilter(orgImage, parent, "ShearTool")
{ 
    m_hAngle    = hAngle;
    m_vAngle    = vAngle;
    m_orgW      = orgW;
    m_orgH      = orgH;
    m_antiAlias = antialiasing;
    m_backgroundColor = backgroundColor;
            
    initFilter();
}

void ShearTool::filterImage(void)
{
    register int x, y, p = 0, pt;
    int          new_width, new_height;
    double       nx, ny, dx, dy;
    double       horz_factor, vert_factor;
    double       horz_add, vert_add;
    double       horz_beta_angle, vert_beta_angle;

    int nWidth  = m_orgImage.width();
    int nHeight = m_orgImage.height();
    
    uchar *pBits = m_orgImage.bits();        
    
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
    if( m_hAngle > 0.0 ) 
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
    if( m_vAngle > 0.0 ) 
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
    m_destImage.create(new_width, new_height, 32);
    m_destImage.fill( m_backgroundColor.rgb() );
    uchar *pResBits = m_destImage.bits();
    
    for( y = 0; y < new_height; y++) 
        {
        for( x = 0; x < new_width; x++, p += 4 ) 
            {
            // get new positions
            nx = x + dx + y * horz_factor;
            ny = y + dy + x * vert_factor;

            // if is inside the source image
            if (isInside (nWidth, nHeight, ROUND( nx ), ROUND( ny )))            
                {
                if( m_antiAlias ) 
                    {
                    // get anti aliased pixel
                    Digikam::ImageFilters::pixelAntiAliasing(pBits, nWidth, nHeight, nx, ny, 
                             &pResBits[p+3], &pResBits[p+2], &pResBits[p+1], &pResBits[p]);
                    }
                else 
                    {
                    // else, get exact pixel
                    pt = setPosition (nWidth, ROUND( nx ), ROUND( ny ));

                    pResBits[p+3] = pBits[pt+3];
                    pResBits[p+2] = pBits[pt+2];
                    pResBits[p+1] = pBits[pt+1];
                    pResBits[ p ] = pBits[ pt ];
                    }
                }
            }
        }
        
    // To compute the rotated destination image size using original image dimensions.           
    int W = (int)(fabs(m_orgH * ( ( m_hAngle < 0.0 ) ? sin( horz_beta_angle ) : cos( horz_beta_angle ))))+
            m_orgW;
    int H = (int)(fabs(m_orgW * ( ( m_vAngle < 0.0 ) ? sin( vert_beta_angle ) : cos( vert_beta_angle ))))+
            m_orgH;
    
    m_newSize.setWidth(W);
    m_newSize.setHeight(H);
}

}  // NameSpace DigikamShearToolImagesPlugin
