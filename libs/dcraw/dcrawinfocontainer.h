/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2007-05-02
 * Description : RAW file identification information container 
 *
 * Copyright 2007 by Gilles Caulier
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

#ifndef DCRAWINFOCONTAINER_H
#define DCRAWINFOCONTAINER_H

// QT includes.

#include <qstring.h>
#include <qdatetime.h>

namespace Digikam
{

class DcrawInfoContainer
{

public:

    DcrawInfoContainer()
    {
        sensitivity  = -1;
        exposureTime = -1.0;
        aperture     = -1.0;
        focalLength  = -1.0;
    };
    
    bool isEmpty()
    {
        if ( model.isEmpty()       && 
             exposureTime == -1.0  && 
             aperture == -1.0      && 
             focalLength == -1.0   && 
             sensitivity == -1     && 
             !dateTime.isValid() )
            return true;
        else
            return false;
    };
    
    long      sensitivity;

    float     exposureTime;   // ==> 1/exposureTime = exposure time in seconds.
    float     aperture;       // ==> Aperture value in APEX.
    float     focalLength;    // ==> Focal Length value in mm.

    QString   model;

    QDateTime dateTime;
};

} // namespace Digikam

#endif /* DCRAWINFOCONTAINER_H */
