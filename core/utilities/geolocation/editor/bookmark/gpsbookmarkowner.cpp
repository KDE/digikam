/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Central object for managing bookmarks
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include "gpsbookmarkmodelhelper.h"
#include "gpsundocommand.h"
#include "gpsimagemodel.h"
#include "bookmarknode.h"
#include "bookmarksmenu.h"
#include "bookmarksdlg.h"

namespace Digikam
{

class GPSBookmarkOwner::Private
{
public:

    Private()
      : parent(0),
        bookmarkManager(0),
        bookmarkMenu(0),
        addBookmarkEnabled(true),
        bookmarkModelHelper(0)
    {
    }

    QWidget*                 parent;
    BookmarksManager*        bookmarkManager;
    BookmarksMenu*           bookmarkMenu;
    bool                     addBookmarkEnabled;
    GPSBookmarkModelHelper*  bookmarkModelHelper;
    GeoCoordinates           lastCoordinates;
    QString                  lastTitle;
};

GPSBookmarkOwner::GPSBookmarkOwner(GPSImageModel* const gpsImageModel, QWidget* const parent)
    : d(new Private())
{
    d->parent = parent;

    const QString bookmarksFileName =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                                         QLatin1Char('/') +
                                         QLatin1String("digikam/geobookmarks.xml");
    d->bookmarkManager              = new BookmarksManager(bookmarksFileName, this);
    d->bookmarkManager->load();
    d->bookmarkMenu                 = new BookmarksMenu(d->bookmarkManager, d->parent);
    d->bookmarkModelHelper          = new GPSBookmarkModelHelper(d->bookmarkManager,
                                                                 gpsImageModel, this);

    createBookmarksMenu();
}

GPSBookmarkOwner::~GPSBookmarkOwner()
{
    delete d;
}

QMenu* GPSBookmarkOwner::getMenu() const
{
    return dynamic_cast<QMenu*>(d->bookmarkMenu);
}

QString GPSBookmarkOwner::currentTitle() const
{
    if (d->lastTitle.isEmpty())
    {
        return currentUrl();
    }

    return d->lastTitle;
}

QString GPSBookmarkOwner::currentUrl() const
{
    // TODO : check if this QUrl from QString conversion is fine.
    return d->lastCoordinates.geoUrl();
}

void GPSBookmarkOwner::slotOpenBookmark(const QUrl& url)
{
    const QString gps                         = url.url().toLower();
    bool  okay                                = false;
    const GeoCoordinates coordinate = GeoCoordinates::fromGeoUrl(gps, &okay);

    if (okay)
    {
        GPSDataContainer position;
        position.setCoordinates(coordinate);
        emit(positionSelected(position));
    }
}

void GPSBookmarkOwner::slotShowBookmarksDialog()
{
    BookmarksDialog* const dlg = new BookmarksDialog(d->parent, d->bookmarkManager);
    dlg->show();
}

void GPSBookmarkOwner::slotAddBookmark()
{
    AddBookmarkDialog* const dlg = new AddBookmarkDialog(currentUrl(), currentTitle(),
                                                         d->parent, d->bookmarkManager);
    dlg->exec();
}

void GPSBookmarkOwner::changeAddBookmark(const bool state)
{
    d->addBookmarkEnabled = state;

    createBookmarksMenu();
}

void GPSBookmarkOwner::createBookmarksMenu()
{
    d->bookmarkMenu->clear();

    QList<QAction*> bookmarksActions;

    QAction* const showAllBookmarksAction = new QAction(i18n("Edit Bookmarks"), d->parent);
    bookmarksActions.append(showAllBookmarksAction);

    connect(showAllBookmarksAction, SIGNAL(triggered()),
            this, SLOT(slotShowBookmarksDialog()));

    QAction* const addBookmark = new QAction(i18n("Add Bookmark..."), d->parent);
    bookmarksActions.append(addBookmark);

    connect(addBookmark, SIGNAL(triggered()),
            this, SLOT(slotAddBookmark()));

    d->bookmarkMenu->setInitialActions(bookmarksActions);

    connect(d->bookmarkMenu, SIGNAL(openUrl(QUrl)),
            this, SLOT(slotOpenBookmark(QUrl)));
}

BookmarksManager* GPSBookmarkOwner::bookmarkManager() const
{
    return d->bookmarkManager;
}

GPSBookmarkModelHelper* GPSBookmarkOwner::bookmarkModelHelper() const
{
    return d->bookmarkModelHelper;
}

void GPSBookmarkOwner::setPositionAndTitle(const GeoCoordinates& coordinates,
                                           const QString& title)
{
    d->lastCoordinates = coordinates;
    d->lastTitle       = title;
}

} // namespace Digikam
