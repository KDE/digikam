/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : database album interface.
 *
 * Copyright (C) 2009-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes

#include "databasecorebackend.h"
#include "sqlquery.h"
#include "thumbnaildatabaseaccess.h"
#include "digikam_export.h"

namespace Digikam
{

class DatabaseCoreBackend;

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
        : id(-1),
          type(DatabaseThumbnail::UndefinedType),
          orientationHint(0)
    {
    }

    int                     id;
    DatabaseThumbnail::Type type;
    QDateTime               modificationDate;
    int                     orientationHint;
    QByteArray              data;
};

// ------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT ThumbnailDB
{

public:

    bool setSetting(const QString& keyword, const QString& value);
    QString getSetting(const QString& keyword);

    DatabaseThumbnailInfo findByHash(const QString& uniqueHash, qlonglong fileSize);
    DatabaseThumbnailInfo findByFilePath(const QString& path);
    DatabaseThumbnailInfo findByCustomIdentifier(const QString& id);

    /** This is findByFilePath with extra security: Pass the uniqueHash which you have.
     *  If an entry is found by file path, and the entry is referenced by any uniqueHash,
     *  which is different from the given hash, a null info is returned.
     *  If uniqueHash is null, equivalent to the simple findByFilePath.
     */
    DatabaseThumbnailInfo findByFilePath(const QString& path, const QString& uniqueHash);

    DatabaseCoreBackend::QueryState insertUniqueHash(const QString& uniqueHash, qlonglong fileSize, int thumbId);
    DatabaseCoreBackend::QueryState insertFilePath(const QString& path, int thumbId);
    DatabaseCoreBackend::QueryState insertCustomIdentifier(const QString& id, int thumbId);

    /** Removes thumbnail data associated to the given uniqueHash/fileSize */
    DatabaseCoreBackend::QueryState removeByUniqueHash(const QString& uniqueHash, qlonglong fileSize);
    /** Removes thumbnail data associated to the given file path */
    DatabaseCoreBackend::QueryState removeByFilePath(const QString& path);
    DatabaseCoreBackend::QueryState removeByCustomIdentifier(const QString& id);

    DatabaseCoreBackend::QueryState insertThumbnail(const DatabaseThumbnailInfo& info, QVariant* const lastInsertId = 0);
    DatabaseCoreBackend::QueryState replaceThumbnail(const DatabaseThumbnailInfo& info);

    QHash<QString, int> getFilePathsWithThumbnail();

    void replaceUniqueHash(const QString& oldUniqueHash, int oldFileSize, const QString& newUniqueHash, int newFileSize);
    DatabaseCoreBackend::QueryState updateModificationDate(int thumbId, const QDateTime& modificationDate);

    //QStringList getAllThumbnailPaths();

private:

    explicit ThumbnailDB(DatabaseCoreBackend* const backend);
    ~ThumbnailDB();

private:

    class Private;
    Private* const d;

    friend class Digikam::ThumbnailDatabaseAccess;
};

}  // namespace Digikam

#endif /* THUMBNAILDB_H */
