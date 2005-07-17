/* ============================================================
 * File  : sharpen.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-17-07
 * Description : A Sharpen threaded image filter.
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
  
#ifndef SHARPEN_H
#define SHARPEN_H

// Digikam Includes.

#include "digikam_export.h"

// Local includes.

#include "threadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT Sharpen : public Digikam::ThreadedFilter
{

public:
    
    Sharpen(QImage *orgImage, QObject *parent=0, int radius=3);
    
    ~Sharpen(){};
    
private:  // Sharpen filter data.

    int m_radius;
    
private:  // Sharpen filter methods.

    virtual void filterImage(void);
    
    void sharpenImage(uint *data, int Width, int Height, int Radius);
};    

}  // NameSpace Digikam

#endif /* SHARPEN_H */
