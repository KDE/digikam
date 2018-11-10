/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-08-14
 * Description : An unit test to read or write metadata through multi-core threads.
 *
 * Copyright (C) 2016-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metareaderthreadtest.h"

// Qt includes

#include <QDirIterator>
#include <QDebug>
#include <QApplication>
#include <QSignalSpy>
#include <QScopedPointer>
#include <QSettings>

// Local includes

#include "dmetadata.h"
#include "metaengine_previews.h"
#include "digikam_globals.h"

Mytask::Mytask()
    : ActionJob(),
      direction(MetaReaderThread::READ_INFO_FROM_FILE)
{
}

void Mytask::run()
{
    qDebug() << "Processing:" << url.path();
    bool processed = false;

    if (direction != MetaReaderThread::READ_PREVIEW_FROM_FILE)
    {
        QScopedPointer<DMetadata> meta(new DMetadata);
        meta->setSettings(settings);

        if (meta->load(url.toLocalFile()))
        {
            // NOTE: only process data here without to generate extra debug statements,
            //       for performance purpose.

            switch (direction)
            {
                case (MetaReaderThread::READ_INFO_FROM_FILE):
                {

                    // Get most important info used to populate the core-database
                    meta->getItemDimensions();
                    meta->getItemComments();
                    meta->getItemTitles();
                    meta->getCreatorContactInfo();
                    meta->getIptcCoreLocation();
                    meta->getIptcCoreSubjects();
                    meta->getPhotographInformation();
                    QStringList tmp;
                    meta->getItemTagsPath(tmp);
                    meta->getXmpKeywords();
                    meta->getXmpSubjects();
                    meta->getXmpSubCategories();
                    processed = true;
                    break;
                }
                default: // WRITE_INFO_TO_SIDECAR
                {
                    // Just create sidecar files with these info which will touch Exif, Iptc, and Xmp metadata
                    meta->setItemDimensions(QSize(256, 256));
                    meta->setImageDateTime(QDateTime::currentDateTime());
                    meta->setComments(QString::fromLatin1("MetaReaderThread").toLatin1());
                    meta->setItemRating(1);
                    meta->setItemPickLabel(2);
                    meta->setItemColorLabel(3);
                    meta->setItemTagsPath(QStringList() << QLatin1String("digiKam/Unit Tests/Metadata Engine/MetaReaderThread"));

                    // This stage will write a sidecar in temporary directory without to touch original file.
                    meta->save(tempDir + QString::fromUtf8("/%1").arg(url.path().remove(QLatin1Char('/'))));
                    processed = true;
                    break;
                }
            }
        }
    }
    else
    {
        MetaEnginePreviews previews(url.path());
        processed = !previews.isEmpty();
    }

    emit signalStats(url, processed);

    qDebug() << "Processed:" << url.path();

    emit signalDone();
}

// ----------------------------------------------------------------------------------------------------

MetaReaderThread::MetaReaderThread(QObject* const parent)
    : ActionThreadBase(parent)
{
    setObjectName(QLatin1String("MetaReaderThread"));
}

MetaReaderThread::~MetaReaderThread()
{
}

QString MetaReaderThread::directionToString(Direction direction)
{
    QString ret;

    switch (direction)
    {
        case (MetaReaderThread::READ_INFO_FROM_FILE):
             ret = QLatin1String("Read info from file");
             break;
        case (MetaReaderThread::READ_PREVIEW_FROM_FILE):
             ret = QLatin1String("Read preview from file");
             break;
        default: // WRITE_INFO_TO_SIDECAR
             ret = QLatin1String("Write info to side-car");
             break;
     }

     return ret;
}

void MetaReaderThread::readMetadata(const QList<QUrl>& list,
                                    Direction direction,
                                    const MetaEngineSettingsContainer& settings,
                                    const QString& temp)
{
    ActionJobCollection collection;

    foreach (const QUrl& url, list)
    {
        Mytask* const job = new Mytask();
        job->url          = url;
        job->direction    = direction;
        job->settings     = settings;
        job->tempDir      = temp;
        collection.insert(job, 0);

        connect(job, SIGNAL(signalStats(QUrl,bool)),
                this, SLOT(slotStats(QUrl,bool)));
    }

    appendJobs(collection);
    m_timer.start();
}

void MetaReaderThread::slotStats(const QUrl& url, bool p)
{
    m_stats.insertMulti(url, p);
}

void MetaReaderThread::slotJobFinished()
{
    ActionThreadBase::slotJobFinished();

    qDebug() << "Pending items to process:" << pendingCount();
    qDebug() << "Elaspsed time in seconds:" << m_timer.elapsed() / 1000.0;

    if (isEmpty())
        emit done();
}

QString MetaReaderThread::stats(const QStringList& mimeTypes)
{
    QString out;
    QStringList list;
    int count = 0;

    foreach (const QUrl& url, m_stats.keys())
    {
        list << QFileInfo(url.path()).suffix();
    }

    foreach (QString mt, mimeTypes)
    {
        mt.remove(QLatin1String("*."));
        count  = list.count(mt);
        count += list.count(mt.toUpper());

        if (count != 0)
        {
            out.append(QString::fromLatin1("%1(%2) ").arg(mt).arg(count));
        }
    }

    count = m_stats.values().count(false);
    out.append(QString::fromLatin1("Failed(%1 - %2%) ").arg(count).arg(count*100.0/m_stats.count()));

    return out;
}

// ----------------------------------------------------------------------------------------------------

QTEST_MAIN(MetaReaderThreadTest)

void MetaReaderThreadTest::testMetaReaderThread()
{
    MetaEngineSettingsContainer settings;
    settings.useXMPSidecar4Reading        = false;
    settings.metadataWritingMode          = DMetadata::WRITE_TO_SIDECAR_ONLY;

    QString filters;
    supportedImageMimeTypes(QIODevice::ReadOnly, filters);              // By defaults, all mime-types supported by digiKam.
    int threadsToUse                      = 0;                          // By default all cpu cores.
    QString path                          = m_originalImageFolder;      // By default the unit-tests data dir.

    // Read configuration from ~/.config/MetaReaderThreadTest.conf
    // Template file can be found at core/tests/metadataengine/data/

    QSettings conf(QLatin1String("MetaReaderThreadTest"));
    qDebug() << "Check custom configuration file" << conf.fileName();

    if (!QFileInfo(conf.fileName()).exists())
    {
        qDebug() << "Configuration file do not exists.";
        qDebug() << "You can customize this unit-test to copy the template file from";
        qDebug() << m_originalImageFolder << "in your home directory...";
    }
    else
    {
        bool useConf = conf.value(QLatin1String("Enable"), 0).toInt();

        // If configuration file is used, only one unit-test will be processed accordingly with the settings.

        if (useConf)
        {
            qDebug() << "We will use custom configuration file with this unit-test...";

            threadsToUse        = conf.value(QLatin1String("ThreadsToUse"), 0).toInt();
            QString confFilters = conf.value(QLatin1String("Filters"), QString()).toString();

            if (!confFilters.isEmpty())
            {
                filters = confFilters;
            }

            QString confPath = conf.value(QLatin1String("Path"), QString()).toString();

            if (!confPath.isEmpty())
            {
                path = confPath;
            }

            MetaReaderThread::Direction direction = (MetaReaderThread::Direction)conf.value(QLatin1String("Direction"),
                                                    (int)MetaReaderThread::NOT_DEFINED).toInt();

            if (direction == MetaReaderThread::NOT_DEFINED)
            {
                direction = MetaReaderThread::READ_INFO_FROM_FILE;
            }

            runMetaReader(path, filters.split(QLatin1Char(' ')), direction, settings, threadsToUse);

            return;
        }
    }

    // Standard unit-tests

    QStringList mimeTypes = filters.split(QLatin1Char(' '));

    runMetaReader(path, mimeTypes, MetaReaderThread::READ_INFO_FROM_FILE,    settings, threadsToUse);
    runMetaReader(path, mimeTypes, MetaReaderThread::READ_PREVIEW_FROM_FILE, settings, threadsToUse);
    runMetaReader(path, mimeTypes, MetaReaderThread::WRITE_INFO_TO_SIDECAR,  settings, threadsToUse);
}

void MetaReaderThreadTest::runMetaReader(const QString& path,
                                         const QStringList& mimeTypes,
                                         MetaReaderThread::Direction direction,
                                         const MetaEngineSettingsContainer& settings,
                                         int threadsToUse)
{
    qDebug() << "-- Start to process" << path << "------------------------------";

    QList<QUrl> list;
    QDirIterator it(path, mimeTypes,
                    QDir::Files,
                    QDirIterator::Subdirectories);

    while (it.hasNext())
    {
        QString path = it.next();
        list.append(QUrl::fromLocalFile(path));
    }

    if (list.isEmpty())
    {
        qDebug() << "Files list to process is empty!";
        return;
    }

    MetaReaderThread* const thread = new MetaReaderThread(this);

    if (threadsToUse > 0)
        thread->setMaximumNumberOfThreads(threadsToUse);

    thread->readMetadata(list, direction, settings, m_tempDir.absolutePath());

    QSignalSpy spy(thread, SIGNAL(done()));

    thread->start();

    while (!spy.wait(1000));  // Time-out in milliseconds

    qDebug() << endl << "Scan have been completed:"                                                    << endl
             <<         "    Processing duration:" << thread->m_timer.elapsed() / 1000.0 << " seconds" << endl
             <<         "    Root path          :" << path                                             << endl
             <<         "    Number of files    :" << list.size()                                      << endl
             <<         "    Number of threads  :" << thread->maximumNumberOfThreads()                 << endl
             <<         "    Direction          :" << MetaReaderThread::directionToString(direction)   << endl
             <<         "    Type-mimes         :" << mimeTypes.join(QLatin1Char(' '))                 << endl
             <<         "    Engine settings    :" << settings                                         << endl
             <<         "    Statistics         :" << thread->stats(mimeTypes)                         << endl;

    thread->cancel();
    delete thread;
}
