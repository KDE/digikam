/* ============================================================
 * Authors: Unai Garro <ugarro at users dot sourceforge dot net>
 *          Gilles Caulier <caulier dot gilles at free dot fr>
 * Date   : 2005-03-27
 * Description : Threaded image filter to fix hot pixels
 * 
 * Copyright 2005-2007 by Unai Garro and Gilles Caulier
 *
 * The algorithm for fixing the hot pixels was based on
 * the code of jpegpixi, which was released under the GPL license,
 * and is Copyright (C) 2003, 2004 Martin Dickopp
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
 * ============================================================*/

#ifndef HOTPIXELFIXER_H
#define HOTPIXELFIXER_H

// Qt includes.

#include <qimage.h>
#include <qobject.h>
#include <qvaluelist.h>
#include <qrect.h>
#include <qstring.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "hotpixel.h"
#include "weights.h"

namespace DigikamHotPixelsImagesPlugin
{

class HotPixelFixer : public Digikam::DImgThreadedFilter
{

public:
    
    enum InterpolationMethod 
    {
        AVERAGE_INTERPOLATION   = 0,
        LINEAR_INTERPOLATION    = 1,
        QUADRATIC_INTERPOLATION = 2,
        CUBIC_INTERPOLATION     = 3
    };
    
    enum Direction
    {
        TWODIM_DIRECTION        = 0,
        VERTICAL_DIRECTION      = 1,
        HORIZONTAL_DIRECTION    = 2    
    };

public:
        
    HotPixelFixer(Digikam::DImg *orgImage, QObject *parent, 
                  const QValueList<HotPixel>& hpList, int interpolationMethod);
    ~HotPixelFixer();

private: 

    virtual void filterImage(void);
    
    void interpolate (Digikam::DImg &img,HotPixel &hp, int method);
    void weightPixels (Digikam::DImg &img, HotPixel &px, int method, Direction dir, int maxComponent);
    
    inline bool validPoint(Digikam::DImg &img, QPoint p)
    {
        return (p.x()>=0 && p.y()>=0 && p.x()<(long) img.width() && p.y()<(long) img.height());
    };
    
    QValueList <Weights> mWeightList;

private: 

    int                  m_interpolationMethod;
       
    QValueList<HotPixel> m_hpList;
};

}  // NameSpace DigikamHotPixelsImagesPlugin

#endif  // HOTPIXELFIXER_H
