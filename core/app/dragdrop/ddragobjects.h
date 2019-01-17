/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-02-29
 * Description : Drag object info containers.
 *
 * Copyright (C) 2008-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DDRAG_OBJECTS_H
#define DIGIKAM_DDRAG_OBJECTS_H

// Qt includes

#include <QMimeData>
#include <QList>
#include <QStringList>
#include <QUrl>

// Local includes

#include "cameratype.h"

class QWidget;

namespace Digikam
{

/**
 * Provides a drag object with additional information for internal drag&drop
 *
 * Images can be moved through ItemDrag. It is possible to move them on
 * another application which is supported through QT to e.g. copy the images.
 * digiKam can use the IDs, if ItemDrag is dropped on digikam itself.
 * The urls set via setUrls() are used for external drops (k3b, gimp, ...)
 */
class DItemDrag : public QMimeData
{
public:

    DItemDrag(const QList<QUrl>& urls,
              const QList<int>& albumIDs,
              const QList<qlonglong>& imageIDs);

    static bool canDecode(const QMimeData* e);
    static QStringList mimeTypes();
    static bool decode(const QMimeData* e,
                       QList<QUrl>& urls,
                       QList<int>& albumIDs,
                       QList<qlonglong>& imageIDs);
};

// ------------------------------------------------------------------------

/**
 * Provides a drag object for an album
 *
 * When an album is moved through drag'n'drop an object of this class
 * is created.
 */
class DAlbumDrag : public QMimeData
{
public:

    DAlbumDrag(const QUrl& databaseUrl, int albumid, const QUrl& fileUrl = QUrl());
    static QStringList mimeTypes();
    static bool canDecode(const QMimeData* e);
    static bool decode(const QMimeData* e, QList<QUrl>& urls, int& albumID);
};

// ------------------------------------------------------------------------

/**
 * Provides a drag object for a list of tags
 *
 * When a tag is moved through drag'n'drop an object of this class
 * is created.
 */
class DTagListDrag : public QMimeData
{
public:

    explicit DTagListDrag(const QList<int>& tagIDs);
    static QStringList mimeTypes();
    static bool canDecode(const QMimeData* e);
    static bool decode(const QMimeData* e, QList<int>& tagIDs);
};

// ------------------------------------------------------------------------

/**
 * Provides a drag object for a list of camera items
 *
 * When a camera item is moved through drag'n'drop an object of this class
 * is created.
 */
class DCameraItemListDrag : public QMimeData
{
public:

    explicit DCameraItemListDrag(const QStringList& cameraItemPaths);
    static QStringList mimeTypes();
    static bool canDecode(const QMimeData* e);
    static bool decode(const QMimeData* e, QStringList& cameraItemPaths);
};

// ------------------------------------------------------------------------

/**
 * Provides a drag object for a camera object
 *
 * When a camera object is moved through drag'n'drop an object of this class
 * is created.
 */
class DCameraDragObject : public QMimeData
{

public:

    explicit DCameraDragObject(const CameraType& ctype);
    static QStringList mimeTypes();
    static bool canDecode(const QMimeData* e);
    static bool decode(const QMimeData* e, CameraType& ctype);
};

} // namespace Digikam

#endif // DIGIKAM_DDRAG_OBJECTS_H
