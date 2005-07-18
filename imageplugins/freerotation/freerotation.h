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

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamFreeRotationImagesPlugin
{

class FreeRotation : public Digikam::ThreadedFilter
{

public:
    
    FreeRotation(QImage *orgImage, QObject *parent=0, double angle=0.0, int orgW=0, int orgH=0);
    
    ~FreeRotation(){};
    
    QSize getNewSize(void){ return m_newSize; };
            
private:  

    int m_orgW;
    int m_orgH;
    
    double m_angle;
    
    QSize  m_newSize;
    
private:  

    virtual void filterImage(void);

};    

}  // NameSpace DigikamFreeRotationImagesPlugin

#endif /* FREE_ROTATION_H */
