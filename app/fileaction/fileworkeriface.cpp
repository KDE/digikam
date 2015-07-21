/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : database worker interface
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "fileworkeriface.moc"

// KDE includes

#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "fileactionmngr_p.h"
#include "imageattributeswatch.h"
#include "imageinfotasksplitter.h"
#include "scancontroller.h"
#include "globals.h"
#include "jpegutils.h"
#include "dimg.h"

#include "facetagseditor.h"

namespace Digikam
{

void FileActionMngrFileWorker::writeOrientationToFiles(FileActionImageInfoList infos, int orientation)
{
    QStringList failedItems;

    foreach(const ImageInfo& info, infos)
    {
        if (state() == WorkerObject::Deactivating)
        {
            break;
        }

        QString path                  = info.filePath();
        DMetadata metadata(path);
        DMetadata::ImageOrientation o = (DMetadata::ImageOrientation)orientation;
        metadata.setImageOrientation(o);

        if (!metadata.applyChanges())
        {
            failedItems.append(info.name());
        }
        else
        {
            emit imageDataChanged(path, true, true);
            KUrl url = KUrl::fromPath(path);
            ImageAttributesWatch::instance()->fileMetadataChanged(url);
        }

        infos.writtenToOne();
    }

    if (!failedItems.isEmpty())
    {
        emit imageChangeFailed(i18n("Failed to revise Exif orientation these files:"), failedItems);
    }

    infos.finishedWriting();
}

void FileActionMngrFileWorker::writeMetadataToFiles(FileActionImageInfoList infos)
{
    d->startingToWrite(infos);

    MetadataHub hub;

    ScanController::instance()->suspendCollectionScan();

    foreach(const ImageInfo& info, infos)
    {
        if (state() == WorkerObject::Deactivating)
        {
            break;
        }

        hub.load(info);
        QString filePath = info.filePath();

        ScanController::FileMetadataWrite writeScope(info);
        writeScope.changed(hub.write(filePath, MetadataHub::FullWrite));
        // hub emits fileMetadataChanged

        infos.writtenToOne();
    }

    ScanController::instance()->resumeCollectionScan();

    infos.finishedWriting();
}

void FileActionMngrFileWorker::writeMetadata(FileActionImageInfoList infos, MetadataHub* hub)
{
    d->startingToWrite(infos);

    MetadataSettingsContainer writeSettings = MetadataSettings::instance()->settings();

    ScanController::instance()->suspendCollectionScan();

    foreach(const ImageInfo& info, infos)
    {
        if (state() == WorkerObject::Deactivating)
        {
            break;
        }

        // apply to file metadata
        ScanController::FileMetadataWrite writeScope(info);
        writeScope.changed(hub->writeToMetadata(info, MetadataHub::FullWrite, writeSettings));
        // hub emits fileMetadataChanged

        infos.writtenToOne();
    }

    ScanController::instance()->resumeCollectionScan();

    delete hub;

    infos.finishedWriting();
}

void FileActionMngrFileWorker::transform(FileActionImageInfoList infos, int action)
{
    d->startingToWrite(infos);

    QStringList failedItems;
    ScanController::instance()->suspendCollectionScan();

    foreach(const ImageInfo& info, infos)
    {
        if (state() == WorkerObject::Deactivating)
        {
            break;
        }

        QString path                                = info.filePath();
        QString format                              = info.format();
        KExiv2::ImageOrientation currentOrientation = (KExiv2::ImageOrientation)info.orientation();
        bool isRaw                                  = info.format().startsWith(QLatin1String("RAW"));

        bool rotateAsJpeg     = false;
        bool rotateLossy      = false;

        MetadataSettingsContainer::RotationBehaviorFlags behavior;
        behavior = MetadataSettings::instance()->settings().rotationBehavior;

        bool rotateByMetadata = (behavior & MetadataSettingsContainer::RotateByMetadataFlag);

        // Check if rotation by content, as desired, is feasible
        // We'll later check again if it was successful
        if (behavior & MetadataSettingsContainer::RotatingPixels)
        {
            if (format == "JPG" && JPEGUtils::isJpegImage(path))
            {
                rotateAsJpeg = true;
            }

            if (behavior & MetadataSettingsContainer::RotateByLossyRotation)
            {
                DImg::FORMAT format = DImg::fileFormat(path);

                switch (format)
                {
                    case DImg::JPEG:
                    case DImg::PNG:
                    case DImg::TIFF:
                    case DImg::JP2K:
                    case DImg::PGF:
                        rotateLossy = true;
                    default:
                        break;
                }
            }
        }

        ajustFaceRectangles(info,action);

        KExiv2Iface::RotationMatrix matrix;
        matrix *= currentOrientation;
        matrix *= (KExiv2Iface::RotationMatrix::TransformationAction)action;
        KExiv2::ImageOrientation finalOrientation = matrix.exifOrientation();

        bool rotatedPixels = false;

        if (rotateAsJpeg)
        {
            JPEGUtils::JpegRotator rotator(path);
            rotator.setCurrentOrientation(currentOrientation);

            if (action == KExiv2Iface::RotationMatrix::NoTransformation)
            {
                rotatedPixels = rotator.autoExifTransform();
            }
            else
            {
                rotatedPixels = rotator.exifTransform((KExiv2Iface::RotationMatrix::TransformationAction)action);
            }

            if (!rotatedPixels)
            {
                failedItems.append(info.name());
            }
        }
        else if (rotateLossy)
        {
            // Non-JPEG image: DImg
            DImg image;

            if (!image.load(path))
            {
                failedItems.append(info.name());
            }
            else
            {
                if (action == KExiv2Iface::RotationMatrix::NoTransformation)
                {
                    image.rotateAndFlip(currentOrientation);
                }
                else
                {
                    image.transform(action);
                }

                // TODO: Atomic operation!!
                // prepare metadata, including to reset Exif tag
                image.prepareMetadataToSave(path, image.format(), true);

                if (image.save(path, image.detectedFormat()))
                {
                    rotatedPixels = true;
                }
                else
                {
                    failedItems.append(info.name());
                }
            }
        }

        if (rotatedPixels)
        {
            // reset for DB. Metadata is already edited.
            finalOrientation = KExiv2::ORIENTATION_NORMAL;
        }
        else if (rotateByMetadata)
        {
            // Setting the rotation flag on Raws with embedded JPEG is a mess
            // Can apply to the RAW data, or to the embedded JPEG, or to both.
            if (!isRaw)
            {
                DMetadata metadata(path);
                metadata.setImageOrientation(finalOrientation);
                metadata.applyChanges();
            }
        }

        // DB rotation
        ImageInfo(info).setOrientation(finalOrientation);

        if (!failedItems.contains(info.name()))
        {
            emit imageDataChanged(path, true, true);
            ImageAttributesWatch::instance()->fileMetadataChanged(info.fileUrl());
        }

        infos.writtenToOne();
    }

    if (!failedItems.isEmpty())
    {
        emit imageChangeFailed(i18n("Failed to transform these files:"), failedItems);
    }

    infos.finishedWriting();

    ScanController::instance()->resumeCollectionScan();
}

void FileActionMngrFileWorker::ajustFaceRectangles(const ImageInfo& info, int action)
{
    /**
     *  Get all faces from database and rotate them
     */
    QList<DatabaseFace> facesList = FaceTagsEditor().databaseFaces(info.id());

    QMap<QString, QRect> ajustedFaces;

    foreach(const DatabaseFace& dface, facesList)
    {
        QString name = FaceTags::faceNameForTag(dface.tagId());

        QRect oldrect = dface.region().toRect();

        if(action == 5) // TODO: use enum
        {
            QRect newRect      = TagRegion::ajustToRotatedImg(oldrect,info.dimensions(),0);
            ajustedFaces[name] = newRect;

        }

        if(action == 7) // TODO: use enum
        {
            QRect newRect      = TagRegion::ajustToRotatedImg(oldrect,info.dimensions(),1);
            ajustedFaces[name] = newRect;

        }
    }

    /**
     *  Delete all old faces and add rotated ones
     */
    FaceTagsEditor().removeAllFaces(info.id());

    QMap<QString,QRect>::ConstIterator it = ajustedFaces.constBegin();

    for( ;it!=ajustedFaces.constEnd();++it)
    {
        int tagId = FaceTags::getOrCreateTagForPerson(it.key());

        if (!tagId)
        {
            kDebug() << "Failed to create a person tag for name" << it.key();
        }

        TagRegion region(it.value());
        FaceTagsEditor().add(info.id(), tagId, region, false);
    }

    /** Write medatada **/

    MetadataHub hub;
    hub.load(info);
    QSize tempS = info.dimensions ();
    hub.loadFaceTags (info,QSize(tempS.height (),tempS.width ()));
    hub.write (info.filePath (),MetadataHub::FullWrite);
}

} // namespace Digikam
