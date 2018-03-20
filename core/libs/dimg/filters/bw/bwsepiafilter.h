/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-06
 * Description : black and white image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef BWSEPIAFILTER_H
#define BWSEPIAFILTER_H

// Qt includes

#include <QPolygon>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"
#include "bcgfilter.h"
#include "curvesfilter.h"
#include "tonalityfilter.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT BWSepiaContainer
{

public:

    enum BlackWhiteConversionType
    {
        BWNoFilter = 0,       // B&W filter to the front of lens.
        BWGreenFilter,
        BWOrangeFilter,
        BWRedFilter,
        BWYellowFilter,
        BWYellowGreenFilter,
        BWBlueFilter,

        BWGeneric,            // B&W film simulation.
        BWAgfa200X,
        BWAgfapan25,
        BWAgfapan100,
        BWAgfapan400,
        BWIlfordDelta100,
        BWIlfordDelta400,
        BWIlfordDelta400Pro3200,
        BWIlfordFP4,
        BWIlfordHP5,
        BWIlfordPanF,
        BWIlfordXP2Super,
        BWKodakTmax100,
        BWKodakTmax400,
        BWKodakTriX,

        BWIlfordSFX200,       // Infrared film simulation.
        BWIlfordSFX400,
        BWIlfordSFX800,

        BWNoTone,             // Chemical color tone filter.
        BWSepiaTone,
        BWBrownTone,
        BWColdTone,
        BWSeleniumTone,
        BWPlatinumTone,
        BWGreenTone,

        // Filter version 2
        BWKodakHIE            // Infrared film simulation.
    };

public:

    BWSepiaContainer()
    {
        previewType = BWGeneric;
        preview     = false;
        filmType    = BWGeneric;
        filterType  = BWNoFilter;
        toneType    = BWNoTone;
        strength    = 1.0;
    };

    explicit BWSepiaContainer(int ptype)
    {
        previewType = ptype;
        preview     = true;
        strength    = 1.0;
        filmType    = BWGeneric;
        filterType  = BWNoFilter;
        toneType    = BWNoTone;
    };

    BWSepiaContainer(int ptype, const CurvesContainer& container)
    {
        previewType = ptype;
        preview     = true;
        strength    = 1.0;
        filmType    = BWGeneric;
        filterType  = BWNoFilter;
        toneType    = BWNoTone;
        curvesPrm   = container;
    };

    ~BWSepiaContainer() {};

public:

    bool            preview;

    int             previewType;
    int             filmType;
    int             filterType;
    int             toneType;

    double          strength;

    CurvesContainer curvesPrm;

    BCGContainer    bcgPrm;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT BWSepiaFilter : public DImgThreadedFilter
{

public:

    explicit BWSepiaFilter(QObject* parent = 0);
    explicit BWSepiaFilter(DImg* orgImage, QObject* parent=0, const BWSepiaContainer& settings=BWSepiaContainer());
    virtual ~BWSepiaFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:BWSepiaFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Black & White / Sepia Filter"));
    }

    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 1 << 2;
    }

    static int              CurrentVersion()
    {
        return 2;
    }

    virtual QString         filterIdentifier() const
    {
        return FilterIdentifier();
    }

    virtual FilterAction    filterAction();
    void                    readParameters(const FilterAction& action);

private:

    void filterImage();

    DImg getThumbnailForEffect(DImg& img);

    void blackAndWhiteConversion(DImg& img, int type);
    void applyChannelMixer(DImg& img);
    void applyInfraredFilter(DImg& img, int sensibility);
    void applyToneFilter(DImg& img, TonalityContainer& settings);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // BWSEPIAFILTER_H
