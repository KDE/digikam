/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-09-02
 * Description : Start Web Service methods.
 *
 * Copyright (C) 2018 by Maik Qualmann <metzpinguin at gmail dot com>
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

#include "wsstarter.h"

// Qt includes

#include <QPointer>

// Local includes

#include "digikam_debug.h"
#include "flickrwindow.h"

namespace Digikam
{

class Q_DECL_HIDDEN WSStarter::Private
{
public:

    explicit Private()
    {
    }

    QPointer<FlickrWindow> flickrWindow;
};

class Q_DECL_HIDDEN WSStarterCreator
{
public:

    WSStarter object;
};

Q_GLOBAL_STATIC(WSStarterCreator, creator)

// ------------------------------------------------------------------------------------------------

WSStarter* WSStarter::instance()
{
    return &creator->object;
}

void WSStarter::cleanUp()
{
    delete instance()->d->flickrWindow;
}

void WSStarter::exportFlickr(DInfoInterface* const iface, QWidget* const parent)
{
    instance()->toFlickr(iface, parent);
}

// ------------------------------------------------------------------------------------------------

WSStarter::WSStarter()
    : d(new Private)
{
    qDebug() << "New WSStarter";
}

WSStarter::~WSStarter()
{
    delete d;
}

void WSStarter::toFlickr(DInfoInterface* const iface, QWidget* const parent)
{
    if (d->flickrWindow && (d->flickrWindow->isMinimized() || !d->flickrWindow->isHidden()))
    {
        d->flickrWindow->showNormal();       // krazy:exclude=qmethods
        d->flickrWindow->activateWindow();
        d->flickrWindow->raise();
    }
    else
    {
        qDebug() << "New FlickrWindow";
        d->flickrWindow = new FlickrWindow(iface, parent);
        d->flickrWindow->show();
    }
}

} // namespace Digikam
