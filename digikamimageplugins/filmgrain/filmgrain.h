/* ============================================================
 * File  : filmgrain.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Filmgrain threaded image filter.
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
  
#ifndef FILMGRAIN_H
#define FILMGRAIN_H

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamFilmGrainImagesPlugin
{

class FilmGrain : public Digikam::ThreadedFilter
{

public:
    
    FilmGrain(QImage *orgImage, QObject *parent=0, int sensibility=12);
    
    ~FilmGrain(){};
    
private:  // FilmGrain filter data.

    int  m_sensibility;
    
private:  // FilmGrain filter methods.

    virtual void filterImage(void);

    void filmgrainImage(uint* data, int Width, int Height, int Sensibility);

    // A color is represented in RGB value (e.g. 0xFFFFFF is white color). 
    // But R, G and B values has 256 values to be used so, this function analize 
    // the value and limits to this range.
    inline uchar LimitValues (int ColorValue)
       {
       if (ColorValue > 255) ColorValue = 255;        
       if (ColorValue < 0) ColorValue = 0;
       return ((uchar) ColorValue);
       };
    
    inline int GetStride (int Width)
       { 
       int LineWidth = Width * 4;
       if (LineWidth % 4) return (4 - (LineWidth % 4)); 
       return (0); 
       };
           
};    

}  // NameSpace DigikamFilmGrainImagesPlugin

#endif /* FILMGRAIN_H */
