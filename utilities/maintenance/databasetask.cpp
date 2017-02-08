/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-01-29
 * Description : Thread actions task for database cleanup.
 *
 * Copyright (C) 2017 by Mario Frank <mario dot frank at uni minus potsdam dot de>
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

#include "databasetask.h"

// Qt includes

#include <QUrlQuery>

// Local includes

#include "digikam_debug.h"
#include "imageinfo.h"
#include "thumbsdb.h"
#include "thumbsdbaccess.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "recognitiondatabase.h"
#include "facetagseditor.h"

namespace Digikam
{

class DatabaseTask::Private
{
public:

    Private():
        computeDatabaseJunk(false),
        scanThumbsDb(false),
        scanRecognitionDb(false),
        shrinkDatabases(false),
        cancel(false)
    {
    }

    QList<qlonglong>              imageIds;
    QList<int>                    thumbIds;
    QList<FacesEngine::Identity>  identities;

    QString                       objectIdentification;

    bool                          computeDatabaseJunk;
    bool                          scanThumbsDb;
    bool                          scanRecognitionDb;
    bool                          shrinkDatabases;

    bool                          cancel;
};

// -------------------------------------------------------

DatabaseTask::DatabaseTask()
    : ActionJob(),
      d(new Private)
{
}

DatabaseTask::~DatabaseTask()
{
    slotCancel();

    delete d;
}

void DatabaseTask::setItem(qlonglong imageId)
{
    setItems(QList<qlonglong>() << imageId);
}

void DatabaseTask::setItems(const QList<qlonglong>& imageIds)
{
    d->imageIds               = imageIds;
    d->objectIdentification   = QLatin1String("item id batch"); 
}

void DatabaseTask::setThumbId(int thumbId)
{
    setThumbIds(QList<int>() << thumbId);
}

void DatabaseTask::setThumbIds(const QList<int>& thumbIds)
{
    d->thumbIds               = thumbIds;
    d->objectIdentification   = QLatin1String("thumbnail id batch "); 
}

void DatabaseTask::setIdentity(const FacesEngine::Identity& identity)
{
    setIdentities(QList<FacesEngine::Identity>() << identity);
}

void DatabaseTask::setIdentities(const QList<FacesEngine::Identity>& identities)
{
    d->identities             = identities;
    d->objectIdentification   = QLatin1String("face identity batch"); 
}

void DatabaseTask::setShrinkJob()
{
    d->shrinkDatabases = true;
}

void DatabaseTask::computeDatabaseJunk(bool thumbsDb, bool facesDb)
{
    d->computeDatabaseJunk    = true;
    d->scanThumbsDb           = thumbsDb;
    d->scanRecognitionDb      = facesDb;
}

void DatabaseTask::slotCancel()
{
    d->cancel = true;
}

void DatabaseTask::run()
{
    if (d->shrinkDatabases)
    {
        if (CoreDbAccess().db()->integrityCheck())
        {
            CoreDbAccess().db()->vacuum();
            if (!CoreDbAccess().db()->integrityCheck())
            {
                qCWarning(DIGIKAM_DATABASE_LOG) << "Integrity check for core DB failed after vacuum. Something went wrong.";
            }
            else
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "Finished vacuuming of core DB. Integrity check after vacuuming was positive.";
            }
        }
        else
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Integrity check for core DB failed. Will not vacuum. Either you use MySQL which is currently not supported or your core DB is corrupt.";
        }

        if (ThumbsDbAccess::isInitialized())
        {
            if (ThumbsDbAccess().db()->integrityCheck())
            {
                ThumbsDbAccess().db()->vacuum();
                if (!ThumbsDbAccess().db()->integrityCheck())
                {
                    qCWarning(DIGIKAM_DATABASE_LOG) << "Integrity check for thumbnails DB failed after vacuum. Something went wrong.";
                }
                else
                {
                    qCDebug(DIGIKAM_DATABASE_LOG) << "Finished vacuuming of thumbnails DB. Integrity check after vacuuming was positive.";
                }
            }
            else
            {
                qCWarning(DIGIKAM_DATABASE_LOG) << "Integrity check for thumbnails DB failed. Will not vacuum. Either you use MySQL which is currently not supported or your thumbnails DB is corrupt.";
            }
        }
        else
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Thumbnails DB is not initialised. Will not vacuum.";
        }

        FacesEngine::RecognitionDatabase rDatabase;
        if (rDatabase.integrityCheck())
        {
            rDatabase.vacuum();
            if (!rDatabase.integrityCheck())
            {
                qCWarning(DIGIKAM_DATABASE_LOG) << "Integrity check for recognition DB failed after vacuum. Something went wrong.";
            }
            else
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "Finished vacuuming of recognition DB. Integrity check after vacuuming was positive.";
            }
        }
        else
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Integrity check for recognition DB failed. Will not vacuum. Either you use MySQL which is currently not supported or your core DB is corrupt.";
        }
    }
    else if (d->computeDatabaseJunk)
    {
        CoreDbAccess coreDbAccess;

        // Get the count of image entries in DB to delete.
        d->imageIds   = coreDbAccess.db()->getImageIds(DatabaseItem::Status::Obsolete);

        // Get the stale thumbnail paths.

        if (d->scanThumbsDb && ThumbsDbAccess::isInitialized())
        {
            // Thumbnails should be deleted, if the following conditions hold:
            // 1) The file path to which the thumb is assigned does not lead to an item
            // 2) The unique hash and file size are not used in core db for an item.
            // 3) The custom identifier does not exist in core db for an item.
            // OR
            // The thumbnail is stale, i.e. no thumbs db table references it.

            ThumbsDbAccess thumbsDbAccess;
            QSet<int> thumbIds     = thumbsDbAccess.db()->findAll().toSet();

            // Get all items, i.e. images, videos, ...
            QList<qlonglong> items = coreDbAccess.db()->getAllItems();

            FaceTagsEditor editor;

            foreach(qlonglong item, items)
            {
                ImageInfo info(item);

                if (!info.isNull())
                {
                    QString hash       = coreDbAccess.db()->getImagesFields(item,DatabaseFields::ImagesField::UniqueHash).first().toString();
                    qlonglong fileSize = info.fileSize();
                    bool removed       = false;

                    // Remove the id that is found by the file path. Finding the id -1 does no harm
                    removed            = thumbIds.remove(thumbsDbAccess.db()->findByFilePath(info.filePath()).id);

                    if (!removed)
                    {
                        // Remove the id that is found by the hash and file size. Finding the id -1 does no harm
                        thumbIds.remove(thumbsDbAccess.db()->findByHash(hash,fileSize).id);
                    }

                    // Add the custom identifier.
                    // get all faces for the image and generate the custom identifiers
                    QUrl url;
                    url.setScheme(QLatin1String("detail"));
                    url.setPath(info.filePath());
                    QList<FaceTagsIface> faces = editor.databaseFaces(item);

                    foreach(FaceTagsIface face, faces)
                    {
                        QRect rect = face.region().toRect();
                        QString r  = QString::fromLatin1("%1,%2-%3x%4").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
                        QUrlQuery q(url);

                        // Remove the previous query if existent.
                        q.removeQueryItem(QLatin1String("rect"));
                        q.addQueryItem(QLatin1String("rect"), r);
                        url.setQuery(q);

                        //qCDebug(DIGIKAM_GENERAL_LOG) << "URL: " << url.toString(); 

                        // Remove the id that is found by the custom identifyer. Finding the id -1 does no harm
                        thumbIds.remove(thumbsDbAccess.db()->findByCustomIdentifier(url.toString()).id);
                    }
                }
            }

            // The remaining thumbnail ids should be used to remove them since they are stale.
            d->thumbIds = thumbIds.toList();
        }

        // Get the stale face identities.
        if (d->scanRecognitionDb)
        {
            QList<TagProperty> properties = coreDbAccess.db()->getTagProperties(TagPropertyName::faceEngineUuid());
            QSet<QString> uuidSet;

            foreach(TagProperty prop, properties)
            {
                uuidSet << prop.value;
            }

            FacesEngine::RecognitionDatabase rDatabase;

            QList<FacesEngine::Identity> identities = rDatabase.allIdentities();

            // Get all identitites to remove. Don't remove now in order to make sure no side effects occur.

            foreach(FacesEngine::Identity identity, identities)
            {
                QString value = identity.attribute(QLatin1String("uuid"));

                if (!value.isEmpty() && !uuidSet.contains(value))
                {
                    d->identities << identity;
                }
            }
        }

        emit signalData(d->imageIds,d->thumbIds,d->identities);
        // do not emit the finished signal. We do not need it.
        // It would only trigger the advance slot of MaintenanceThread
    }
    else if (!d->imageIds.isEmpty())
    {
        CoreDbAccess access;

        foreach(qlonglong imageId, d->imageIds)
        {
            access.db()->deleteItem(imageId);
            access.db()->removeImagePropertyByName(QLatin1String("similarityTo_")+QString::number(imageId));
        
            emit signalFinished();
        }
    }
    else if (!d->thumbIds.empty())
    {
        ThumbsDbAccess access;
        BdEngineBackend::QueryState lastQueryState = BdEngineBackend::ConnectionError;

        // Connect to the database
        lastQueryState                             = access.backend()->beginTransaction();

        if (BdEngineBackend::NoErrors == lastQueryState)
        {
            // Start removing.

            foreach(int thumbId, d->thumbIds)
            {
                lastQueryState = access.db()->remove(thumbId);
                emit signalFinished();
            }

            // Check for errors.

            if (BdEngineBackend::NoErrors == lastQueryState)
            {
                // Commit the removel if everything was fine.
                lastQueryState = access.backend()->commitTransaction();

                if (BdEngineBackend::NoErrors != lastQueryState)
                {
                    qCWarning(DIGIKAM_THUMBSDB_LOG) << "Could not commit the removal of " << d->objectIdentification << " due to error ";
                }
            }
            else
            {
                qCWarning(DIGIKAM_THUMBSDB_LOG) << "Could not start the removal of " << d->objectIdentification << " due to error ";
            }
        }
        else
        {
            qCWarning(DIGIKAM_THUMBSDB_LOG) << "Could not begin the transaction for the removal of " << d->objectIdentification << " due to error ";
        }
    }
    else if (!d->identities.isEmpty())
    {
        foreach (FacesEngine::Identity identity, d->identities)
        {
            FacesEngine::RecognitionDatabase().deleteIdentity(identity);
            emit signalFinished();
        }
    }

    emit signalDone();
}

} // namespace Digikam
