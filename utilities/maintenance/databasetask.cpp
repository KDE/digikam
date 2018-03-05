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
#include "maintenancedata.h"

namespace Digikam
{

class DatabaseTask::Private
{
public:

    Private()
        : scanThumbsDb(false),
          scanRecognitionDb(false),
          mode(Mode::Unknown),
          data(0)
    {
    }

    QString          objectIdentification;

    bool             scanThumbsDb;
    bool             scanRecognitionDb;

    Mode             mode;
    MaintenanceData* data;
};

// -------------------------------------------------------

DatabaseTask::DatabaseTask()
    : ActionJob(),
      d(new Private)
{
}

DatabaseTask::~DatabaseTask()
{
    cancel();
    delete d;
}

void DatabaseTask::computeDatabaseJunk(bool thumbsDb, bool facesDb)
{
    d->scanThumbsDb      = thumbsDb;
    d->scanRecognitionDb = facesDb;
}

void DatabaseTask::setMode(Mode mode)
{
    d->mode = mode;
}

void DatabaseTask::setMaintenanceData(MaintenanceData* const data)
{
    d->data = data;
}

void DatabaseTask::run()
{
    if (m_cancel)
    {
        return;
    }

    emit signalStarted();

    QThread::sleep(1);

    if (d->mode == Mode::ShrinkDatabases)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Shrinking databases";
        if (CoreDbAccess().db()->integrityCheck())
        {
            CoreDbAccess().db()->vacuum();

            if (!CoreDbAccess().db()->integrityCheck())
            {
                qCWarning(DIGIKAM_DATABASE_LOG) << "Integrity check for core DB failed after vacuum. Something went wrong.";
                // Signal that the database was vacuumed but failed the integrity check afterwards.
                emit signalFinished(true,false);
            }
            else
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "Finished vacuuming of core DB. Integrity check after vacuuming was positive.";
                emit signalFinished(true,true);
            }
        }
        else
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Integrity check for core DB failed. Will not vacuum.";
            // Signal that the integrity check failed and thus the vacuum was skipped
            emit signalFinished(false,false);
        }

        QThread::sleep(1);

        if (m_cancel)
        {
            return;
        }

        if (ThumbsDbAccess::isInitialized())
        {
            if (ThumbsDbAccess().db()->integrityCheck())
            {
                ThumbsDbAccess().db()->vacuum();

                if (!ThumbsDbAccess().db()->integrityCheck())
                {
                    qCWarning(DIGIKAM_DATABASE_LOG) << "Integrity check for thumbnails DB failed after vacuum. Something went wrong.";
                    // Signal that the database was vacuumed but failed the integrity check afterwards.
                    emit signalFinished(true,false);
                }
                else
                {
                    qCDebug(DIGIKAM_DATABASE_LOG) << "Finished vacuuming of thumbnails DB. Integrity check after vacuuming was positive.";
                    emit signalFinished(true,true);
                }
            }
            else
            {
                qCWarning(DIGIKAM_DATABASE_LOG) << "Integrity check for thumbnails DB failed. Will not vacuum.";
                // Signal that the integrity check failed and thus the vacuum was skipped
                emit signalFinished(false,false);
            }
        }
        else
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Thumbnails DB is not initialised. Will not vacuum.";
            emit signalFinished(false,false);
        }

        QThread::sleep(1);

        if (m_cancel)
        {
            return;
        }

        if (RecognitionDatabase().integrityCheck())
        {
            RecognitionDatabase().vacuum();

            if (!RecognitionDatabase().integrityCheck())
            {
                qCWarning(DIGIKAM_DATABASE_LOG) << "Integrity check for recognition DB failed after vacuum. Something went wrong.";
                // Signal that the database was vacuumed but failed the integrity check afterwards.
                emit signalFinished(true,false);
            }
            else
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "Finished vacuuming of recognition DB. Integrity check after vacuuming was positive.";
                emit signalFinished(true,true);
            }
        }
        else
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Integrity check for recognition DB failed. Will not vacuum.";
            // Signal that the integrity check failed and thus the vacuum was skipped
            emit signalFinished(false,false);
        }

        QThread::sleep(1);
    }
    else if (d->mode == Mode::ComputeDatabaseJunk)
    {
        QList<qlonglong>              staleImageIds;
        QList<int>                    staleThumbIds;
        QList<Identity>  staleIdentities;
        int additionalItemsToProcess = 0;

        // Get the count of image entries in DB to delete.
        staleImageIds   = CoreDbAccess().db()->getImageIds(DatabaseItem::Status::Obsolete);

        // get the count of items to process for thumbnails cleanup it enabled.
        if (d->scanThumbsDb && ThumbsDbAccess::isInitialized())
        {
            additionalItemsToProcess += CoreDbAccess().db()->getAllItems().size();
        }

        // get the count of items to process for identities cleanup it enabled.
        if (d->scanRecognitionDb)
        {
            additionalItemsToProcess += RecognitionDatabase().allIdentities().size();
        }

        if (additionalItemsToProcess > 0)
        {
            emit signalAddItemsToProcess(additionalItemsToProcess);
        }

        emit signalFinished();

        // Get the stale thumbnail paths.

        if (d->scanThumbsDb && ThumbsDbAccess::isInitialized())
        {
            // Thumbnails should be deleted, if the following conditions hold:
            // 1) The file path to which the thumb is assigned does not lead to an item
            // 2) The unique hash and file size are not used in core db for an item.
            // 3) The custom identifier does not exist in core db for an item.
            // OR
            // The thumbnail is stale, i.e. no thumbs db table references it.

            QSet<int> thumbIds     = ThumbsDbAccess().db()->findAll().toSet();

            // Get all items, i.e. images, videos, ...
            QList<qlonglong> items = CoreDbAccess().db()->getAllItems();

            FaceTagsEditor editor;

            foreach(qlonglong item, items)
            {
                if (m_cancel)
                {
                    return;
                }

                ImageInfo info(item);

                if (!info.isNull())
                {
                    QString hash       = CoreDbAccess().db()->getImagesFields(item,DatabaseFields::ImagesField::UniqueHash).first().toString();
                    qlonglong fileSize = info.fileSize();
                    bool removed       = false;

                    // Remove the id that is found by the file path. Finding the id -1 does no harm
                    removed            = thumbIds.remove(ThumbsDbAccess().db()->findByFilePath(info.filePath()).id);

                    if (!removed)
                    {
                        // Remove the id that is found by the hash and file size. Finding the id -1 does no harm
                        thumbIds.remove(ThumbsDbAccess().db()->findByHash(hash,fileSize).id);
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
                        thumbIds.remove(ThumbsDbAccess().db()->findByCustomIdentifier(url.toString()).id);
                    }
                }

                // Signal that this item was processed.
                emit signalFinished();
            }

            // The remaining thumbnail ids should be used to remove them since they are stale.
            staleThumbIds = thumbIds.toList();

            // Signal that the database was processed.
            emit signalFinished();
        }

        if (m_cancel)
        {
            return;
        }

        // Get the stale face identities.
        if (d->scanRecognitionDb)
        {
            QList<TagProperty> properties = CoreDbAccess().db()->getTagProperties(TagPropertyName::faceEngineUuid());
            QSet<QString> uuidSet;

            foreach(TagProperty prop, properties)
            {
                uuidSet << prop.value;
            }

            QList<Identity> identities = RecognitionDatabase().allIdentities();

            // Get all identitites to remove. Don't remove now in order to make sure no side effects occur.

            foreach(Identity identity, identities)
            {
                QString value = identity.attribute(QLatin1String("uuid"));

                if (!value.isEmpty() && !uuidSet.contains(value))
                {
                    staleIdentities << identity;
                }

                // Signal that this identity was processed.
                emit signalFinished();
            }

            // Signal that the database was processed.
            emit signalFinished();
        }

        emit signalData(staleImageIds,staleThumbIds,staleIdentities);
    }
    else if (d->mode == Mode::CleanCoreDb)
    {
        // While we have data (using this as check for non-null)
        while (d->data)
        {
            if (m_cancel)
            {
                return;
            }

            qlonglong imageId = d->data->getImageId();
            if (imageId == -1)
            {
                break;
            }

            CoreDbAccess().db()->deleteItem(imageId);
            CoreDbAccess().db()->removeImagePropertyByName(QLatin1String("similarityTo_")+QString::number(imageId));

            emit signalFinished();
        }
    }
    else if (d->mode == Mode::CleanThumbsDb)
    {
        BdEngineBackend::QueryState lastQueryState = BdEngineBackend::ConnectionError;

        // Connect to the database
        lastQueryState                             = ThumbsDbAccess().backend()->beginTransaction();

        if (BdEngineBackend::NoErrors == lastQueryState)
        {
            // Start removing.

            // While we have data (using this as check for non-null)
            while (d->data)
            {
                if (m_cancel)
                {
                    return;
                }

                int thumbId = d->data->getThumbnailId();

                if (thumbId == -1)
                {
                    break;
                }

                lastQueryState = ThumbsDbAccess().db()->remove(thumbId);
                emit signalFinished();
            }

            // Check for errors.

            if (BdEngineBackend::NoErrors == lastQueryState)
            {
                // Commit the removel if everything was fine.
                lastQueryState = ThumbsDbAccess().backend()->commitTransaction();

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
    else if (d->mode == Mode::CleanRecognitionDb)
    {
        // While we have data (using this as check for non-null)
        while (d->data)
        {
            if (m_cancel)
            {
                return;
            }

            Identity identity = d->data->getIdentity();

            if (identity.isNull())
            {
                break;
            }

            RecognitionDatabase().deleteIdentity(identity);
            emit signalFinished();
        }
    }

    emit signalDone();
}

} // namespace Digikam
