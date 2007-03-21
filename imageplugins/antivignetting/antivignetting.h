/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date  : 2005-05-25
 * Description : Antivignetting threaded image filter.
 * 
 * Copyright 2005-2007 by Gilles Caulier
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
  
#ifndef ANTIVIGNETTING_H
#define ANTIVIGNETTING_H

// Digikam includes.

#include "dimgthreadedfilter.h"

namespace DigikamAntiVignettingImagesPlugin
{

class AntiVignetting : public Digikam::DImgThreadedFilter
{

public:
    
    AntiVignetting(Digikam::DImg *orgImage, QObject *parent=0, double density=2.0,
                   double power=1.0, double radius=1.0, int xshift=0, int yshift=0, bool normalize=true);
    
    ~AntiVignetting(){};
            
private: 

    virtual void filterImage(void);

private:  

    bool   m_normalize;

    int    m_xshift;
    int    m_yshift;

    double m_density;
    double m_power;
    double m_radius;
};    

}  // NameSpace DigikamAntiVignettingImagesPlugin

#endif /* ANTIVIGNETTING_H */
