/* ============================================================
 * File  : pixelaccess.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-27
 * Description : 
 *
 * Copyright 2004-2005 by Gilles Caulier
 *
 * Original Distortion Correction algorithm copyrighted 
 * 2001-2003 David Hodson <hodsond@acm.org>
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

// C++ include.

#include <cstring>
#include <cmath>
#include <cstdlib>
 
// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "pixelaccess.h"

namespace DigikamLensDistortionImagesPlugin
{

PixelAccess::PixelAccess(uint *data, int Width, int Height)
{
    m_width       = PixelAccessWidth;
    m_height      = PixelAccessHeight;
    m_depth       = 4;
    m_imageWidth  = Width;
    m_imageHeight = Height;
    m_srcPR       = data;
     
    m_image.create( m_imageWidth, m_imageHeight, 32 );
    memcpy(m_image.bits(), m_srcPR, m_image.numBytes());
    
    for ( int i = 0 ; i < PixelAccessRegions ; i++ ) 
       {
       m_buffer[i] = new uchar[m_height * m_width * m_depth];
       
       m_region = m_image.copy(0, 0, m_width, m_height);
       memcpy(m_buffer[i], m_region.bits(), m_region.numBytes());
       
       m_tileMinX[i] = 1;
       m_tileMaxX[i] = m_width - 2;
       m_tileMinY[i] = 1;
       m_tileMaxY[i] = m_height - 2;
       }
}

PixelAccess::~PixelAccess()
{
    for( int i = 0 ; i < PixelAccessRegions ; i++ ) 
       delete [] m_buffer[i];
}

uchar* PixelAccess::pixelAccessAddress(int i, int j)
{
    return m_buffer[0] + m_depth * (m_width * (j + 1 - m_tileMinY[0]) + (i + 1 - m_tileMinX[0]));
}

// Swap region[n] with region[0].
void PixelAccess::pixelAccessSelectRegion(int n)
{
    uchar* temp;
    int    a, b, c, d;
    int    i;

    temp = m_buffer[n];
    a    = m_tileMinX[n];
    b    = m_tileMaxX[n];
    c    = m_tileMinY[n];
    d    = m_tileMaxY[n];

    for( i = n ; i > 0 ; i--) 
       {
       m_buffer[i]   = m_buffer[i-1];
       m_tileMinX[i] = m_tileMinX[i-1];
       m_tileMaxX[i] = m_tileMaxX[i-1];
       m_tileMinY[i] = m_tileMinY[i-1];
       m_tileMaxY[i] = m_tileMaxY[i-1];
       }

    m_buffer[0]   = temp;
    m_tileMinX[0] = a;
    m_tileMaxX[0] = b;
    m_tileMinY[0] = c;
    m_tileMaxY[0] = d;
}

// Buffer[0] is cleared, should start at [i, j], fill rows that overlap image.
void PixelAccess::pixelAccessDoEdge(int i, int j)
{
    int    lineStart, lineEnd;
    int    rowStart, rowEnd;
    int    lineWidth;
    uchar* line;

    lineStart = i;
    if (lineStart < 0) lineStart = 0;
    lineEnd = i + m_width;
    if (lineEnd > m_imageWidth) lineEnd = m_imageWidth;
    lineWidth = lineEnd - lineStart;

    if( lineStart >= lineEnd ) 
       return;

    rowStart = j;
    if (rowStart < 0) rowStart = 0;
    rowEnd = j + m_height;
    if (rowEnd > m_imageHeight) rowEnd = m_imageHeight;

    for( int y = rowStart ; y < rowEnd ; y++ ) 
       {
       line = pixelAccessAddress(lineStart, y);

       m_region = m_image.copy(lineStart, y, lineWidth, 1);
       memcpy(line, m_region.bits(), m_region.numBytes());
       }
}

// Moves buffer[0] so that [x, y] is inside it.
void PixelAccess::pixelAccessReposition(int xInt, int yInt)
{
    int newStartX = xInt - PixelAccessXOffset;
    int newStartY = yInt - PixelAccessYOffset;

    m_tileMinX[0] = newStartX + 1;
    m_tileMaxX[0] = newStartX + m_width - 2;
    m_tileMinY[0] = newStartY + 1;
    m_tileMaxY[0] = newStartY + m_height - 2;

    if ( (newStartX < 0) || ((newStartX + m_width) >= m_imageWidth) ||
         (newStartY < 0) || ((newStartY + m_height) >= m_imageHeight) ) 
       {
       // some data is off edge of image 

       memset(m_buffer[0], 0, m_width * m_height * m_depth);

       if ( ((newStartX + m_width) < 0) || (newStartX >= m_imageWidth) ||
            ((newStartY + m_height) < 0) || (newStartY >= m_imageHeight) ) 
          {
          // totally outside, just leave it. 
          } 
       else 
          {
          pixelAccessDoEdge(newStartX, newStartY);
          }
       } 
    else 
       {
       m_region = m_image.copy(newStartX, newStartY, m_width, m_height);
       memcpy(m_buffer[0], m_region.bits(), m_region.numBytes());
       }
}

void PixelAccess::pixelAccessGetCubic(double srcX, double srcY, double brighten, uchar* dst, int dstDepth)
{
    int     xInt, yInt;
    double  dx, dy;
    uchar  *corner;

    xInt = (int)floor(srcX);
    dx   = srcX - xInt;
    yInt = (int)floor(srcY);
    dy   = srcY - yInt;

    // We need 4x4 pixels, xInt-1 to xInt+2 horz, yInt-1 to yInt+2 vert 
    // they're probably in the last place we looked... 
    
    if ((xInt >= m_tileMinX[0]) && (xInt < m_tileMaxX[0]) &&
        (yInt >= m_tileMinY[0]) && (yInt < m_tileMaxY[0]) ) 
      {
      corner = pixelAccessAddress(xInt - 1, yInt - 1);
      cubicInterpolate(corner, m_depth * m_width, m_depth, dst, dstDepth, dx, dy, brighten);
      return;
      }

    // Or maybe it was a while back... 
    
    for ( int i = 1 ; i < PixelAccessRegions ; i++) 
       {
       if ((xInt >= m_tileMinX[i]) && (xInt < m_tileMaxX[i]) &&
           (yInt >= m_tileMinY[i]) && (yInt < m_tileMaxY[i]) ) 
          {
          // Check here first next time 
          
          pixelAccessSelectRegion(i);
          corner = pixelAccessAddress(xInt - 1, yInt - 1);
          cubicInterpolate(corner, m_depth * m_width, m_depth, dst, dstDepth, dx, dy, brighten);
          return;
          }
       }

    // Nope, recycle an old region.
    
    pixelAccessSelectRegion(PixelAccessRegions - 1);
    pixelAccessReposition(xInt, yInt);
    
    corner = pixelAccessAddress(xInt - 1, yInt - 1);
    cubicInterpolate(corner, m_depth * m_width, m_depth, dst, dstDepth, dx, dy, brighten);
}

/*
 * Catmull-Rom cubic interpolation
 *
 * equally spaced points p0, p1, p2, p3
 * interpolate 0 <= u < 1 between p1 and p2
 *
 * (1 u u^2 u^3) (  0.0  1.0  0.0  0.0 ) (p0)
 *               ( -0.5  0.0  0.5  0.0 ) (p1)
 *               (  1.0 -2.5  2.0 -0.5 ) (p2)
 *               ( -0.5  1.5 -1.5  0.5 ) (p3)
 *
 */
void PixelAccess::cubicInterpolate(uchar* src, int rowStride, int srcDepth, uchar* dst, 
                                   int dstDepth, double dx, double dy, double brighten)
{
    float um1, u, up1, up2;
    float vm1, v, vp1, vp2;
    int   c;
    float verts[4 * MAX_PIXEL_DEPTH];

    um1 = ((-0.5 * dx + 1.0) * dx - 0.5) * dx;
    u   = (1.5 * dx - 2.5) * dx * dx + 1.0;
    up1 = ((-1.5 * dx + 2.0) * dx + 0.5) * dx;
    up2 = (0.5 * dx - 0.5) * dx * dx;

    vm1 = ((-0.5 * dy + 1.0) * dy - 0.5) * dy;
    v   = (1.5 * dy - 2.5) * dy * dy + 1.0;
    vp1 = ((-1.5 * dy + 2.0) * dy + 0.5) * dy;
    vp2 = (0.5 * dy - 0.5) * dy * dy;

    // Note: if dstDepth < srcDepth, we calculate unneeded pixels here 
    // later - select or create index array.
  
    for (c = 0 ; c < 4 * srcDepth ; c++) 
       {
       verts[c] = vm1 * src[c] + v * src[c+rowStride] + vp1 * src[c+rowStride*2] + vp2 * src[c+rowStride*3];
       }
  
    for (c = 0 ; c < dstDepth ; c++) 
       {
       float result;
       result = um1 * verts[c] + u * verts[c+srcDepth] + up1 * verts[c+srcDepth*2] + up2 * verts[c+srcDepth*3];
       result *= brighten;
       
       if (result < 0.0) 
          {
          dst[c] = 0;
          }
       else if (result > 255.0) 
          {
          dst[c] = 255;
          } 
       else 
          {
          dst[c] = (uint)result;
          }
       }
}

}  // NameSpace DigikamLensDistortionImagesPlugin

