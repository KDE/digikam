/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-18
 * Description : Shear threaded image filter.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SHEAR_H
#define SHEAR_H

// Qt includes.

#include <qsize.h>
#include <qcolor.h>

// Digikam includes.

#include "dimgthreadedfilter.h"

namespace DigikamShearToolImagesPlugin
{

class Shear : public Digikam::DImgThreadedFilter
{

public:

    Shear(Digikam::DImg *orgImage, QObject *parent=0, float hAngle=0.0, float vAngle=0.0,
          bool antialiasing=true, QColor backgroundColor=Qt::black, int orgW=0, int orgH=0);

    ~Shear(){};

    QSize getNewSize(void){ return m_newSize; };

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

private:

    bool   m_antiAlias;

    int    m_orgW;
    int    m_orgH;

    float  m_hAngle;
    float  m_vAngle;

    QColor m_backgroundColor;

    QSize  m_newSize;
};

}  // NameSpace DigikamShearToolImagesPlugin

#endif /* SHEAR_H */
