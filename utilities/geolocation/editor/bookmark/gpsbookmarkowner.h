/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-11-21
 * @brief  Central object for managing bookmarks
 *
 * @author Copyright (C) 2009,2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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

class KBookmarkManager;

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

    QMenu* getMenu() const;

    void changeAddBookmark(const bool state);
    void setPositionAndTitle(const GeoIface::GeoCoordinates& coordinates, const QString& title);

    KBookmarkManager*       bookmarkManager()     const;
    GPSBookmarkModelHelper* bookmarkModelHelper() const;

public:

    virtual bool    supportsTabs() const;
    virtual QString currentTitle() const;
    virtual QUrl    currentUrl()   const;
    virtual bool enableOption(BookmarkOption option) const;
    virtual void openBookmark(const KBookmark&, Qt::MouseButtons, Qt::KeyboardModifiers);

Q_SIGNALS:

    void positionSelected(const GPSDataContainer& position);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // GPSBOOKMARKOWNER_H
