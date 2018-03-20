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

// Local includes

#include "redeyecorrectioncontainer.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

RedEyeCorrectionContainer::RedEyeCorrectionContainer()
{
    m_redToAvgRatio = 2.1;
}

bool RedEyeCorrectionContainer::isDefault() const
{
    return (*this == RedEyeCorrectionContainer());
}

bool RedEyeCorrectionContainer::operator==(const RedEyeCorrectionContainer& other) const
{
    return (m_redToAvgRatio == other.m_redToAvgRatio);
}

void RedEyeCorrectionContainer::writeToFilterAction(FilterAction& action, const QString& prefix) const
{
    action.addParameter(prefix + QLatin1String("redtoavgratio"), m_redToAvgRatio);
}

RedEyeCorrectionContainer RedEyeCorrectionContainer::fromFilterAction(const FilterAction& action, const QString& prefix)
{
    RedEyeCorrectionContainer settings;
    settings.m_redToAvgRatio = action.parameter(prefix + QLatin1String("redtoavgratio"), settings.m_redToAvgRatio);

    return settings;
}

}  // namespace Digikam
