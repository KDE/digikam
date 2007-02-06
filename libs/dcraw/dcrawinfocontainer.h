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
        sensitivity       = -1;
        exposureTime      = -1.0;
        aperture          = -1.0;
        focalLength       = -1.0;
        rawColors         = -1;
        hasIccProfile     = false;
        isDecodable       = false;
        hasSecondaryPixel = false;
        daylightMult[0]   = 0.0;
        daylightMult[1]   = 0.0;
        daylightMult[2]   = 0.0;
        cameraMult[0]     = 0.0;
        cameraMult[1]     = 0.0;
        cameraMult[2]     = 0.0;
        cameraMult[3]     = 0.0;
    };
    
    bool isEmpty()
    {
        if ( model.isEmpty()      && 
             exposureTime == -1.0 && 
             aperture     == -1.0 && 
             focalLength  == -1.0 && 
             sensitivity  == -1   && 
             rawColors    == -1   &&
             !dateTime.isValid()  && 
             !imageSize.isValid() )
            return true;
        else
            return false;
    };
    
    bool      hasSecondaryPixel;  // True if camera sensor use a secondary pixel.
    bool      hasIccProfile;      // True if RAW file include an ICC color profile.
    bool      isDecodable;        // True is RAW file is decodable by dcraw.

    int       rawColors;          // The number of RAW colors.

    long      sensitivity;        // The sensitivity in ISO used by camera to take the picture.

    float     exposureTime;       // ==> 1/exposureTime = exposure time in seconds.
    float     aperture;           // ==> Aperture value in APEX.
    float     focalLength;        // ==> Focal Length value in mm.

    double    daylightMult[3];    // White color balance settings.
    double    cameraMult[4];

    QString   make;               // The camera maker.
    QString   model;              // The camera model.

    QDateTime dateTime;           // Date & time when have been taken the picture.

    QSize     imageSize;          // The image dimensions in pixels.
};

} // namespace Digikam

#endif /* DCRAWINFOCONTAINER_H */
