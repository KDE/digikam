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
#include <QElapsedTimer>
#include <QScopedPointer>
#include <QSettings>

// Local includes

#include "dmetadata.h"
#include "digikam_globals.h"

class Q_DECL_HIDDEN Mytask : public ActionJob
{
public:

    Mytask()
        : ActionJob(),
          direction(MetaReaderThread::READ_FROM_FILE)
    {
    }

    QUrl                        url;
    MetaReaderThread::Direction direction;
    MetaEngineSettingsContainer settings;

protected:

    void run()
    {
        qDebug() << "Processing file: " << url;

        {
            QScopedPointer<DMetadata> meta(new DMetadata);
            meta->setSettings(settings);

            if (!meta->load(url.toLocalFile()))
            {
                // TODO handle this case with stats
            }
            else
            {
                // NOTE : only process data here without to generate debug statements

                if (direction == MetaReaderThread::READ_FROM_FILE)
                {
                    // Get most important info used to populate the database
                    meta->getImageDimensions();
                    meta->getImageTitles();
                    meta->getCreatorContactInfo();
                    meta->getIptcCoreLocation();
                    meta->getIptcCoreSubjects();
                    meta->getPhotographInformation();
                    meta->getVideoInformation();
                    meta->getXmpKeywords();
                    meta->getXmpSubjects();
                    meta->getXmpSubCategories();
                }
                else // WRITE_TO_FILE
                {
                    // Just patch file with this info which will touch Exif, Iptc, and Xmp metadata
                    meta->setImageProgramId(QLatin1String("digiKam"), QLatin1String("Exiv2"));
                    meta->applyChanges();
                }
            }
        }

        qDebug() << "Processed file: " << url;

        emit signalDone();
    }
};

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
    return (direction == MetaReaderThread::READ_FROM_FILE) ? QLatin1String("Read from files")
                                                           : QLatin1String("Write to files");
}

void MetaReaderThread::readMetadata(const QList<QUrl>& list,
                                    Direction direction,
                                    const MetaEngineSettingsContainer& settings)
{
    ActionJobCollection collection;

    foreach (const QUrl& url, list)
    {
        Mytask* const job = new Mytask();
        job->url          = url;
        job->direction    = direction;
        job->settings     = settings;
        collection.insert(job, 0);
    }

    appendJobs(collection);
}

void MetaReaderThread::slotJobFinished()
{
    ActionThreadBase::slotJobFinished();

    qDebug() << "Pending items to process:" << pendingCount();

    if (isEmpty())
        emit done();
}

// ----------------------------------------------------------------------------------------------------

QTEST_MAIN(MetaReaderThreadTest)

void MetaReaderThreadTest::testMetaReaderThread()
{
    // Read configuration from ~/.config/MetaReaderThreadTest.conf
    // Template file can be found at core/tests/metadataengine/data/

    QSettings conf(QLatin1String("MetaReaderThreadTest"));
    qDebug() << "Read configuration from" << conf.fileName();
    qDebug() << conf.allKeys();

    bool useConf    = conf.value(QLatin1String("Enable"), 0).toInt();

    if (useConf)
    {
        qDebug() << "We will use configuration file with this unit-test...";
    }

    QString filters = useConf ? conf.value(QLatin1String("Filters"), QString()).toString() : QString();

    if (filters.isEmpty())
    {
        supportedImageMimeTypes(QIODevice::ReadOnly, filters);
    }

    QStringList mimeTypes          = filters.split(QLatin1Char(' '));

    MetaEngineSettingsContainer settings;
    settings.useXMPSidecar4Reading = false;
    settings.metadataWritingMode   = DMetadata::WRITE_TO_SIDECAR_ONLY;

    QString path = useConf ? conf.value(QLatin1String("Path"), QString()).toString() : QString();

    if (path.isEmpty())
    {
        path = m_originalImageFolder;
    }

    MetaReaderThread::Direction direction = useConf ?
                                            (MetaReaderThread::Direction)conf.value(QLatin1String("Direction"), (int)MetaReaderThread::NOT_DEFINED).toInt()
                                            : MetaReaderThread::NOT_DEFINED;

    if (direction == MetaReaderThread::NOT_DEFINED)
    {
        direction = MetaReaderThread::READ_FROM_FILE;
    }

    runMetaReader(path, mimeTypes, direction, settings);
}

void MetaReaderThreadTest::runMetaReader(const QString& path,
                                         const QStringList& mimeTypes,
                                         MetaReaderThread::Direction direction,
                                         const MetaEngineSettingsContainer& settings)
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
    thread->readMetadata(list, direction, settings);
    QSignalSpy spy(thread, SIGNAL(done()));
    QElapsedTimer timer;
    timer.start();

    thread->start();

    QVERIFY(spy.wait(3*1000*1000));  // Time-out in milliseconds

    thread->cancel();
    delete thread;

    qDebug() << endl << "MetaReader have been completed:"                                            << endl
             <<         "    Processing duration:" << timer.elapsed() / 1000.0 << " seconds"         << endl
             <<         "    Root path          :" << path                                           << endl
             <<         "    Number of files    :" << list.size()                                    << endl
             <<         "    Direction          :" << MetaReaderThread::directionToString(direction) << endl
             <<         "    Type-mimes         :" << mimeTypes.join(QLatin1Char(' '))               << endl
             <<         "    Engine settings    :" << settings                                       << endl;
}
