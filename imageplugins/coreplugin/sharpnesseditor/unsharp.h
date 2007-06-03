/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Unsharp Mask threaded image filter.
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
  
#ifndef UNSHARP_MASK_H
#define UNSHARP_MASK_H

// Local includes.

#include "dimgthreadedfilter.h"

namespace DigikamImagesPluginCore
{

class UnsharpMask : public Digikam::DImgThreadedFilter
{

public:
    
    UnsharpMask(Digikam::DImg *orgImage, QObject *parent=0, int radius=1, 
                double amount=1.0, double threshold=0.05);
    
    ~UnsharpMask(){};
    
private:  

   virtual void filterImage(void);

private:  

    int    m_radius;

    double m_amount;
    double m_threshold;
};    

}  // NameSpace DigikamImagesPluginCore

#endif /* UNSHARP_MASK_H */
