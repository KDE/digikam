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

#ifndef DTRASH_H
#define DTRASH_H

#include <QObject>
#include <QFileInfo>

namespace Digikam
{

class DTrash
{

public:

    static DTrash* instance();

    bool deleteImage(const QString &imageToDelete);
    bool deleteDirRecursivley(const QString& dirToDelete);

private:

    /**
     * @brief Ensures that the trash caching folder exists before moving items to trash
     * @param collectionPath: the full path to the collection to prepare the trash for
     * @return true if the trash is prepared successfully
     */
    bool prepareCollectionTrash(const QString& collectionPath);

    /**
     * @brief Creates a json file containing the image path and deletion timestamp
     *        and return the baseName for this json file
     * @param collectionPath: Path of collection that the image belongs to
     * @param imagePath: path of image to create json file for
     *
     * @example createJsonRecordForFile("home/user/Pics", "/home/user/Pics/cats/cute/katy.jpg")
     *          returns => "katy"
     */
    QString createJsonRecordForFile(const QString& collectionPath, const QString& imagePath);

    /**
     * @brief Checks for duplicates of files names inside the trash and if there is
     *        a duplication it does a simple versioning recursively
     * @param collectionPath: Path of collection that the image belongs to
     * @param baseName: the baseName of the image
     * @param version: a digit to append to the image baseName
     *
     * @example getAvialableJsonFilePathInTrash("home/user/Pics", "katy", 0)
     *          return => "home/user/Pics/.trash/info/katy.dtrashInfo"
     *          if this name was available, if not
     *          returns => "home/user/Pics/.trash/info/katy{version}.dtrashInfo"
     *          where {version} is a integer number that is available in trash
     */
    QString getAvialableJsonFilePathInTrash(const QString& collectionPath, const QString& baseName, int version = 0);

private:

    friend class DTrashCreator;
    DTrash();
};

} // namespace Digikam

#endif // DTRASH_H
