/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date   : 2005-05-25
 * Description : Infrared threaded image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef INFRAREDFILTER_H
#define INFRAREDFILTER_H

// Local includes

#include "dimgthreadedfilter.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT InfraredContainer
{

public:

    InfraredContainer()
    {
        redGain     = 0.4;
        greenGain   = 2.1;
        blueGain    = -0.8;
        sensibility = 200;
    };

    ~InfraredContainer() {};

public:

    // Sensibility: 200..2600 ISO
    int    sensibility;

    double redGain;
    double greenGain;
    double blueGain;
};

// ---------------------------------------------------------------------------

class DIGIKAM_EXPORT InfraredFilter : public DImgThreadedFilter
{

public:

    explicit InfraredFilter(QObject* parent = 0);
    explicit InfraredFilter(DImg* orgImage, QObject* parent=0, const InfraredContainer& settings=InfraredContainer());
    ~InfraredFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:InfraredFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Infrared Filter"));
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

    inline int intMult8(uint a, uint b);
    inline int intMult16(uint a, uint b);

private:

    InfraredContainer m_settings;
};

}  // namespace Digikam

#endif /* INFRAREDFILTER_H */
