/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-10-10
 * Description : Lut3D color adjustment tool.
 *
 * Copyright (C) 2015 by Andrej Krutak <dev at andree dot sk>
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

#ifndef LUT3DFILTER_H
#define LUT3DFILTER_H

// Local includes

#include "dimgthreadedfilter.h"
#include "lut3dcontainer.h"
#include "digikam_export.h"

using namespace Digikam;

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT Lut3DFilter : public DImgThreadedFilter
{

public:

    explicit Lut3DFilter(QObject* const parent = 0);
    Lut3DFilter(DImg* const orgImage, const Lut3DContainer& par, QObject* const parent=0);
    virtual ~Lut3DFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:Lut3DFilter");
    }

    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int              CurrentVersion()
    {
        return 1;
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Lut3D"));
    }

    virtual QString         filterIdentifier() const
    {
        return FilterIdentifier();
    }

    virtual FilterAction    filterAction();

    void                    readParameters(const FilterAction& action);

private:

    void filterImage();
    void applyLut3D();
    void loadLut3D(const QString& path);
    void initToIdentity();

private:

    quint16* m_lutTable;     // RGBA, A is unused
    int      m_lutTableSize; // all axis are of this size
    int      m_intensity;
};

}  // namespace Digikam

#endif /* LUT3DFILTER_H */
