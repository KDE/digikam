/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Central object for managing bookmarks
 *
 * Copyright (C) 2010-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Michael G. Hansen <mike at mghansen dot de>
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

#include "gpsbookmarkowner.h"

// Qt includes

#include <QStandardItemModel>
#include <QStandardPaths>

// KDE includes

#include <kactioncollection.h>
#include <kbookmarkmenu.h>

// Local includes

#include "gpsbookmarkmodelhelper.h"
#include "gpsundocommand.h"
#include "gpsimagemodel.h"

namespace Digikam
{

class GPSBookmarkOwner::Private
{
public:

    Private()
      : parent(0),
        actionCollection(0),
        bookmarkManager(0),
        bookmarkMenuController(0),
        bookmarkMenu(0),
        addBookmarkEnabled(true),
        bookmarkModelHelper(0)
    {
    }

    QWidget*                 parent;
    KActionCollection*       actionCollection;
    KBookmarkManager*        bookmarkManager;
    KBookmarkMenu*           bookmarkMenuController;
    QMenu*                   bookmarkMenu;
    bool                     addBookmarkEnabled;
    GPSBookmarkModelHelper*  bookmarkModelHelper;
    GeoIface::GeoCoordinates lastCoordinates;
    QString                  lastTitle;
};

GPSBookmarkOwner::GPSBookmarkOwner(GPSImageModel* const gpsImageModel, QWidget* const parent)
    : d(new Private())
{
    d->parent = parent;

    // TODO: where do we save the bookmarks? right now, they are application-specific
    const QString bookmarksFileName = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                                      QLatin1Char('/') + QLatin1String("digikam/geobookmarks.xml");
    d->actionCollection             = new KActionCollection(this);
    d->bookmarkManager              = KBookmarkManager::managerForFile(bookmarksFileName, QLatin1String("digikamgeobookmarks"));
    d->bookmarkManager->setUpdate(true);
    d->bookmarkMenu                 = new QMenu(parent);
    d->bookmarkMenuController       = new KBookmarkMenu(d->bookmarkManager, this, d->bookmarkMenu, d->actionCollection);
    d->bookmarkModelHelper          = new GPSBookmarkModelHelper(d->bookmarkManager, gpsImageModel, this);
}

GPSBookmarkOwner::~GPSBookmarkOwner()
{
    delete d;
}

QMenu* GPSBookmarkOwner::getMenu() const
{
    return d->bookmarkMenu;
}

bool GPSBookmarkOwner::supportsTabs() const
{
    return false;
}

QString GPSBookmarkOwner::currentTitle() const
{
    if (d->lastTitle.isEmpty())
    {
        return currentUrl().toString();
    }

    return d->lastTitle;
}

QUrl GPSBookmarkOwner::currentUrl() const
{
    // TODO : check if this QUrl from QString conversion is fine.
    return QUrl(d->lastCoordinates.geoUrl());
}

bool GPSBookmarkOwner::enableOption(BookmarkOption option) const
{
    switch (option)
    {
        case ShowAddBookmark:
            return d->addBookmarkEnabled;

        case ShowEditBookmark:
            return true;

        default:
            return false;
    }
}

void GPSBookmarkOwner::openBookmark(const KBookmark& bookmark, Qt::MouseButtons, Qt::KeyboardModifiers)
{
    const QString url                         = bookmark.url().url().toLower();
    bool  okay                                = false;
    const GeoIface::GeoCoordinates coordinate = GeoIface::GeoCoordinates::fromGeoUrl(url, &okay);

    if (okay)
    {
        GPSDataContainer position;
        position.setCoordinates(coordinate);
        emit(positionSelected(position));
    }
}

void GPSBookmarkOwner::changeAddBookmark(const bool state)
{
    d->addBookmarkEnabled = state;

    // re-create the menus:
    // TODO: is there an easier way?
    delete d->bookmarkMenuController;
    d->bookmarkMenu->clear();
    d->bookmarkMenuController = new KBookmarkMenu(d->bookmarkManager, this, d->bookmarkMenu, d->actionCollection);
}

KBookmarkManager* GPSBookmarkOwner::bookmarkManager() const
{
    return d->bookmarkManager;
}

GPSBookmarkModelHelper* GPSBookmarkOwner::bookmarkModelHelper() const
{
    return d->bookmarkModelHelper;
}

void GPSBookmarkOwner::setPositionAndTitle(const GeoIface::GeoCoordinates& coordinates, const QString& title)
{
    d->lastCoordinates = coordinates;
    d->lastTitle       = title;
}

} // namespace Digikam
