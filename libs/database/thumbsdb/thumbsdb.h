/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : Thumbnails database interface.
 *
 * Copyright (C)      2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef THUMBNAILS_DATABASE_H
#define THUMBNAILS_DATABASE_H

// Qt includes

#include <QString>
#include <QList>
#include <QStringList>
#include <QDateTime>
#include <QPair>
#include <QMap>
#include <QHash>

// Local includes

#include "dbenginesqlquery.h"
#include "thumbsdbbackend.h"
#include "thumbsdbaccess.h"
#include "digikam_export.h"

namespace Digikam
{

namespace DatabaseThumbnail
{

enum Type
{
    UndefinedType = 0,
    NoThumbnail,
    PGF,
    JPEG,              // Warning : no alpha channel support. Cannot be used as well.
    JPEG2000,
    PNG
    //FreeDesktopHash
};

} // namespace DatabaseThumbnail

class DIGIKAM_EXPORT ThumbsDbInfo
{

public:

    ThumbsDbInfo()
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

class DIGIKAM_EXPORT ThumbsDb
{

public:

    bool setSetting(const QString& keyword, const QString& value);
    QString getSetting(const QString& keyword);
    QString getLegacySetting(const QString& keyword);

    ThumbsDbInfo findByHash(const QString& uniqueHash, qlonglong fileSize);
    ThumbsDbInfo findByFilePath(const QString& path);
    ThumbsDbInfo findByCustomIdentifier(const QString& id);

    /** This is findByFilePath with extra security: Pass the uniqueHash which you have.
     *  If an entry is found by file path, and the entry is referenced by any uniqueHash,
     *  which is different from the given hash, a null info is returned.
     *  If uniqueHash is null, equivalent to the simple findByFilePath.
     */
    ThumbsDbInfo findByFilePath(const QString& path, const QString& uniqueHash);

    /** Returns the thumbnail ids of all thumbnails in the database.
     */
    QList<int> findAll();

    BdEngineBackend::QueryState insertUniqueHash(const QString& uniqueHash, qlonglong fileSize, int thumbId);
    BdEngineBackend::QueryState insertFilePath(const QString& path, int thumbId);
    BdEngineBackend::QueryState insertCustomIdentifier(const QString& id, int thumbId);

    BdEngineBackend::QueryState remove(int thumbId);
    /** Removes thumbnail data associated to the given uniqueHash/fileSize */
    BdEngineBackend::QueryState removeByUniqueHash(const QString& uniqueHash, qlonglong fileSize);
    /** Removes thumbnail data associated to the given file path */
    BdEngineBackend::QueryState removeByFilePath(const QString& path);
    BdEngineBackend::QueryState removeByCustomIdentifier(const QString& id);

    BdEngineBackend::QueryState insertThumbnail(const ThumbsDbInfo& info, QVariant* const lastInsertId = 0);
    BdEngineBackend::QueryState replaceThumbnail(const ThumbsDbInfo& info);

    QHash<QString, int> getFilePathsWithThumbnail();

    void replaceUniqueHash(const QString& oldUniqueHash, int oldFileSize, const QString& newUniqueHash, int newFileSize);
    BdEngineBackend::QueryState updateModificationDate(int thumbId, const QDateTime& modificationDate);

    //QStringList getAllThumbnailPaths();

    // ----------- Database shrinking methods ----------

    /**
     * Returns true if the integrity of the database is preserved.
     */
    bool integrityCheck();

    /**
     * Shrinks the database.
     */
    void vacuum();

private:

    explicit ThumbsDb(ThumbsDbBackend* const backend);
    ~ThumbsDb();

private:

    class Private;
    Private* const d;

    friend class ThumbsDbAccess;
};

}  // namespace Digikam

#endif /* THUMBNAILS_DATABASE_H */
