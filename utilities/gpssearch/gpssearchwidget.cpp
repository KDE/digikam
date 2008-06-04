/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-30
 * Description : a widget to search image over a map.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes.

#include <klocale.h>

// Local includes.

#include "gpssearchwidget.h"

namespace Digikam
{

#ifdef HAVE_MARBLEWIDGET

class GPSSearchWidgetPriv
{
public:

    GPSSearchWidgetPriv(){}

    QRectF selection;
};

GPSSearchWidget::GPSSearchWidget(QWidget *parent)
               : MarbleWidget(parent)
{
    d = new GPSSearchWidgetPriv;

#ifdef MARBLE_VERSION
    setMapThemeId("earth/srtm/srtm.dgml");
#endif // MARBLE_VERSION

    connect(this, SIGNAL(newSelection(const QRectF&)),
            this, SLOT(slotNewSelection(const QRectF&)));
}

GPSSearchWidget::~GPSSearchWidget()
{
    delete d;
}

bool GPSSearchWidget::asSelection() const
{
    return d->selection.isValid();
}

QRectF GPSSearchWidget::selectionCoordinates() const
{
    return d->selection;
}

void GPSSearchWidget::slotNewSelection(const QRectF& sel)
{
    d->selection = sel;
    emit signalNewSelection();
}

#else // // HAVE_MARBLEWIDGET

GPSSearchWidget::GPSSearchWidget(QWidget *parent)
               : QLabel(parent)
{
    setText(i18n("Geolocation using Marble not available"));
    setWordWrap(true);
}

GPSSearchWidget::~GPSSearchWidget()
{
}

#endif // HAVE_MARBLEWIDGET

}  // namespace Digikam
