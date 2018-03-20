/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-01-12
 * Description : exposure indicator settings container.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef EXPOSURECONTAINER_H
#define EXPOSURECONTAINER_H

// Qt includes

#include <QColor>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ExposureSettingsContainer
{

public:

    ExposureSettingsContainer()
    {
        underExposureIndicator = false;
        overExposureIndicator  = false;
        exposureIndicatorMode  = true;

        underExposurePercent   = 1.0;
        overExposurePercent    = 1.0;

        underExposureColor     = Qt::white;
        overExposureColor      = Qt::black;
    };

    virtual ~ExposureSettingsContainer()
    {
    };

public:

    bool   underExposureIndicator;
    bool   overExposureIndicator;

    /** If this option is true, over and under exposure indicators will be displayed
        only when pure white and pure black color matches, as all color components match
        the condition in the same time.
        Else indicators are turn on when one of color components match the condition.
     */
    bool   exposureIndicatorMode;

    float  underExposurePercent;
    float  overExposurePercent;

    QColor underExposureColor;
    QColor overExposureColor;
};

}  // namespace Digikam

#endif  // EXPOSURECONTAINER_H
