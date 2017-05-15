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

#ifndef GPSBOOKMARKOWNER_H
#define GPSBOOKMARKOWNER_H

// Qt includes

#include <QMenu>

// KDE includes

#include <kbookmarkmanager.h>

// Local includes

#include "gpsdatacontainer.h"

namespace Digikam
{

class GPSImageModel;
class GPSBookmarkModelHelper;

class GPSBookmarkOwner : public QObject, public KBookmarkOwner
{
    Q_OBJECT

public:

    GPSBookmarkOwner(GPSImageModel* const gpsImageModel, QWidget* const parent);
    virtual ~GPSBookmarkOwner();

    void changeAddBookmark(const bool state);
    void setPositionAndTitle(const GeoIface::GeoCoordinates& coordinates, const QString& title);

    QMenu* getMenu()                              const;
    KBookmarkManager*       bookmarkManager()     const;
    GPSBookmarkModelHelper* bookmarkModelHelper() const;

public:

    virtual bool    supportsTabs()                      const;
    virtual QString currentTitle()                      const;
    virtual QUrl    currentUrl()                        const;
    virtual bool    enableOption(BookmarkOption option) const;
    virtual void    openBookmark(const KBookmark&, Qt::MouseButtons, Qt::KeyboardModifiers);

Q_SIGNALS:

    void positionSelected(const GPSDataContainer& position);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // GPSBOOKMARKOWNER_H
