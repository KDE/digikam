/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : interface to item information for shared tools
 *               based on DMetadata.
 *
 * Copyright (C) 2017-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAAM_DMETA_INFO_IFACE_H
#define DIGIKAAM_DMETA_INFO_IFACE_H

// Local includes

#include "digikam_export.h"
#include "dinfointerface.h"

namespace Digikam
{

class DIGIKAM_EXPORT DMetaInfoIface : public DInfoInterface
{
    Q_OBJECT

public:

    explicit DMetaInfoIface(QObject* const, const QList<QUrl>&);
    ~DMetaInfoIface();

    Q_SLOT void slotDateTimeForUrl(const QUrl& url, const QDateTime& dt, bool updModDate);
    Q_SLOT void slotMetadataChangedForUrl(const QUrl& url);
    
    Q_SIGNAL void signalItemChanged(const QUrl& url);

public:

    QList<QUrl> currentSelectedItems()            const;
    QList<QUrl> currentAlbumItems()               const;
    QList<QUrl> allAlbumItems()                   const;

    DInfoMap    itemInfo(const QUrl&)             const;
    bool        supportAlbums()                   const;

    QWidget* uploadWidget(QWidget* const parent)  const;
    QUrl     uploadUrl()                          const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAAM_DMETA_INFO_IFACE_H
