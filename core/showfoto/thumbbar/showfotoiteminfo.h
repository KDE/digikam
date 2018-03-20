/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-1
 * Description : Showfoto item info container
 *
 * Copyright (C) 2013 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef SHOWFOTOITEMINFO_H
#define SHOWFOTOITEMINFO_H

// Qt includes

#include <QList>
#include <QByteArray>
#include <QDebug>
#include <QUrl>

// Local includes

#include "infocontainer.h"

using namespace Digikam;

class QDataStream;

namespace ShowFoto
{

class ShowfotoItemInfo
{
public:

    ShowfotoItemInfo();
    ~ShowfotoItemInfo();

    /** Return true if all member in this container are null.
     */
    bool isNull() const;

    /** Compare for information equality and un-equality, not including variable values.
     */
    bool operator==(const ShowfotoItemInfo& info) const;
    bool operator!=(const ShowfotoItemInfo& info) const;

public:

    /// Static values.
    qint64             size;                 // file size in bytes.
    QUrl               url;                  // file Url

    QString            name;                 // File name in file-system
    QString            folder;               // Folder path to access to file
    QString            mime;                 // Type mime of file

    /// Unique image id
    qlonglong          id;

    PhotoInfoContainer photoInfo;

    QDateTime          dtime;                // creation time on disk
    QDateTime          ctime;                // camera date stamp
    int                width;                // Image width in pixels
    int                height;               // Image height in pixels
};

QDataStream& operator<<(QDataStream&, const ShowfotoItemInfo&);
QDataStream& operator>>(QDataStream&, ShowfotoItemInfo&);

typedef QList<ShowfotoItemInfo> ShowfotoItemInfoList;

//! qDebug() stream operator. Writes property @a info to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const ShowfotoItemInfo& info);

} // namespace Showfoto

#endif // ShowfotoItemInfo_H
