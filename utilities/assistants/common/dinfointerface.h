/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : template interface to image informations.
 *               This class do not depend of digiKam database library
 *               to permit to re-use tools on Showfoto.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DINFO_INTERFACE_H
#define DINFO_INTERFACE_H

// Qt includes

#include <QMap>
#include <QString>
#include <QObject>
#include <QVariant>
#include <QUrl>
#include <QList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DInfoInterface : public QObject
{

public:

    typedef QMap<QString, QVariant> DInfoMap; // Map of properties name and value.

public:

    explicit DInfoInterface(QObject* const parent);
    ~DInfoInterface();

    virtual QList<QUrl> currentSelection()  const;
    virtual QList<QUrl> currentAlbum()      const;
    virtual QList<QUrl> allAlbums()         const;

    virtual DInfoMap albumInfo(const QUrl&) const;
    virtual DInfoMap itemInfo(const QUrl&)  const;
};

}  // namespace Digikam

#endif  // DINFO_INTERFACE_H
