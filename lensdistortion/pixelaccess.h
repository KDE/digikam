/* ============================================================
 * File  : pixelaccess.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-27
 * Description : 
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

#ifndef PIXEL_ACCESS_H
#define PIXEL_ACCESS_H

#define PixelAccessRegions 20
#define PixelAccessWidth   40
#define PixelAccessHeight  20
#define PixelAccessXOffset 3
#define PixelAccessYOffset 3

#define MAX_PIXEL_DEPTH    4

// Qt includes.

#include <qimage.h>

namespace DigikamLensDistortionImagesPlugin
{

    /* PixelAcess class: solving the eternal problem: random, cubic-interpolated,
     * sub-pixel coordinate access to an image.
     * Assuming that accesses are at least slightly coherent, 
     * PixelAccess keeps PixelAccessRegions buffers, each containing a
     * PixelAccessWidth x PixelAccessHeight region of pixels.
     * Buffer[0] is always checked first, so move the last accessed
     * region into that position.
     * When a request arrives which is outside all the regions,
     * get a new region.
     * The new region is placed so that the requested pixel is positioned
     * at [PixelAccessXOffset, PixelAccessYOffset] in the region.
     */

class PixelAccess
{
public:
    
    PixelAccess(uint *data, int Width, int Height);
    ~PixelAccess();

    void pixelAccessGetCubic(double srcX, double srcY, double brighten, uchar* dst, int dstDepth);
    
private:   
    
    QImage m_image;
    QImage m_region;                 
    
    uint*  m_srcPR;
    uchar* m_buffer[PixelAccessRegions];
    
    int    m_width;
    int    m_height;
    int    m_depth;
    int    m_imageWidth;
    int    m_imageHeight;
    int    m_tileMinX[PixelAccessRegions];
    int    m_tileMaxX[PixelAccessRegions];
    int    m_tileMinY[PixelAccessRegions];
    int    m_tileMaxY[PixelAccessRegions];
    
protected:

    inline uchar* pixelAccessAddress(int i, int j);
    void pixelAccessSelectRegion(int n);
    void pixelAccessDoEdge(int i, int j);
    void pixelAccessReposition(int xInt, int yInt);
    void cubicInterpolate(uchar* src, int rowStride, int srcDepth, uchar* dst, 
                          int dstDepth, double dx, double dy, double brighten);
};

}  // NameSpace DigikamLensDistortionImagesPlugin

#endif /* PIXEL_ACCESS_H */
