/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : filter to add Film Grain to image.
 * 
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
  
#ifndef FILMGRAINFILTER_H
#define FILMGRAINFILTER_H

// Local includes

#include "dimgthreadedfilter.h"

using namespace Digikam;

namespace DigikamFilmGrainImagesPlugin
{

class FilmGrainFilter : public DImgThreadedFilter
{

public:

    explicit FilmGrainFilter(DImg* orgImage, QObject* parent=0, int sensibility=400);
    ~FilmGrainFilter(){};

private:

    void filterImage();

private:

    int m_sensibility;
};

}  // namespace DigikamFilmGrainImagesPlugin

#endif /* FILMGRAINFILTER_H */
