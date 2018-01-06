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

#ifndef GPS_BOOK_MARK_OWNER_H
#define GPS_BOOK_MARK_OWNER_H

// Qt includes

#include <QMenu>
#include <QUrl>

// Local includes

#include "gpsdatacontainer.h"
#include "bookmarksmngr.h"

namespace Digikam
{

class GPSImageModel;
class GPSBookmarkModelHelper;

class GPSBookmarkOwner : public QObject
{
    Q_OBJECT

public:

    GPSBookmarkOwner(GPSImageModel* const gpsImageModel, QWidget* const parent);
    virtual ~GPSBookmarkOwner();

    void changeAddBookmark(const bool state);
    void setPositionAndTitle(const GeoCoordinates& coordinates, const QString& title);

    QMenu*                  getMenu()             const;
    BookmarksManager*       bookmarkManager()     const;
    GPSBookmarkModelHelper* bookmarkModelHelper() const;
    QString                 currentTitle()        const;
    QString                 currentUrl()          const;

Q_SIGNALS:

    void positionSelected(const GPSDataContainer& position);

private Q_SLOTS:

    void slotOpenBookmark(const QUrl&);
    void slotShowBookmarksDialog();
    void slotAddBookmark();

private:

    void createBookmarksMenu();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // GPS_BOOK_MARK_OWNER_H
