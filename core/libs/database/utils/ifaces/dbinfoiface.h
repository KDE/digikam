/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : interface to database information for shared tools.
 *
 * Copyright (C) 2017-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2017      by Mario Frank <mario dot frank at uni minus potsdam dot de>
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

#ifndef DIGIKAM_DB_INFO_IFACE_H
#define DIGIKAM_DB_INFO_IFACE_H

// Local includes

#include "applicationsettings.h"
#include "dinfointerface.h"
#include "digikam_export.h"

namespace Digikam
{

class Album;

class DIGIKAM_EXPORT DBInfoIface : public DInfoInterface
{
    Q_OBJECT

public:

    explicit DBInfoIface(QObject* const parent,
                         const QList<QUrl>& lst = QList<QUrl>(),
                         const ApplicationSettings::OperationType type = ApplicationSettings::Unspecified);
    ~DBInfoIface();

public:

    Q_SLOT void slotDateTimeForUrl(const QUrl& url, const QDateTime& dt, bool updModDate) override;
    Q_SLOT void slotMetadataChangedForUrl(const QUrl& url) override;

public:

    QList<QUrl> currentSelectedItems()                    const override;
    QList<QUrl> currentAlbumItems()                       const override;

    QList<QUrl> albumItems(Album* const album)            const;
    QList<QUrl> albumItems(int id)                        const override;
    QList<QUrl> albumsItems(const DAlbumIDs&)             const override;
    QList<QUrl> allAlbumItems()                           const override;

    DInfoMap    albumInfo(int)                            const override;

    DInfoMap    itemInfo(const QUrl&)                     const override;
    void        setItemInfo(const QUrl&, const DInfoMap&) const override;

    QWidget*    albumChooser(QWidget* const parent)       const override;
    DAlbumIDs   albumChooserItems()                       const override;
    bool        supportAlbums()                           const override;

    QWidget*    uploadWidget(QWidget* const parent)       const override;
    QUrl        uploadUrl()                               const override;

    QUrl        defaultUploadUrl()                        const override;

    QAbstractItemModel* tagFilterModel()                        override;

#ifdef HAVE_MARBLE
    QList<GPSItemContainer*> currentGPSItems()            const override;
#endif

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_DB_INFO_IFACE_H
