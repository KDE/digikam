/* ============================================================
 * File  : emboss.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Emboss threaded image filter.
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
  
#ifndef EMBOSS_H
#define EMBOSS_H

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamEmbossImagesPlugin
{

class Emboss : public Digikam::ThreadedFilter
{

public:
    
    Emboss(QImage *orgImage, QObject *parent=0, int depth=30);
    
    ~Emboss(){};
    
private:  // Emboss filter data.

    int m_depth;
    
private:  // Emboss filter methods.

    virtual void filterImage(void);

    
    void embossImage(uint* data, int Width, int Height, int d);

    // Function to limit the max and min values defined by the developer.
    
    inline int Lim_Max (int Now, int Up, int Max)
           { 
           --Max;
           while (Now > Max - Up)
               --Up;
           return (Up);
           };
};    

}  // NameSpace DigikamEmbossImagesPlugin

#endif /* EMBOSS_H */
