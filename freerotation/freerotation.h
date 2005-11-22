/* ============================================================
 * File  : freerotation.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-18
 * Description : Free rotation threaded image filter.
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
  
#ifndef FREE_ROTATION_H
#define FREE_ROTATION_H

// Qt includes.

#include <qsize.h>
#include <qcolor.h>

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamFreeRotationImagesPlugin
{

class FreeRotation : public Digikam::ThreadedFilter
{

public:
    
    FreeRotation(QImage *orgImage, QObject *parent=0, double angle=0.0, 
                 bool antialiasing=true, int autoCrop=NoAutoCrop, QColor backgroundColor=Qt::black, 
                 int orgW=0, int orgH=0);
    
    ~FreeRotation(){};
    
    QSize getNewSize(void){ return m_newSize; };

public:
    
    enum AutoCropTypes 
    {
    NoAutoCrop=0,
    WidestArea,
    LargestArea
    };
        
private:  

    bool   m_antiAlias;
    
    int    m_autoCrop;
    int    m_orgW;
    int    m_orgH;
    
    double m_angle;
    
    QSize  m_newSize;
 
    QColor m_backgroundColor;
        
private:  

    virtual void filterImage(void);
           
    inline int setPosition (int Width, int X, int Y)
       {
       return (Y *Width*4 + 4*X); 
       };
    
    inline bool isInside (int Width, int Height, int X, int Y)
       {
       bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
       bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);
       return (bIsWOk && bIsHOk);
       };
};    

}  // NameSpace DigikamFreeRotationImagesPlugin

#endif /* FREE_ROTATION_H */
