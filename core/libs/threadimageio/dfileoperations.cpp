/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : misc file operation methods
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "dfileoperations.h"

// C ANSI includes

#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>

// Qt includes

#include <QFileInfo>
#include <QByteArray>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QMimeType>
#include <QMimeDatabase>
#include <QDesktopServices>
#include <QFileInfo>

// KDE includes

#include <kmimetypetrader.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "metadatasettings.h"

namespace Digikam
{

bool DFileOperations::localFileRename(const QString& source,
                                      const QString& orgPath,
                                      const QString& destPath,
                                      bool ignoreSettings)
{
    QString dest = destPath;
    // check that we're not replacing a symlink
    QFileInfo info(dest);

    if (info.isSymLink())
    {
        dest = info.symLinkTarget();

        qCDebug(DIGIKAM_GENERAL_LOG) << "Target filePath"
                                     << QDir::toNativeSeparators(dest)
                                     << "is a symlink pointing to"
                                     << QDir::toNativeSeparators(dest)
                                     << ". Storing image there.";
    }

#ifndef Q_OS_WIN

    QByteArray dstFileName = QFile::encodeName(dest).constData();

    // Store old permissions:
    // Just get the current umask.
    mode_t curr_umask = umask(S_IREAD | S_IWRITE);
    // Restore the umask.
    umask(curr_umask);

    // For new files respect the umask setting.
    mode_t filePermissions = (S_IREAD | S_IWRITE | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP) & ~curr_umask;

    // For existing files, use the mode of the original file.
    struct stat stbuf;

    if (::stat(dstFileName.constData(), &stbuf) == 0)
    {
        filePermissions = stbuf.st_mode;
    }

#endif // Q_OS_WIN

    struct stat st;

    if (::stat(QFile::encodeName(source).constData(), &st) == 0)
    {
        // See bug #329608: Restore file modification time from original file only if updateFileTimeStamp for Setup/Metadata is turned off.

        if (!ignoreSettings && !MetadataSettings::instance()->settings().updateFileTimeStamp)
        {
            struct utimbuf ut;
            ut.modtime = st.st_mtime;
            ut.actime  = st.st_atime;

            if (::utime(QFile::encodeName(orgPath).constData(), &ut) != 0)
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to restore modification time for file "
                                               << dest;
            }
        }
    }

    // remove dest file if it exist
    if (orgPath != dest && QFile::exists(orgPath) && QFile::exists(dest))
    {
        QFile::remove(dest);
    }
    // rename tmp file to dest
    // QFile::rename() takes care of QString -> bytestring encoding
    if (!QFile::rename(orgPath, dest))
    {
        return false;
    }

#ifndef Q_OS_WIN

    // restore permissions
    if (::chmod(dstFileName.constData(), filePermissions) != 0)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to restore file permissions for file "
                                       << dstFileName;
    }

#endif // Q_OS_WIN

    return true;
}

void DFileOperations::openFilesWithDefaultApplication(const QList<QUrl>& urls)
{
    if (urls.isEmpty())
    {
        return;
    }

    foreach (const QUrl& url, urls)
    {
        QDesktopServices::openUrl(url);
    }
}

QUrl DFileOperations::getUniqueFileUrl(const QUrl& orgUrl,
                                       bool* const newurl)
{
    QUrl destUrl(orgUrl);

    if (newurl)
        *newurl = false;

    QFileInfo fi(destUrl.toLocalFile());

    if (fi.exists())
    {
        int i          = 0;
        bool fileFound = false;

        do
        {
            QFileInfo nfi(destUrl.toLocalFile());

            if (!nfi.exists())
            {
                fileFound = false;

                if (newurl)
                    *newurl = true;
            }
            else
            {
                destUrl = destUrl.adjusted(QUrl::RemoveFilename);
                destUrl.setPath(destUrl.path() + fi.completeBaseName() + QString::fromUtf8(" (%1).").arg(++i) + fi.completeSuffix());
                fileFound = true;
            }
        }
        while (fileFound);
    }

    return destUrl;
}

bool DFileOperations::runFiles(KService* const service,
                               const QList<QUrl>& urls)
{
    return (runFiles(service->exec(), urls, service));
}

bool DFileOperations::runFiles(const QString& appCmd,
                               const QList<QUrl>& urls,
                               KService* const service)
{
    QRegExp split(QLatin1String(" +(?=(?:[^\"]*\"[^\"]*\")*[^\"]*$)"));
    QStringList cmdList = appCmd.split(split, QString::SkipEmptyParts);
    QList<QUrl> urlList = urls;

    if (cmdList.isEmpty() || urlList.isEmpty())
    {
        return false;
    }

    if (!appCmd.contains(QLatin1String("%f"), Qt::CaseInsensitive) &&
        !appCmd.contains(QLatin1String("%u"), Qt::CaseInsensitive) &&
        !appCmd.contains(QLatin1String("%d"), Qt::CaseInsensitive))
    {
        cmdList << QLatin1String("%f");
    }

    QString exec;
    QString name;
    QString icon;
    QString term;

    QStringList dirs;
    QStringList files;
    QStringList cmdArgs;
    QStringList termOpts;

    bool useTerminal = false;
    bool openNewRun  = false;

    if (service)
    {
        name = service->desktopEntryName();
        icon = service->icon();

#ifdef Q_OS_LINUX
        if (service->terminal())
        {
            termOpts = service->terminalOptions().split(split, QString::SkipEmptyParts);
            term     = QStandardPaths::findExecutable(QLatin1String("konsole"));

            if (term.isEmpty())
            {
                term = QStandardPaths::findExecutable(QLatin1String("xterm"));
                termOpts.replaceInStrings(QLatin1String("--noclose"),
                                          QLatin1String("-hold"));
            }

            useTerminal = !term.isEmpty();
        }
#endif
    }

    QProcess* const process = new QProcess();
    QProcessEnvironment env = adjustedEnvironmentForAppImage();

    foreach(const QUrl& url, urlList)
    {
        dirs  << url.adjusted(QUrl::RemoveFilename).toLocalFile();
        files << url.toLocalFile();
    }

    foreach(const QString& cmdString, cmdList)
    {
        QString cmd = cmdString;

        if (cmd.startsWith(QLatin1Char('"')) && cmd.endsWith(QLatin1Char('"')))
        {
            cmd.remove(0, 1).chop(1);
        }

        if (exec.isEmpty() && cmd.contains(QLatin1Char('=')))
        {
            QStringList envList = cmd.split(QLatin1Char('='), QString::SkipEmptyParts);

            if (envList.count() == 2)
            {
                env.insert(envList[0], envList[1]);
            }

            continue;
        }
        else if (exec.isEmpty())
        {
            exec = cmd;
            continue;
        }

        if (cmd == QLatin1String("%c"))
            cmdArgs << name;
        else if (cmd == QLatin1String("%i"))
            cmdArgs << icon;
        else if (cmd == QLatin1String("%f"))
        {
            cmdArgs << files.first();
            openNewRun = true;
        }
        else if (cmd == QLatin1String("%F"))
            cmdArgs << files;
        else if (cmd == QLatin1String("%u"))
        {
            cmdArgs << files.first();
            openNewRun = true;
        }
        else if (cmd == QLatin1String("%U"))
            cmdArgs << files;
        else if (cmd == QLatin1String("%d"))
        {
            cmdArgs << dirs.first();
            openNewRun = true;
        }
        else if (cmd == QLatin1String("%D"))
            cmdArgs << dirs;
        else
            cmdArgs << cmd;
    }

    process->setProcessEnvironment(env);

    if (useTerminal)
    {
        termOpts << QLatin1String("-e") << exec << cmdArgs;
        process->start(term, termOpts);
    }
    else
    {
        process->start(exec, cmdArgs);
    }

    bool ret = true;
    ret     &= process->waitForStarted();

    if (openNewRun)
    {
        urlList.removeFirst();

        if (!urlList.isEmpty())
        {
            ret &= runFiles(appCmd, urlList, service);
        }
    }

    return ret;
}

KService::List DFileOperations::servicesForOpenWith(const QList<QUrl>& urls)
{
    // This code is inspired by KonqMenuActions:
    // kdebase/apps/lib/konq/konq_menuactions.cpp

    QStringList    mimeTypes;
    KService::List offers;

    foreach(const QUrl& item, urls)
    {
        const QString mimeType = QMimeDatabase().mimeTypeForFile(item.toLocalFile(), QMimeDatabase::MatchExtension).name();

        if (!mimeTypes.contains(mimeType))
        {
            mimeTypes << mimeType;
        }
    }

    if (!mimeTypes.isEmpty())
    {
        // Query trader
        const QString firstMimeType      = mimeTypes.takeFirst();
        const QString constraintTemplate = QLatin1String("'%1' in ServiceTypes");
        QStringList constraints;

        foreach(const QString& mimeType, mimeTypes)
        {
            constraints << constraintTemplate.arg(mimeType);
        }

        offers = KMimeTypeTrader::self()->query(firstMimeType, QLatin1String("Application"), constraints.join(QLatin1String(" and ")));

        // remove duplicate service entries
        QSet<QString> seenApps;

        for (KService::List::iterator it = offers.begin(); it != offers.end();)
        {
            const QString appName((*it)->name());

            if (!seenApps.contains(appName))
            {
                seenApps.insert(appName);
                ++it;
            }
            else
            {
                it = offers.erase(it);
            }
        }
    }

    return offers;
}

bool DFileOperations::copyFolderRecursively(const QString& srcPath,
                                            const QString& dstPath)
{
    QDir srcDir(srcPath);
    QString newCopyPath = dstPath + QLatin1Char('/') + srcDir.dirName();

    if (!srcDir.mkpath(newCopyPath))
    {
        return false;
    }

    foreach (const QFileInfo& fileInfo, srcDir.entryInfoList(QDir::Files))
    {
        QString copyPath = newCopyPath + QLatin1Char('/') + fileInfo.fileName();

        if (!QFile::copy(fileInfo.filePath(), copyPath))
            return false;
    }

    foreach (const QFileInfo& fileInfo, srcDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        if (!copyFolderRecursively(fileInfo.filePath(), newCopyPath))
            return false;
    }

    return true;
}

bool DFileOperations::copyFiles(const QStringList& srcPaths,
                                const QString& dstPath)
{
    foreach (const QString& path, srcPaths)
    {
        QFileInfo fileInfo(path);
        QString copyPath = dstPath + QLatin1Char('/') + fileInfo.fileName();

        if (!QFile::copy(fileInfo.filePath(), copyPath))
            return false;
    }

    return true;
}

} // namespace Digikam
