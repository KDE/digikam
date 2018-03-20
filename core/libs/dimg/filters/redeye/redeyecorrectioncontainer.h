/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-15
 * Description : Red Eyes auto conrrection settings container.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2016      by Omar Amin <Omar dot moh dot amin at gmail dot com>
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

#ifndef REDEYECORRECTIONCONTAINER_H
#define REDEYECORRECTIONCONTAINER_H

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class FilterAction;

class DIGIKAM_EXPORT RedEyeCorrectionContainer
{

public:

    RedEyeCorrectionContainer();

    bool isDefault() const;
    bool operator==(const RedEyeCorrectionContainer& other) const;

    void writeToFilterAction(FilterAction& action, const QString& prefix = QString()) const;
    static RedEyeCorrectionContainer fromFilterAction(const FilterAction& action, const QString& prefix = QString());

public:

    double m_redToAvgRatio;
};

}  // namespace Digikam

#endif /* REDEYECORRECTIONCONTAINER_H */
