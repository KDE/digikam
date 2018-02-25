/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : interface to item informations for shared tools
 *               based on DMetadata.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DMETA_INFO_IFACE_H
#define DMETA_INFO_IFACE_H

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

}  // namespace Digikam

#endif // DMETA_INFO_IFACE_H
