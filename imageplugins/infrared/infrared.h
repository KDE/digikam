/* ============================================================
 * File  : infrared.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2005-05-25
 * Description : Infrared threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006 by Gilles Caulier and Marcel Wiesweg
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
  
#ifndef INFRARED_H
#define INFRARED_H

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamInfraredImagesPlugin
{

class Infrared : public Digikam::DImgThreadedFilter
{

public:

    Infrared(Digikam::DImg *orgImage, QObject *parent=0, int sensibility=1, bool grain=true);

    ~Infrared(){};

private:  // Infrared filter data.

    bool m_grain;

    int  m_sensibility;

private:  // Infrared filter methods.

    virtual void filterImage(void);

    void infraredImage(Digikam::DImg *orgImage, int Sensibility, bool Grain);

};

}  // NameSpace DigikamInfraredImagesPlugin

#endif /* INFRARED_H */
