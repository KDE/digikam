/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2005-05-25
 * Description : Filmgrain threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
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

#include "dimgthreadedfilter.h"

namespace DigikamFilmGrainImagesPlugin
{

class FilmGrain : public Digikam::DImgThreadedFilter
{

public:

    FilmGrain(Digikam::DImg *orgImage, QObject *parent=0, int sensibility=12);

    ~FilmGrain(){};

private:

    virtual void filterImage(void);

    void filmgrainImage(Digikam::DImg *orgImage, int Sensibility);

private:

    int m_sensibility;
};

}  // NameSpace DigikamFilmGrainImagesPlugin

#endif /* FILMGRAIN_H */
