/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : Chanels mixer filter
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

#ifndef MIXERFILTER_H
#define MIXERFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT MixerContainer
{

public:

    MixerContainer()
    {
        bPreserveLum   = true;
        bMonochrome    = false;
        redRedGain     = 1.0;
        redGreenGain   = 0.0;
        redBlueGain    = 0.0;
        greenRedGain   = 0.0;
        greenGreenGain = 1.0;
        greenBlueGain  = 0.0;
        blueRedGain    = 0.0;
        blueGreenGain  = 0.0;
        blueBlueGain   = 1.0;
        blackRedGain   = 1.0;
        blackGreenGain = 0.0;
        blackBlueGain  = 0.0;
    };

    ~MixerContainer()
    {
    };

public:

    bool   bPreserveLum;
    bool   bMonochrome;

    // Standard settings.
    double redRedGain;
    double redGreenGain;
    double redBlueGain;
    double greenRedGain;
    double greenGreenGain;
    double greenBlueGain;
    double blueRedGain;
    double blueGreenGain;
    double blueBlueGain;

    // Monochrome settings.
    double blackRedGain;
    double blackGreenGain;
    double blackBlueGain;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT MixerFilter : public DImgThreadedFilter
{

public:

    explicit MixerFilter(QObject* const parent = 0);
    explicit MixerFilter(DImg* const orgImage, QObject* const parent=0, const MixerContainer& settings=MixerContainer());
    virtual ~MixerFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:MixerFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Channel Mixer Tool"));
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
    void                    readParameters(const FilterAction& action);

private:

    void filterImage();

    inline double CalculateNorm(double RedGain, double GreenGain, double BlueGain, bool bPreserveLum);

    inline unsigned short MixPixel(double RedGain, double GreenGain, double BlueGain,
                                   unsigned short R, unsigned short G, unsigned short B, bool sixteenBit,
                                   double Norm);

private:

    MixerContainer m_settings;
};

}  // namespace Digikam

#endif /* MIXERFILTER_H */
