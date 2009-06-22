/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : database album interface.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef THUMBNAILDB_H
#define THUMBNAILDB_H

// Qt includes

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>
#include <QtCore/QPair>
#include <QtCore/QMap>
#include <QtCore/QHash>

// KDE includes

// Local includes

#include "thumbnaildatabaseaccess.h"
#include "digikam_export.h"

namespace Digikam
{

class DatabaseCoreBackend;
class ThumbnailDBPriv;

namespace DatabaseThumbnail
{

enum Type
{
    UndefinedType = 0,
    NoThumbnail,
    PGF,
    JPEG,              // Warning : no alpha chanel support. Cannot be used as well.
    JPEG2000,
    PNG
    //FreeDesktopHash
};

} // namespace DatabaseThumbnail

class DIGIKAM_EXPORT DatabaseThumbnailInfo
{

public:

    DatabaseThumbnailInfo()
        : id(-1), type(DatabaseThumbnail::UndefinedType)
    {
    }

    int                     id;
    DatabaseThumbnail::Type type;
    QDateTime               modificationDate;
    int                     orientationHint;
    QByteArray              data;
};

class DIGIKAM_EXPORT ThumbnailDB
{

public:

    void setSetting(const QString& keyword, const QString& value);
    QString getSetting(const QString& keyword);

    DatabaseThumbnailInfo findByHash(const QString &uniqueHash, int fileSize);
    DatabaseThumbnailInfo findByFilePath(const QString &path);

    void insertUniqueHash(const QString &uniqueHash, int fileSize, int thumbId);
    void insertFilePath(const QString &path, int thumbId);

    /** Removes thumbnail data associated to the given uniqueHash/fileSize */
    void removeByUniqueHash(const QString &uniqueHash, int fileSize);
    /** Removes thumbnail data associated to the given file path */
    void removeByFilePath(const QString &path);

    int insertThumbnail(const DatabaseThumbnailInfo &info);
    void replaceThumbnail(const DatabaseThumbnailInfo &info);

    QHash<QString, int> getValidFilePaths();
    QHash<QString, int> getInvalidFilePaths();

private:

    friend class Digikam::ThumbnailDatabaseAccess;

    ThumbnailDB(DatabaseCoreBackend *backend);
    ~ThumbnailDB();

    ThumbnailDBPriv* const d;
};

}  // namespace Digikam

#endif /* THUMBNAILDB_H */
