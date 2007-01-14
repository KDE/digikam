/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2007-01-12
 * Description : exposure indicator settings container.
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

#ifndef EXPOSURESETTINGSCONTAINER_H
#define EXPOSURESETTINGSCONTAINER_H

// Qt includes.

#include <qcolor.h>

namespace Digikam
{

class ExposureSettingsContainer
{

public:
    
    ExposureSettingsContainer()
    {
        underExposureIndicator = false;
        overExposureIndicator  = false;

        underExposureColor     = Qt::white;
        overExposureColor      = Qt::black;
    };
    
    ~ExposureSettingsContainer(){};

public:

    bool   underExposureIndicator;
    bool   overExposureIndicator;

    QColor underExposureColor;
    QColor overExposureColor;
};

}  // namespace Digikam

#endif  // EXPOSURESETTINGSCONTAINER_H
