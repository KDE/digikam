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

// Qt includes

#include <QFileInfo>
#include <QDir>
#include <QFile>

// KDE includes

#include <KConfigGroup>
#include <KDiskFreeSpaceInfo>
#include <KDebug>
#include <KGlobal>
#include <KUrl>

// Local includes

#include "versionmanager.h"

namespace Digikam
{

VersionManagerSettings::VersionManagerSettings()
{
    showAllVersions             = true;
    saveIntermediateVersions    = NoIntermediates;
    formatForStoringRAW         = "JPG";
    formatForStoringSubversions = "JPG";
}

void VersionManagerSettings::readFromConfig(KConfigGroup& group)
{
    showAllVersions             = group.readEntry("Show All Available Versions", true);
    int save                    = group.readEntry("Save Intermediate Versions", (int)NoIntermediates);
    saveIntermediateVersions    = IntermediateBehavior(save);
    formatForStoringRAW         = group.readEntry("Format For Storing Versions Of RAW Images", QString("JPG"));
    formatForStoringSubversions = group.readEntry("Format For Storing Other Versions Of Files", QString("JPG"));
}

void VersionManagerSettings::writeToConfig(KConfigGroup& group) const
{
    group.writeEntry("Show All Available Versions", showAllVersions);
    group.writeEntry("Save Intermediate Versions", (int)saveIntermediateVersions);
    group.writeEntry("Format For Storing Versions Of RAW Images", formatForStoringRAW);
    group.writeEntry("Format For Storing Other Versions Of Files", formatForStoringSubversions);
}

// --------------

class DefaultVersionNamingScheme : public VersionNamingScheme
{
public:

    virtual QString baseName(const QString& currentPath, const QString& filename);
    virtual QString versionFileName(const QString& currentPath, const QString& filename,
                                    const QString& format, const QVariant& counter);
    virtual QString intermediateFileName(const QString& currentPath, const QString& filename,
                                         const QString& format, const QVariant& version, const QVariant& counter);
    virtual QString directory(const QString& currentPath, const QString& filename);
    virtual QVariant initialCounter();
    virtual QVariant incrementedCounter(const QVariant& counter);
};

QVariant DefaultVersionNamingScheme::initialCounter()
{
    // start with _v1
    return 1;
}

QVariant DefaultVersionNamingScheme::incrementedCounter(const QVariant& counter)
{
    return counter.toInt() + 1;
}

QString DefaultVersionNamingScheme::baseName(const QString& currentPath, const QString& fileName)
{
    Q_UNUSED(currentPath);
    // Perl: /^(.+?)(_v\d+?)?(-\d+?)?\.([^\.]+?)$/
    // But setMinimal() cannot replace Perl's non-greedy quantifiers, so we need three regexps

    // DSC000636_v5-3.JPG
    QRegExp versionIntermediate("(.+)(_v\\d+)(-\\d+)\\.([^\\.]+)");
    if (versionIntermediate.exactMatch(fileName))
        return versionIntermediate.cap(1);

    // DSC000636_v5.JPG
    QRegExp version("(.+)(_v\\d+)\\.([^\\.]+)");
    if (version.exactMatch(fileName))
        return version.cap(1);

    // DSC000636.JPG
    QRegExp basename("(.+)\\.([^\\.]+)");
    if (basename.exactMatch(fileName))
        return basename.cap(1);

    // failure: no file suffix?
    // DSC000636
    return fileName;
}

QString DefaultVersionNamingScheme::versionFileName(const QString& currentPath, const QString& fileName,
                                                    const QString& format, const QVariant& counter)
{
    Q_UNUSED(currentPath);
    return QString("%1_v%2.%3").arg(fileName).arg(counter.toInt()).arg(format.toLower());
}

QString DefaultVersionNamingScheme::intermediateFileName(const QString& currentPath, const QString& fileName,
                                                         const QString& format, const QVariant& version,
                                                         const QVariant& counter)
{
    Q_UNUSED(currentPath);
    return QString("%1_v%2-%3.%4").arg(fileName).arg(version.toInt()).arg(counter.toInt()).arg(format.toLower());
}

QString DefaultVersionNamingScheme::directory(const QString& currentPath, const QString& fileName)
{
    Q_UNUSED(fileName);
    return currentPath;
}

// --------------

class VersionNameCreator
{
public:

    VersionNameCreator(const VersionFileInfo& loadedFile,
                       const DImageHistory& m_resolvedInitialHistory, const DImageHistory& m_currentHistory,
                       VersionManager *q);

    void checkNeedNewVersion();
    void fork();
    void setSaveDirectory();
    void setSaveFormat();
    void setSaveFileName();
    void nextIntermediateName();

public:

    VersionFileInfo m_result;
    VersionFileInfo m_loadedFile;
    VersionFileInfo m_intermediate;

    const DImageHistory m_resolvedInitialHistory;
    const DImageHistory m_currentHistory;

    bool m_fromRaw;
    bool m_newVersion;

    QVariant        m_version;
    QVariant        m_intermediateCounter;

    VersionManager* const q;
};

VersionNameCreator::VersionNameCreator(const VersionFileInfo& loadedFile,
                                       const DImageHistory& m_resolvedInitialHistory, const DImageHistory& m_currentHistory,
                                       VersionManager *q)
        : m_loadedFile(loadedFile),
          m_resolvedInitialHistory(m_resolvedInitialHistory), m_currentHistory(m_currentHistory),
          m_fromRaw(false), m_newVersion(false), q(q)
{
    m_loadedFile.format = m_loadedFile.format.toUpper();

    m_fromRaw = (m_loadedFile.format == "RAW");

    m_version = q->namingScheme()->initialCounter();
    m_intermediateCounter = q->namingScheme()->initialCounter();
}

void VersionNameCreator::checkNeedNewVersion()
{
    // First we check if we have any other files available.
    // The resolved initial history contains only referred files found in the collection
    // Note: The loaded file will have type Current
    kDebug() << m_resolvedInitialHistory.hasReferredImageOfType(HistoryImageId::Original)
    << m_resolvedInitialHistory.hasReferredImageOfType(HistoryImageId::Intermediate)
    << m_fromRaw << q->workspaceFileFormats().contains(m_loadedFile.format);
    if (!m_resolvedInitialHistory.hasReferredImageOfType(HistoryImageId::Original)
        && !m_resolvedInitialHistory.hasReferredImageOfType(HistoryImageId::Intermediate))
    {
        m_newVersion = true;
    }
    // We check the loaded format: If it is not one of the workspace formats, or even raw, we need a new version
    else if (m_fromRaw || !q->workspaceFileFormats().contains(m_loadedFile.format))
    {
        m_newVersion = true;
    }
    else
    {
        m_newVersion = false;
    }
}

void VersionNameCreator::fork()
{
    m_newVersion = true;
}

void VersionNameCreator::setSaveDirectory()
{
    m_result.path = q->namingScheme()->directory(m_loadedFile.path, m_loadedFile.fileName);
}

void VersionNameCreator::setSaveFormat()
{
    if (m_fromRaw)
        m_result.format = q->settings().formatForStoringRAW;
    else
        m_result.format = q->settings().formatForStoringSubversions;
}

void VersionNameCreator::setSaveFileName()
{
    kDebug() << "need new version" << m_newVersion;
    if (!m_newVersion)
    {
        m_result.fileName = m_loadedFile.fileName;
    }
    else
    {
        VersionNamingScheme *scheme = q->namingScheme();

        QString baseName = scheme->baseName(m_loadedFile.path, m_loadedFile.fileName);
        QDir dirInfo(m_loadedFile.path);

        // To find the right number for the new version, go through all the items in the given dir,
        // the version number won't be bigger than count()
        QVariant counter = scheme->initialCounter();
        for (uint i = 0; i <= dirInfo.count(); i++)
        {
            QString suggestedName = scheme->versionFileName(m_loadedFile.path, baseName, m_result.format, counter);

            if (!dirInfo.exists(suggestedName))
            {
                m_result.fileName = suggestedName;
                m_version = counter;
                break;
            }

            counter = scheme->incrementedCounter(counter);
        }
    }
}

void VersionNameCreator::nextIntermediateName()
{
    VersionNamingScheme *scheme = q->namingScheme();

    m_intermediate.path   = m_result.path;
    m_intermediate.format = m_result.format;

    QString baseName = scheme->baseName(m_loadedFile.path, m_loadedFile.fileName);
    QDir dirInfo(m_loadedFile.path);

    for (uint i = 0; i <= dirInfo.count(); i++)
    {
        QString suggestedName = scheme->intermediateFileName(m_loadedFile.path, baseName,
                                                             m_result.format, m_version, m_intermediateCounter);

        if (!dirInfo.exists(suggestedName))
        {
            m_intermediate.fileName = suggestedName;
            break;
        }

        m_intermediateCounter = scheme->incrementedCounter(m_intermediateCounter);
    }
}

// --------------

QString VersionFileInfo::filePath() const
{
    return path + "/" + fileName;
}

KUrl VersionFileInfo::fileUrl() const
{
    KUrl url = KUrl::fromPath(path);
    url.addPath(fileName);
    return url;
}

// --------------

class VersionManager::VersionManagerPriv
{
public:

    VersionManagerPriv()
    {
        scheme = 0;
    }

    VersionManagerSettings  settings;
    VersionNamingScheme    *scheme;

    DefaultVersionNamingScheme defaultScheme;
};

/*
    bool hasFreeSpace(const QString& path, int assumedFileSize) const;
bool VersionManager::VersionManagerPriv::hasFreeSpace(const QString& path, int assumedFileSize) const
{
    KDiskFreeSpaceInfo diskInfo = KDiskFreeSpaceInfo::freeSpaceInfo(originalPath);
    return diskInfo.isValid() && diskInfo.available() > (uint)assumedFileSize * 2;
}
*/

// --------------

VersionManager::VersionManager()
              : d(new VersionManagerPriv)
{
}

VersionManager::~VersionManager()
{
    delete d->scheme;
    delete d;
}

void VersionManager::setSettings(const VersionManagerSettings& settings)
{
    d->settings = settings;
}

VersionManagerSettings VersionManager::settings() const
{
    return d->settings;
}

void VersionManager::setNamingScheme(VersionNamingScheme *scheme)
{
    d->scheme = scheme;
}

VersionNamingScheme *VersionManager::namingScheme() const
{
    if (d->scheme)
        return d->scheme;
    else
        return &d->defaultScheme;
}

VersionFileOperation VersionManager::operation(FileNameType request, const VersionFileInfo& loadedFile,
                                               const DImageHistory& initialResolvedHistory,
                                               const DImageHistory& currentHistory)
{
    VersionNameCreator name(loadedFile, initialResolvedHistory, currentHistory, this);
    if (request == CurrentVersionName)
        name.checkNeedNewVersion();
    else if (request == NewVersionName)
        name.fork();
    name.setSaveDirectory();
    name.setSaveFormat();
    name.setSaveFileName();

    VersionFileOperation operation;
    operation.loadedFile = loadedFile;
    operation.currentHistory = currentHistory;
    operation.saveFile = name.m_result;

    if (name.m_newVersion)
    {
        operation.tasks |= VersionFileOperation::NewFile;
    }
    else
    {
        operation.tasks |= VersionFileOperation::Replace;
        if (d->settings.saveIntermediateVersions & VersionManagerSettings::AfterEachSession)
        {
            operation.tasks |= VersionFileOperation::MoveToIntermediate;
            name.nextIntermediateName();
            operation.intermediateForLoadedFile = name.m_intermediate;
        }
    }

    /* TODO: Implement WhenNecessary / StoreIntermediates */

    return operation;
}

QString VersionManager::toplevelDirectory(const QString& path)
{
    Q_UNUSED(path);
    return "/";
}

QStringList VersionManager::workspaceFileFormats() const
{
    QStringList formats;
    formats << "JPEG" << "PNG" << "TIFF" << "PGF";
    QString f = d->settings.formatForStoringRAW.toUpper();
    if (!formats.contains(f))
        formats << f;
    f = d->settings.formatForStoringSubversions.toUpper();
    if (!formats.contains(f))
        formats << f;
    return formats;
}

} // namespace Digikam
