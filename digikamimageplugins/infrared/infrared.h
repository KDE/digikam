/* ============================================================
 * File  : infrared.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Infrared threaded image filter.
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
  
#ifndef INFRARED_H
#define INFRARED_H

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamInfraredImagesPlugin
{

class Infrared : public Digikam::ThreadedFilter
{

public:
    
    Infrared(QImage *orgImage, QObject *parent=0, int sensibility=1, bool grain=true);
    
    ~Infrared(){};
    
private:  // Infrared filter data.

    bool m_grain;
    
    int  m_sensibility;
    
private:  // Infrared filter methods.

    virtual void filterImage(void);

    void infraredImage(uint* data, int Width, int Height, int Sensibility, bool Grain);

    // A color is represented in RGB value (e.g. 0xFFFFFF is white color). 
    // But R, G and B values has 256 values to be used so, this function analize 
    // the value and limits to this range.
    inline uchar LimitValues (int ColorValue)
       {
       if (ColorValue > 255) ColorValue = 255;        
       if (ColorValue < 0) ColorValue = 0;
       return ((uchar) ColorValue);
       };
    
};    

}  // NameSpace DigikamInfraredImagesPlugin

#endif /* INFRARED_H */
