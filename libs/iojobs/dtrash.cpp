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
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QDateTime>

// Local includes
#include "digikam_debug.h"
#include "collectionmanager.h"

namespace Digikam
{

class DTrashCreator
{
public:

    DTrash object;
};

Q_GLOBAL_STATIC(DTrashCreator, creator)

// ----------------------------------------------

DTrash::DTrash()
{
}

DTrash* DTrash::instance()
{
    return& creator->object;
}

bool DTrash::deleteImage(const QUrl& imageToDelete)
{
    QString collection = CollectionManager::instance()->albumRootPath(imageToDelete.path());

    qCDebug(DIGIKAM_IOJOB_LOG)  << "DTrash: Image album root path:"
                                << collection;

    if (!prepareCollectionTrash(collection))
    {
        return false;
    }

    QString baseNameForMovingIntoTrash = createJsonRecordForFile(collection, imageToDelete.path());

    QFileInfo imageFileInfo(imageToDelete.path());
    QFile imageFile(imageToDelete.path());

    QString destinationInTrash = collection + QDir::separator() + QLatin1String(".trash") +
                                 QDir::separator() + QLatin1String("files") + QDir::separator() +
                                 baseNameForMovingIntoTrash + QLatin1String(".") +
                                 imageFileInfo.completeSuffix();

    if (!imageFile.rename(destinationInTrash))
    {
        return false;
    }

    return true;
}

bool DTrash::prepareCollectionTrash(const QString& collectionPath)
{
    QString trashFolder = collectionPath + QDir::separator() + QLatin1String(".trash");
    QDir trashDir(trashFolder);

    if (!trashDir.exists())
    {
        bool isCreated = true;

        isCreated &= trashDir.mkpath(trashFolder);
        isCreated &= trashDir.mkpath(trashFolder + QDir::separator() + QLatin1String("files"));
        isCreated &= trashDir.mkpath(trashFolder + QDir::separator() + QLatin1String("info"));

        if(!isCreated)
        {
            qCDebug(DIGIKAM_IOJOB_LOG) << "DTrash: could not create trash folder for collection";
            return false;
        }
    }

    qCDebug(DIGIKAM_IOJOB_LOG) << "Trash folder for collection: " << trashFolder;

    return true;
}

QString DTrash::createJsonRecordForFile(const QString& collectionPath, const QString& imagePath)
{
    QJsonObject jsonObjForImg;

    QJsonValue pathJsonVal(imagePath);
    QJsonValue timestampJsonVal(QDateTime::currentDateTime().toString());

    jsonObjForImg.insert(QLatin1String("path"), pathJsonVal);
    jsonObjForImg.insert(QLatin1String("deletionTimestamp"), timestampJsonVal);

    QJsonDocument jsonDocForImg(jsonObjForImg);

    QFileInfo imgFileInfo(imagePath);

    QString jsonFileName = getAvialableFilePathInTrash(collectionPath, imgFileInfo.baseName());

    QFile jsonFileForImg(jsonFileName);
    jsonFileForImg.open(QFile::WriteOnly);
    jsonFileForImg.write(jsonDocForImg.toJson());
    jsonFileForImg.close();

    QFileInfo jsonFileInfo(jsonFileName);
    return jsonFileInfo.baseName();
}

QString DTrash::getAvialableJsonFilePathInTrash(const QString& collectionPath, const QString& baseName, int version)
{
    QString pathToCreateJsonFile = collectionPath + QDir::separator() +
                                   QLatin1String(".trash") + QDir::separator() +
                                   QLatin1String("info") + QDir::separator() +
                                   baseName +
                                   (version ? QString::number(version) : QLatin1String("")) +
                                   QLatin1String(".dtrashInfo");

    QFileInfo jsonFileInfo(pathToCreateJsonFile);

    if (jsonFileInfo.exists())
    {
        return getAvialableFilePathInTrash(collectionPath, baseName, ++version);
    }
    else
    {
        return pathToCreateJsonFile;
    }
}

} // namespace Digikam
