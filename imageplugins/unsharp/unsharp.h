/* ============================================================
 * File  : unsharp.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-05-25
 * Description : Unsharp Mask threaded image filter.
 * 
 * Copyright 2005-2006 by Gilles Caulier
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
  
#ifndef UNSHARPMASK_H
#define UNSHARPMASK_H

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamUnsharpMaskImagesPlugin
{

class UnsharpMask : public Digikam::DImgThreadedFilter
{

public:
    
    UnsharpMask(Digikam::DImg *orgImage, QObject *parent=0, int radius=1, 
                double amount=1.0, double threshold=0.05);
    
    ~UnsharpMask(){};
    
private:  // Unsharp Mask filter data.

    int    m_radius;

    double m_amount;
    double m_threshold;
    
private:  // Unsharp Mask filter methods.

   virtual void filterImage(void);

};    

}  // NameSpace DigikamUnsharpMaskImagesPlugin

#endif /* UNSHARPMASK_H */
