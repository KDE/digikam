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

#include "geolocationfilter.h"

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

GeolocationFilter::GeolocationFilter(QWidget* const parent)
    : QComboBox(parent)
{
    addItem( i18n("No geo filtering"),           QVariant::fromValue(ImageFilterSettings::GeolocationNoFilter) );
    addItem( i18n("Images with coordinates"),    QVariant::fromValue(ImageFilterSettings::GeolocationHasCoordinates) );
    addItem( i18n("Images without coordinates"), QVariant::fromValue(ImageFilterSettings::GeolocationNoCoordinates) );

    setToolTip(i18n("Filter by geolocation"));
    setWhatsThis(i18n("Select how geolocation should affect the images which are shown."));

    setGeolocationFilter(ImageFilterSettings::GeolocationNoFilter);

    connect(this, SIGNAL(activated(int)),
            this, SLOT(slotFilterChanged()));
}

GeolocationFilter::~GeolocationFilter()
{
}

void GeolocationFilter::setGeolocationFilter(const ImageFilterSettings::GeolocationCondition& condition)
{
    // findData does not seem to work...
//     const int newIndex = findData(QVariant::fromValue<ImageFilterSettings::GeolocationCondition>(condition));

    for (int i=0; i<count(); ++i)
    {
        const ImageFilterSettings::GeolocationCondition currentdata = itemData(i).value<ImageFilterSettings::GeolocationCondition>();
        if (currentdata == condition)
        {
            setCurrentIndex(i);
            emit signalFilterChanged(condition);
            break;
        }
    }
}

ImageFilterSettings::GeolocationCondition GeolocationFilter::geolocationFilter() const
{
    return itemData(currentIndex()).value<ImageFilterSettings::GeolocationCondition>();
}

void GeolocationFilter::slotFilterChanged()
{
    emit signalFilterChanged(geolocationFilter());
}

}  // namespace Digikam
