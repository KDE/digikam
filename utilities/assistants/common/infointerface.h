/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : template interface to image informations.
 *               This class do not depend of digiKam database library
 *               to permeit to re-use tools on Showfoto.
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

#ifndef INFO_INTERFACE_H
#define INFO_INTERFACE_H

// Qt includes

#include <QMap>
#include <QString>
#include <QObject>
#include <QVariant>
#include <QUrl>
#include <QList>

namespace Digikam
{

class InfoInterface : public QObject
{
    Q_OBJECT

public:

    typedef QMap<QString, QVariant> InfoMap; // Map of properties name and value.

public:

    explicit InfoInterface(QObject* const parent);
    ~InfoInterface();

    virtual QList<QUrl> currentSelection() const;
    virtual QList<QUrl> currentAlbum()     const;
    virtual QList<QUrl> allAlbums()        const;

    virtual InfoMap albumInfo(const QUrl&) const;
    virtual InfoMap itemInfo(const QUrl&)  const;
};

}  // namespace Digikam

#endif  // INFO_INTERFACE_H
