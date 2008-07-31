/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Charcoal threaded image filter.
 *
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
  
#ifndef CHARCOAL_H
#define CHARCOAL_H

// Digikam includes.

#include "dimgthreadedfilter.h"

namespace DigikamCharcoalImagesPlugin
{

class Charcoal : public Digikam::DImgThreadedFilter
{

public:
    
    Charcoal(Digikam::DImg *orgImage, QObject *parent=0, double pencil=5.0, double smooth=10.0);
    ~Charcoal(){};
            
private:

    void filterImage(void);
    bool convolveImage(const unsigned int order, const double *kernel);
    int  getOptimalKernelWidth(double radius, double sigma);

private:

    double m_pencil;
    double m_smooth;
};    

}  // NameSpace DigikamCharcoalImagesPlugin

#endif /* CHARCOAL_H */
