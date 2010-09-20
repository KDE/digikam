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

// Lib LensFun includes

extern "C"
{
#include <lensfun.h>
}

// KDE includes

#include <klocale.h>

// Local includes

#include "dmetadata.h"
#include "dimgthreadedfilter.h"
#include "digikam_export.h"

namespace Digikam
{

class LensFunIface;

class DIGIKAM_EXPORT LensFunContainer
{
public:

    typedef const lfCamera* DevicePtr;
    typedef const lfLens*   LensPtr;
    typedef QList<LensPtr>  LensList;

public:

    LensFunContainer()
    {
        filterCCA       = true;
        filterVig       = true;
        filterCCI       = true;
        filterDist      = true;
        filterGeom      = true;
        usedCamera      = 0;
        usedLens        = 0;
        focalLength     = -1.0;
        aperture        = -1.0;
        subjectDistance = -1.0;
        cropFactor      = -1.0;
    };

    ~LensFunContainer(){};

    QString asCommentString() const
    {
        QString ret;

        if (usedCamera && usedLens)
        {
            ret.append(i18n("Camera: %1-%2",        usedCamera->Maker, usedCamera->Model));
            ret.append("\n");
            ret.append(i18n("Lens: %1",             usedLens->Model));
            ret.append("\n");
            ret.append(i18n("Subject Distance: %1", QString::number(subjectDistance)));
            ret.append("\n");
            ret.append(i18n("Aperture: %1",         QString::number(aperture)));
            ret.append("\n");
            ret.append(i18n("Focal Length: %1",     QString::number(focalLength)));
            ret.append("\n");
            ret.append(i18n("Crop Factor: %1",      QString::number(cropFactor)));
            ret.append("\n");
            ret.append(i18n("CCA Correction: %1",   filterCCA  ? i18n("enabled") : i18n("disabled")));
            ret.append("\n");
            ret.append(i18n("VIG Correction: %1",   filterVig  ? i18n("enabled") : i18n("disabled")));
            ret.append("\n");
            ret.append(i18n("CCI Correction: %1",   filterCCI  ? i18n("enabled") : i18n("disabled")));
            ret.append("\n");
            ret.append(i18n("DST Correction: %1",   filterDist ? i18n("enabled") : i18n("disabled")));
            ret.append("\n");
            ret.append(i18n("GEO Correction: %1",   filterGeom ? i18n("enabled") : i18n("disabled")));
        }

        return ret;
    };

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

    DevicePtr usedCamera;
    LensPtr   usedLens;
};

class DIGIKAM_EXPORT LensFunFilter : public DImgThreadedFilter
{

public:

    LensFunFilter(DImg* origImage, QObject* parent, LensFunIface* iface);
    ~LensFunFilter();

    bool registerSettingsToXmp(KExiv2Data& data, const LensFunContainer settings) const;

private:

    void filterImage();

private:

    class LensFunFilterPriv;
    LensFunFilterPriv* const d;
};

}  // namespace Digikam

Q_DECLARE_METATYPE( Digikam::LensFunContainer::DevicePtr )
Q_DECLARE_METATYPE( Digikam::LensFunContainer::LensPtr )

#endif /* LENSFUNFILTER_H */
