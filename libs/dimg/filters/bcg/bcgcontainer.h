/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-15
 * Description : a Brightness/Contrast/Gamma settings container.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BCGCONTAINER_H
#define BCGCONTAINER_H

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class FilterAction;

class DIGIKAM_EXPORT BCGContainer
{

public:

    BCGContainer();

    bool isDefault() const;
    bool operator==(const BCGContainer& other) const;

    void writeToFilterAction(FilterAction& action, const QString& prefix = QString()) const;
    static BCGContainer fromFilterAction(const FilterAction& action, const QString& prefix = QString());

public:

    int    channel;

    double brightness;
    double contrast;
    double gamma;
};

}  // namespace Digikam

#endif /* BCGCONTAINER_H */
