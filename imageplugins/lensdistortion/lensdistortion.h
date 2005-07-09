/* ============================================================
 * File  : lensdistortion.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : LensDistortion threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Original Distortion Correction algorithm copyrighted 
 * 2001-2003 David Hodson <hodsond@acm.org>
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
  
#ifndef LENS_DISTORTION_H
#define LENS_DISTORTION_H

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamLensDistortionImagesPlugin
{

class LensDistortion : public Digikam::ThreadedFilter
{

public:
    
    LensDistortion(QImage *orgImage, QObject *parent=0, double main=0.0, 
                   double edge=0.0, double rescale=0.0, double brighten=0.0,
                   int centre_x=0, int centre_y=0);
    
    ~LensDistortion(){};
            
private:  

    double m_main;
    double m_edge;
    double m_rescale;
    double m_brighten;
    
    int    m_centre_x;
    int    m_centre_y;
    
private:  

    virtual void filterImage(void);

};    

}  // NameSpace DigikamLensDistortionImagesPlugin

#endif /* LENS_DISTORTION_H */
