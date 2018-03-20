/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-20
 * Description : description of actions when saving a file with versioning
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef VERSIONFILEOPERATION_H
#define VERSIONFILEOPERATION_H

// Qt includes

#include <QFlags>
#include <QMap>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "dimagehistory.h"

class QUrl;

namespace Digikam
{

class DIGIKAM_EXPORT VersionFileInfo
{
public:

    VersionFileInfo()
    {
    }

    VersionFileInfo(const QString& path, const QString& fileName, const QString& format)
        : path(path),
          fileName(fileName),
          format(format)
    {
    }

    bool    isNull() const;

    QString filePath() const;
    QUrl    fileUrl() const;

    QString path;
    QString fileName;
    QString format;
};

// -------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT VersionFileOperation
{
public:

    /** This class describes an operation necessary for storing an
     *  image under version control.
     *  The loadedFile and current history is given to the VersionManager.
     *  The saveFile is the destination of the save operation.
     *  If the loadedFile shall be moved to an intermediate,
     *  the name is given in intermediateForLoadedFile.
     *  The intermediates map may contain name of intermediates
     *  to save the state after action i of the history
     *  (initialResolvedHistory.size() <= i < currentHistory.size() - 1).
     */

    VersionFileOperation()
    {
    }

public:

    enum Task
    {
        /// saveFile is a new file. Excludes Replace.
        NewFile            = 1 << 0,
        /// loadedFile and saveFile are the same - replace. Excludes NewFile.
        Replace            = 1 << 1,
        /// Similar to Replace, but the new file name differs from the old one, which should be removed
        SaveAndDelete      = 1 << 2,
        /// Move loadedFile to loadedFileToIntermediate
        MoveToIntermediate = 1 << 3,
        /// Store additional snapshots from within history
        StoreIntermediates = 1 << 4
    };
    Q_DECLARE_FLAGS(Tasks, Task)

public:

    Tasks                      tasks;

    VersionFileInfo            loadedFile;

    VersionFileInfo            saveFile;

    VersionFileInfo            intermediateForLoadedFile;

    QMap<int, VersionFileInfo> intermediates;

    /**
     * Returns a list with all saving locations, for main result or intermediates
     */
    QStringList                allFilePaths() const;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::VersionFileOperation::Tasks)

#endif // VERSIONFILEOPERATION_H
