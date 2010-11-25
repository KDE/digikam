/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-18
 * Description : class for determining new file name in terms of version management
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef VERSIONMANAGER_H
#define VERSIONMANAGER_H

// Qt includes

#include <QFlags>
#include <QMap>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "dimagehistory.h"

class KConfigGroup;
class KUrl;

namespace Digikam
{

class DIGIKAM_EXPORT VersionManagerSettings
{
public:

    VersionManagerSettings();

    void readFromConfig(KConfigGroup& group);
    void writeToConfig(KConfigGroup& group) const;

    enum IntermediateSavepoints
    {
        NoIntermediates  = 0,
        AfterEachSession = 1 << 0,
        WhenNecessary    = 1 << 1
    };
    Q_DECLARE_FLAGS(IntermediateBehavior, IntermediateSavepoints)

    bool                 showAllVersions;
    IntermediateBehavior saveIntermediateVersions;
    QString              formatForStoringRAW;
    QString              formatForStoringSubversions;
};

class DIGIKAM_EXPORT VersionNamingScheme
{
public:

    virtual ~VersionNamingScheme() {}
    virtual QString baseName(const QString& path, const QString& filename) = 0;
    virtual QString versionFileName(const QString& path, const QString& filename,
                                    const QString& format, const QVariant& counter) = 0;
    virtual QString intermediateFileName(const QString& path, const QString& filename,
                                         const QString& format, const QVariant& version, const QVariant& counter) = 0;
    virtual QString directory(const QString& path, const QString& filename) = 0;
    virtual QVariant initialCounter() = 0;
    virtual QVariant incrementedCounter(const QVariant& counter) = 0;
};

class DIGIKAM_EXPORT VersionFileInfo
{
public:

    VersionFileInfo() {}
    VersionFileInfo(const QString& path, const QString& fileName, const QString& format)
        : path(path), fileName(fileName), format(format) {}

    QString filePath() const;
    KUrl fileUrl() const;

    QString path;
    QString fileName;
    QString format;
};

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
     *  (initialResolvedHistory.actionCount() <= i < currentHistory.actionCount() - 1).
     */

    VersionFileOperation() {}

    enum Task
    {
        /// saveFile is a new file. Excludes Replace.
        NewFile = 1 << 0,
        /// loadedFile and saveFile are the same - replace. Excludes NewFile.
        Replace = 1 << 1,
        /// Move loadedFile to loadedFileToIntermediate
        MoveToIntermediate = 1 << 2,
        /// Store additional snapshots from within history
        StoreIntermediates = 1 << 3
    };
    Q_DECLARE_FLAGS(Tasks, Task)

    Tasks           tasks;

    VersionFileInfo loadedFile;
    DImageHistory   currentHistory;

    VersionFileInfo saveFile;

    VersionFileInfo intermediateForLoadedFile;

    QMap<int,VersionFileInfo> intermediates;
};

class DIGIKAM_EXPORT VersionManager
{
public:

    VersionManager();
    virtual ~VersionManager();

    void setSettings(const VersionManagerSettings& settings);
    VersionManagerSettings settings() const;

    void setNamingScheme(VersionNamingScheme* scheme);
    VersionNamingScheme* namingScheme() const;

    enum FileNameType
    {
        CurrentVersionName,
        NewVersionName
    };

    VersionFileOperation operation(FileNameType request, const VersionFileInfo& loadedFile,
                                   const DImageHistory& initialResolvedHistory,
                                   const DImageHistory& currentHistory);

    virtual QString toplevelDirectory(const QString& path);

    virtual QStringList workspaceFileFormats() const;

private:

    class VersionManagerPriv;
    VersionManagerPriv* const d;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::VersionManagerSettings::IntermediateBehavior)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::VersionFileOperation::Tasks)

#endif // VERSIONMANAGER_H
