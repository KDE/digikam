/* ============================================================
 * File  : despeckle.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Despeckle threaded image filter.
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
  
#ifndef DESPECKLE_H
#define DESPECKLE_H

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamNoiseReductionImagesPlugin
{

class Despeckle : public Digikam::ThreadedFilter
{

public:
    
    Despeckle(QImage *orgImage, QObject *parent=0, int radius=3, int black_level=7, int white_level=248, 
              bool adaptativeFilter=true, bool recursiveFilter=false);
    
    ~Despeckle(){};
    
private:  // Despeckle filter data.

    int  m_radius;
    int  m_black_level;
    int  m_white_level;
    bool m_adaptativeFilter;
    bool m_recursiveFilter; 
    
private:  // Despeckle filter methods.

    virtual void filterImage(void);

    void despeckleImage(uint* data, int w, int h, int despeckle_radius, 
                        int black_level, int white_level, 
                        bool adaptativeFilter, bool recursiveFilter);

};    

}  // NameSpace DigikamNoiseReductionImagesPlugin

#endif /* DESPECKLE_H */
