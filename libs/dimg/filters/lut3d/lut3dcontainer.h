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

#ifndef LUT3DCONTAINER_H
#define LUT3DCONTAINER_H

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class FilterAction;

class DIGIKAM_EXPORT Lut3DContainer
{

public:
    Lut3DContainer();
    Lut3DContainer(const QString& path, int intensity = 100);

    bool operator==(const Lut3DContainer& other) const;

    void writeToFilterAction(FilterAction& action, const QString& prefix = QString()) const;
    static Lut3DContainer fromFilterAction(const FilterAction& action, const QString& prefix = QString());

public:

    QString path;
    int     intensity;
};

}  // namespace Digikam

#endif /* LUT3DCONTAINER_H */
