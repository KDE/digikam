/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008      by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "config-digikam.h"
#include "dimgthreadedfilter.h"
#include "digikam_export.h"

// The filter is only compiled if GLib is available
#ifdef HAVE_GLIB2

namespace Digikam
{

class DIGIKAM_EXPORT LensFunContainer
{

public:

    LensFunContainer()
    {
        filterCCA       = true;
        filterVIG       = true;
        filterCCI       = true;
        filterDST       = true;
        filterGEO       = true;
        focalLength     = -1.0;
        aperture        = -1.0;
        subjectDistance = -1.0;
        cropFactor      = -1.0;
        cameraMake      = QString();
        cameraModel     = QString();
        lensModel       = QString();
    };

    ~LensFunContainer()
    {
    };

public:

    bool      filterCCA;       /// Chromatic Aberation Corrections
    bool      filterVIG;       /// Vigneting Corrections
    bool      filterCCI;       /// Color Contribution Index Corrections
    bool      filterDST;       /// Distortion Corrections
    bool      filterGEO;       /// Geometry Corrections

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

    explicit LensFunFilter(QObject* const parent = 0);
    explicit LensFunFilter(DImg* const origImage, QObject* const parent, const LensFunContainer& settings);
    ~LensFunFilter();

    bool registerSettingsToXmp(KExiv2Data& data) const;
    void readParameters(const FilterAction& action);

    static QString          FilterIdentifier()
    {
        return "digikam:LensFunFilter";
    }

    static QString          DisplayableName()
    {
        return I18N_NOOP("Lens Auto-Correction Tool");
    }

    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int              CurrentVersion()
    {
        return 1;
    }

    virtual QString         filterIdentifier() const
    {
        return FilterIdentifier();
    }

    virtual FilterAction    filterAction();

private:

    void filterImage();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // HAVE_GLIB2

#endif /* LENSFUNFILTER_H */
