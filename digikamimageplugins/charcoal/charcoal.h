/* ============================================================
 * File  : charcoal.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Charcoal threaded image filter.
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
  
#ifndef CHARCOAL_H
#define CHARCOAL_H

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamCharcoalImagesPlugin
{

class Charcoal : public Digikam::ThreadedFilter
{

public:
    
    Charcoal(QImage *orgImage, QObject *parent=0, double pencil=5.0, double smooth=10.0);
    
    ~Charcoal(){};
            
private:  // Texture filter data.

    double m_pencil;
    double m_smooth;
    
private:  // Texture filter methods.

    virtual void filterImage(void);

};    

}  // NameSpace DigikamCharcoalImagesPlugin

#endif /* CHARCOAL_H */
