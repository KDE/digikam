/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LENSFUNFILTER_H
#define LENSFUNFILTER_H

// KDE includes

#include <klocale.h>

// Local includes

#include "dimgthreadedfilter.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT LensFunContainer
{

public:

    LensFunContainer()
    {
        filterCCA       = true;
        filterVig       = true;
        filterCCI       = true;
        filterDist      = true;
        filterGeom      = true;
        focalLength     = -1.0;
        aperture        = -1.0;
        subjectDistance = -1.0;
        cropFactor      = -1.0;
        cameraMake      = QString();
        cameraModel     = QString();
        lensModel       = QString();
    };

    ~LensFunContainer(){};

public:

    bool      filterCCA;
    bool      filterVig;
    bool      filterCCI;
    bool      filterDist;
    bool      filterGeom;

    double    cropFactor;
    double    focalLength;
    double    aperture;
    double    subjectDistance;

    QString   cameraMake;
    QString   cameraModel;
    QString   lensModel;
};

class DIGIKAM_EXPORT LensFunFilter : public DImgThreadedFilter
{

public:

    LensFunFilter(DImg* origImage, QObject* parent, const LensFunContainer& settings);
    ~LensFunFilter();

    bool registerSettingsToXmp(KExiv2Data& data) const;

private:

    void filterImage();

private:

    class LensFunFilterPriv;
    LensFunFilterPriv* const d;
};

}  // namespace Digikam

#endif /* LENSFUNFILTER_H */
