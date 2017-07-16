/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-07-27
 * Description : Special digiKam trash implementation
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "dtrash.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QUuid>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QDateTime>

// Local includes

#include "digikam_debug.h"
#include "collectionmanager.h"
#include "albummanager.h"

namespace Digikam
{

const QString DTrash::TRASH_FOLDER               = QLatin1String(".dtrash");
const QString DTrash::FILES_FOLDER               = QLatin1String("files");
const QString DTrash::INFO_FOLDER                = QLatin1String("info");
const QString DTrash::INFO_FILE_EXTENSION        = QLatin1String(".dtrashinfo");
const QString DTrash::PATH_JSON_KEY              = QLatin1String("path");
const QString DTrash::DELETIONTIMESTAMP_JSON_KEY = QLatin1String("deletiontimestamp");
const QString DTrash::IMAGEID_JSON_KEY           = QLatin1String("imageid");

// ----------------------------------------------

DTrash::DTrash()
{
}

bool DTrash::deleteImage(const QString& imageToDelete)
{
    QString collection = CollectionManager::instance()->albumRootPath(imageToDelete);

    qCDebug(DIGIKAM_IOJOB_LOG)  << "DTrash: Image album root path:"
                                << collection;

    if (!prepareCollectionTrash(collection))
    {
        return false;
    }

    QFileInfo imageFileInfo(imageToDelete);
    QFile imageFile(imageToDelete);

    QString fileName = imageFileInfo.fileName();
    // Get the album path, i.e. collection + album. For this,
    // get the n leftmost characters where n is the complete path without the size of the filename
    QString completePath = imageFileInfo.path();
    QString albumPath    = CollectionManager::instance()->album(completePath);

    qlonglong imageId = -1;
    // Get the album and with this the image id of the image to trash.
    PAlbum* pAlbum = AlbumManager::instance()->findPAlbum(QUrl::fromLocalFile(completePath));

    if (pAlbum)
    {
        imageId = AlbumManager::instance()->getItemFromAlbum(pAlbum,fileName);
    }

    QString baseNameForMovingIntoTrash = createJsonRecordForFile(collection, imageToDelete, imageId);

    QString destinationInTrash = collection + QLatin1Char('/') + TRASH_FOLDER +
                                 QLatin1Char('/') + FILES_FOLDER + QLatin1Char('/') +
                                 baseNameForMovingIntoTrash + QLatin1String(".") +
                                 imageFileInfo.completeSuffix();

    if (!imageFile.rename(destinationInTrash))
    {
        return false;
    }

    return true;
}

bool DTrash::deleteDirRecursivley(const QString& dirToDelete)
{
    QDir srcDir(dirToDelete);

    foreach (const QFileInfo& fileInfo, srcDir.entryInfoList(QDir::Files))
    {
        if (!deleteImage(fileInfo.filePath()))
        {
            return false;
        }
    }

    foreach (const QFileInfo& fileInfo, srcDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        deleteDirRecursivley(fileInfo.filePath());
    }

    return srcDir.removeRecursively();
}

void DTrash::extractJsonForItem(const QString& collPath, const QString& baseName, DTrashItemInfo& itemInfo)
{
    QString jsonFilePath = collPath + QLatin1Char('/') + TRASH_FOLDER +
                           QLatin1Char('/') + INFO_FOLDER + QLatin1Char('/') +
                           baseName + INFO_FILE_EXTENSION;

    QFile jsonFile(jsonFilePath);

    if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(jsonFile.readAll());
    jsonFile.close();

    QJsonObject fileInfoObj = doc.object();

    itemInfo.jsonFilePath = jsonFilePath;

    itemInfo.collectionPath = fileInfoObj.value(PATH_JSON_KEY).toString();

    itemInfo.collectionRelativePath = fileInfoObj.value(PATH_JSON_KEY).toString()
                                      .replace(collPath, QLatin1String(""));

    itemInfo.deletionTimestamp = QDateTime::fromString(
                                 fileInfoObj.value(DELETIONTIMESTAMP_JSON_KEY).toString());

    QJsonValue imageIdValue = fileInfoObj.value(IMAGEID_JSON_KEY);

    if (!imageIdValue.isUndefined())
    {
        itemInfo.imageId = imageIdValue.toString().toLongLong();
    }
    else
    {
        itemInfo.imageId = -1;
    }
}

bool DTrash::prepareCollectionTrash(const QString& collectionPath)
{
    QString trashFolder = collectionPath + QLatin1Char('/') + TRASH_FOLDER;
    QDir trashDir(trashFolder);

    if (!trashDir.exists())
    {
        bool isCreated = true;

        isCreated &= trashDir.mkpath(trashFolder);
        isCreated &= trashDir.mkpath(trashFolder + QLatin1Char('/') + FILES_FOLDER);
        isCreated &= trashDir.mkpath(trashFolder + QLatin1Char('/') + INFO_FOLDER);

        if (!isCreated)
        {
            qCDebug(DIGIKAM_IOJOB_LOG) << "DTrash: could not create trash folder for collection";
            return false;
        }
    }

    qCDebug(DIGIKAM_IOJOB_LOG) << "Trash folder for collection: " << trashFolder;

    return true;
}

QString DTrash::createJsonRecordForFile(const QString& collectionPath, const QString& imagePath, qlonglong imageId)
{
    QJsonObject jsonObjForImg;

    QJsonValue pathJsonVal(imagePath);
    QJsonValue timestampJsonVal(QDateTime::currentDateTime().toString());
    QJsonValue imageIdJsonVal(QString::number(imageId));

    jsonObjForImg.insert(PATH_JSON_KEY, pathJsonVal);
    jsonObjForImg.insert(DELETIONTIMESTAMP_JSON_KEY, timestampJsonVal);
    jsonObjForImg.insert(IMAGEID_JSON_KEY, imageIdJsonVal);

    QJsonDocument jsonDocForImg(jsonObjForImg);

    QFileInfo imgFileInfo(imagePath);

    QString jsonFileName = getAvialableJsonFilePathInTrash(collectionPath, imgFileInfo.baseName());

    QFile jsonFileForImg(jsonFileName);

    QFileInfo jsonFileInfo(jsonFileName);

    if (!jsonFileForImg.open(QFile::WriteOnly))
        return jsonFileInfo.baseName();

    jsonFileForImg.write(jsonDocForImg.toJson());
    jsonFileForImg.close();

    return jsonFileInfo.baseName();
}

QString DTrash::getAvialableJsonFilePathInTrash(const QString& collectionPath, const QString& baseName, int version)
{
    QString pathToCreateJsonFile = collectionPath + QLatin1Char('/') +
                                   TRASH_FOLDER + QLatin1Char('/') +
                                   INFO_FOLDER + QLatin1Char('/') +
                                   baseName + QLatin1Char('-') +
                                   QUuid::createUuid().toString().mid(1, 8) +
                                   (version ? QString::number(version) : QLatin1String("")) +
                                   INFO_FILE_EXTENSION;

    QFileInfo jsonFileInfo(pathToCreateJsonFile);

    if (jsonFileInfo.exists())
    {
        return getAvialableJsonFilePathInTrash(collectionPath, baseName, ++version);
    }
    else
    {
        return pathToCreateJsonFile;
    }
}

} // namespace Digikam
