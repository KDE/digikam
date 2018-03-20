/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-22
 * Description : a widget to filter album contents by geolocation
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef GEOLOCATIONFILTER_H
#define GEOLOCATIONFILTER_H

// Qt includes

#include <QComboBox>

// Local includes

#include "imagefiltersettings.h"

namespace Digikam
{

class GeolocationFilter : public QComboBox
{
    Q_OBJECT

public:

    explicit GeolocationFilter(QWidget* const parent);
    ~GeolocationFilter();

    void setGeolocationFilter(const ImageFilterSettings::GeolocationCondition& condition);
    ImageFilterSettings::GeolocationCondition geolocationFilter() const;

Q_SIGNALS:

    void signalFilterChanged(const ImageFilterSettings::GeolocationCondition& condition);

private Q_SLOTS:

    void slotFilterChanged();
};

}  // namespace Digikam

#endif // GEOLOCATIONFILTER_H
