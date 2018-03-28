/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : interface to database information for shared tools.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DB_INFO_IFACE_H
#define DB_INFO_IFACE_H

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

    QList<QUrl> currentSelectedItems()               const;
    QList<QUrl> currentAlbumItems()                  const;

    QList<QUrl> albumItems(Album* const album)       const;
    QList<QUrl> albumItems(int id)                   const;
    QList<QUrl> albumsItems(const DAlbumIDs&)        const;
    QList<QUrl> allAlbumItems()                      const;

    DInfoMap    albumInfo(int)                       const;
    DInfoMap    itemInfo(const QUrl&)                const;

    QWidget*    albumChooser(QWidget* const parent)  const;
    DAlbumIDs   albumChooserItems()                  const;
    bool        supportAlbums()                      const;

    QWidget*    uploadWidget(QWidget* const parent)  const;
    QUrl        uploadUrl()                          const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DB_INFO_IFACE_H
