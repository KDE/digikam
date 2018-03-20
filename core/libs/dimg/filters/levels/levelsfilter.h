/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-25-02
 * Description : Levels image filter
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef LEVELSFILTER_H
#define LEVELSFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT LevelsContainer
{

public:

    LevelsContainer()
    {
        for (int i=0 ; i<5 ; ++i)
        {
            lInput[i]  = 0;
            hInput[i]  = 65535;
            lOutput[i] = 0;
            hOutput[i] = 65535;
            gamma[i]   = 1.0;
        }
    };

    ~LevelsContainer() {};

public:

    int    lInput[5];
    int    hInput[5];
    int    lOutput[5];
    int    hOutput[5];

    double gamma[5];
};

// --------------------------------------------------------------------------------

class DIGIKAM_EXPORT LevelsFilter : public DImgThreadedFilter
{

public:

    explicit LevelsFilter(QObject* const parent = 0);
    explicit LevelsFilter(DImg* const orgImage, QObject* const parent=0, const LevelsContainer& settings=LevelsContainer());
    explicit LevelsFilter(const LevelsContainer& settings, DImgThreadedFilter* const master,
                          const DImg& orgImage, DImg& destImage, int progressBegin=0, int progressEnd=100);
    virtual ~LevelsFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:LevelsFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Levels Adjust Tool"));
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

private:

    LevelsContainer m_settings;
};

} // namespace Digikam

#endif // LEVELSFILTER_H
