/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate jAlbum image galleries
 *
 * Copyright (C) 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "jalbumgenerator.h"

// Qt includes

#include <QDir>
#include <QFutureWatcher>
#include <QRegExp>
#include <QStringList>
#include <QtConcurrentMap>
#include <QApplication>
#include <QProcess>
#include <QUrl>
#include <QList>
#include <QTemporaryFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "jalbumsettings.h"
#include "jalbumwizard.h"
#include "dfileoperations.h"

namespace DigikamGenericJAlbumPlugin
{

class Q_DECL_HIDDEN JAlbumGenerator::Private
{
public:

    explicit Private()
      : that(0),
        settings(0),
        warnings(false),
        cancel(false),
        pview(0),
        pbar(0)
    {
    }

    JAlbumGenerator* that;
    JAlbumSettings*  settings;
    QList<QUrl>      urls;

    // State settings
    bool              warnings;

    bool              cancel;
    DHistoryView*     pview;
    DProgressWdg*     pbar;

public:

    bool init()
    {
        cancel = false;

        pview->setVisible(true);
        pbar->setVisible(true);

        return true;
    }

    bool createDir(const QString& dirName)
    {
        logInfo(i18n("Create directories"));

        if (!QDir().mkpath(dirName))
        {
            logError(i18n("Could not create folder '%1'", QDir::toNativeSeparators(dirName)));
            return false;
        }

        return true;
    }

    bool createUrlsList()
    {
        if (settings->m_getOption == JAlbumSettings::ALBUMS)
        {
            // Loop over albums selection

            DInfoInterface::DAlbumIDs::ConstIterator albumIt  = settings->m_albumList.constBegin();
            DInfoInterface::DAlbumIDs::ConstIterator albumEnd = settings->m_albumList.constEnd();

            for ( ; albumIt != albumEnd ; ++albumIt)
            {
                int id = *albumIt;

                // Gather image element list
                QList<QUrl> imageList;

                if (settings->m_iface)
                {
                    imageList = settings->m_iface->albumsItems(DInfoInterface::DAlbumIDs() << id);
                    urls.append(imageList);
                }
            }
        }
        else
        {
            urls = settings->m_imageList;
        }

        return true;
    }

    bool createProjectFiles(const QString& projDir)
    {
        logInfo(i18n("Create jAlbum project files"));

        QDir newAlbumDir = QDir(projDir);

        QFile createFile(newAlbumDir.filePath(QString::fromLatin1("albumfiles.txt")));

        if (!createFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            logInfo(i18n("Failed to create project files"));
            return false;
        }

        QTextStream out(&createFile);

        for (QList<QUrl>::ConstIterator it = urls.constBegin(); it != urls.constEnd(); ++it)
        {
            out << (*it).fileName().toLocal8Bit().data() << "\t" << (*it).path().toLocal8Bit().data() << "\n";
        }

        createFile.close();

        QFile settingsFile(newAlbumDir.filePath(QString::fromLatin1("jalbum-settings.jap")));

        if (!settingsFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            logInfo(i18n("Failed to create settings file"));
            return false;
        }

        QTextStream out2(&settingsFile);
        out2 << "#jAlbum Project\n";

        settingsFile.close();

        return true;
    }

    bool launchJalbum(const QString& projDir, const QString& jarDir, const QString& javaExecutable)
    {
        logInfo(i18n("Launch jAlbum with new project files"));

/*
        QString javaExecutable;
        QDir jrePath = QFileInfo(jarDir).dir();

        if (jrePath.cd(QString::fromLatin1("jre64/bin/")))
        {
            javaExecutable = jrePath.filePath(QString::fromLatin1("java"));
        }
        else
        {
            javaExecutable = QString::fromLatin1("java");
        }
*/

        QDir newAlbumDir = QDir(projDir);

        QStringList args;
        args.append(QString::fromLatin1("-Xmx400M"));
        args.append(QString::fromLatin1("-jar"));
        args.append(jarDir);
        args.append(QDir::toNativeSeparators(newAlbumDir.filePath(QString::fromLatin1("jalbum-settings.jap"))));

        QProcess process;
        process.setProcessEnvironment(adjustedEnvironmentForAppImage());
        process.startDetached(javaExecutable, args);

        return true;
    }

    void logInfo(const QString& msg)
    {
        pview->addEntry(msg, DHistoryView::ProgressEntry);
    }

    void logError(const QString& msg)
    {
        pview->addEntry(msg, DHistoryView::ErrorEntry);
    }

    void logWarning(const QString& msg)
    {
        pview->addEntry(msg, DHistoryView::WarningEntry);
        warnings = true;
    }
};

// ----------------------------------------------------------------------

JAlbumGenerator::JAlbumGenerator(JAlbumSettings* const settings)
    : QObject(),
      d(new Private)
{
    d->that     = this;
    d->settings = settings;
    d->warnings = false;

    connect(this, SIGNAL(logWarningRequested(QString)),
            SLOT(logWarning(QString)), Qt::QueuedConnection);
}

JAlbumGenerator::~JAlbumGenerator()
{
    delete d;
}

bool JAlbumGenerator::run()
{
    if (!d->init())
        return false;

    QString destDir = d->settings->m_destPath;
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << destDir;

    QString javaDir = d->settings->m_javaPath;
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << javaDir;

    QString jarDir  = d->settings->m_jalbumPath;
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << jarDir;

    QString projDir = destDir + QString::fromLatin1("/") + d->settings->m_imageSelectionTitle;
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << projDir;

    if (!d->createDir(projDir))
        return false;

    bool result = d->createUrlsList();

    if (result)
    {
        result = d->createProjectFiles(projDir);
    }

    if (result)
    {
        result = d->launchJalbum(projDir, jarDir, javaDir);
    }

    return result;
}

bool JAlbumGenerator::warnings() const
{
    return d->warnings;
}

void JAlbumGenerator::logWarning(const QString& text)
{
    d->logWarning(text);
}

void JAlbumGenerator::slotCancel()
{
    d->cancel = true;
}

void JAlbumGenerator::setProgressWidgets(DHistoryView* const pView, DProgressWdg* const pBar)
{
    d->pview = pView;
    d->pbar  = pBar;

    connect(d->pbar, SIGNAL(signalProgressCanceled()),
            this, SLOT(slotCancel()));
}

} // namespace DigikamGenericJAlbumPlugin
