/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-05-25
 * Description : LensDistortion threaded image filter.
 * 
 * Copyright 2005-2007 by Gilles Caulier
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

#include "dimgthreadedfilter.h"

namespace DigikamLensDistortionImagesPlugin
{

class LensDistortion : public Digikam::DImgThreadedFilter
{

public:
    
    LensDistortion(Digikam::DImg *orgImage, QObject *parent=0, double main=0.0, 
                   double edge=0.0, double rescale=0.0, double brighten=0.0,
                   int center_x=0, int center_y=0);
    
    ~LensDistortion(){};
            
private:  

    virtual void filterImage(void);

private:  

    int    m_centre_x;
    int    m_centre_y;

    double m_main;
    double m_edge;
    double m_rescale;
    double m_brighten;
};    

}  // NameSpace DigikamLensDistortionImagesPlugin

#endif /* LENS_DISTORTION_H */
