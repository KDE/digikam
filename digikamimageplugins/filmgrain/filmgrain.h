/* ============================================================
 * File  : filmgrain.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Filmgrain threaded image filter.
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
  
#ifndef FILMGRAIN_H
#define FILMGRAIN_H

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamFilmGrainImagesPlugin
{

class FilmGrain : public Digikam::ThreadedFilter
{

public:
    
    FilmGrain(QImage *orgImage, QObject *parent=0, int sensibility=12);
    
    ~FilmGrain(){};
    
private:  // FilmGrain filter data.

    int  m_sensibility;
    
private:  // FilmGrain filter methods.

    virtual void filterImage(void);

    void filmgrainImage(uint* data, int Width, int Height, int Sensibility);

};    

}  // NameSpace DigikamFilmGrainImagesPlugin

#endif /* FILMGRAIN_H */
