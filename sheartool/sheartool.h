/* ============================================================
 * File  : sheartool.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-18
 * Description : Shear tool threaded image filter.
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
  
#ifndef SHEAR_TOOL_H
#define SHEAR_TOOL_H

// Qt includes.

#include <qsize.h>
#include <qcolor.h>

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamShearToolImagesPlugin
{

class ShearTool : public Digikam::ThreadedFilter
{

public:
    
    ShearTool(QImage *orgImage, QObject *parent=0, float hAngle=0.0, float vAngle=0.0,
                 bool antialiasing=true, QColor backgroundColor=Qt::black, int orgW=0, int orgH=0);
    
    ~ShearTool(){};
    
    QSize getNewSize(void){ return m_newSize; };
            
private:  

    bool   m_antiAlias;

    int    m_orgW;
    int    m_orgH;
    
    float  m_hAngle;
    float  m_vAngle;
    
    QColor m_backgroundColor;
    
    QSize  m_newSize;
    
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

}  // NameSpace DigikamShearToolImagesPlugin

#endif /* SHEAR_TOOL_H */
