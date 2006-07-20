/* ============================================================
 * File  : oilpaint.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : OilPainting threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
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
  
#ifndef OILPAINT_H
#define OILPAINT_H

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamOilPaintImagesPlugin
{

class OilPaint : public Digikam::ThreadedFilter
{

public:
    
    OilPaint(QImage *orgImage, QObject *parent=0, int brushSize=1, int smoothness=30);
    
    ~OilPaint(){};
    
private:  // OilPaint filter data.

    int m_brushSize;
    int m_smoothness;
    
private:  // OilPaint filter methods.

    virtual void filterImage(void);

    void oilpaintImage(uint* data, int w, int h, int BrushSize, int Smoothness);

    uint MostFrequentColor (uint* Bits, int Width, int Height, int X, 
                            int Y, int Radius, int Intensity);
    
    // Function to calcule the color intensity and return the luminance (Y)
    // component of YIQ color model.
    inline uint GetIntensity(uint Red, uint Green, uint Blue)
           { return ((uint)(Red * 0.3 + Green * 0.59 + Blue * 0.11)); };
};    

}  // NameSpace DigikamOilPaintImagesPlugin

#endif /* OILPAINT_H */
