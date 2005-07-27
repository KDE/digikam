/* ============================================================
 * File  : hotpixelfixer.h
 * Author: Unai Garro <ugarro at users dot sourceforge dot net>
 * Date  : 2005-03-27
 * Description : Threaded image filter to fix hot pixels
 * 
 * Copyright 2005 by Unai Garro
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
 * ============================================================ 
 * The algorithm for fixing the hot pixels was based on
 * the code of jpegpixi, which was released under the GPL license,
 * and is Copyright (C) 2003, 2004 Martin Dickopp
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

enum InterpolationMethod 
{
    average   = 0,
    linear    = 1,
    quadratic = 2,
    cubic     = 3
};

enum Direction
{
    twodim     = 0,
    vertical   = 1,
    horizontal = 2    
};



class HotPixelFixer : public Digikam::ThreadedFilter
{

public:
        
    HotPixelFixer(QImage *orgImage, QObject *parent, 
                  const QValueList<HotPixel>& hpList, InterpolationMethod method);
    ~HotPixelFixer();

private: // Hot Pixels Removal filter data.

    InterpolationMethod  m_method;
       
    QValueList<HotPixel> m_hpList;
         
private: // Hot Pixels Removal filter methods.

    virtual void filterImage(void);
    
    void interpolate (QImage &img,HotPixel &hp,InterpolationMethod method);
    void weightPixels (QImage &img, HotPixel &px, InterpolationMethod method, Direction dir);
    
    inline bool validPoint(QImage &img, QPoint p)
        {
        return (p.x()>=0 && p.y()>=0 && p.x()<img.width() && p.y()<img.height());
        };
    
    QValueList <Weights> mWeightList;
       
};

}  // NameSpace DigikamHotPixelsImagesPlugin

#endif  // HOTPIXELFIXER_H
