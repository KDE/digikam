/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-08-14
 * Description : An unit test to load metadata from images through multi-core threads.
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
#include <QElapsedTimer>
#include <QSignalSpy>

// Local includes

#include "metaengine.h"
#include "digikam_globals.h"

class Q_DECL_HIDDEN Mytask : public ActionJob
{
public:

    Mytask()
        : ActionJob()
    {
    }

    QUrl    url;
    QString direction;

protected:

    void run()
    {
        MetaEngine meta;

        if (!meta.load(url.toLocalFile()))
        {
            qDebug() << url.fileName() << " : cannot load metadata!";
        }
        else
        {
            if (direction == QLatin1String("READ"))
            {
                 qDebug() << url.fileName() << " :: Dim: " << meta.getImageDimensions() 
                                            << " :: Dat: " << meta.getImageDateTime()
                                            << " :: Com: " << meta.getCommentsDecoded()
                                            << " :: Ori: " << meta.getImageOrientation()
                                            << " :: Col: " << meta.getImageColorWorkSpace();
            }
            else
            {
                 qDebug() << "Patch file: " << url.fileName();
                 meta.setImageProgramId(QLatin1String("digiKam"), QLatin1String("Exiv2"));
                 meta.applyChanges();
            } 
        }

        emit signalDone();
    }
};

// ----------------------------------------------------------------------------------------------------

MetaReaderThread::MetaReaderThread(QObject* const parent)
    : ActionThreadBase(parent)
{
    setObjectName(QLatin1String("MetaReaderThread"));
}

void MetaReaderThread::readMetadata(const QList<QUrl>& list, const QString& direction)
{
    ActionJobCollection collection;

    foreach (const QUrl& url, list)
    {
        Mytask* const job = new Mytask();
        job->url          = url;
        job->direction    = direction;
        collection.insert(job, 0);

        qDebug() << "Appending file to process " << url;
    }

    appendJobs(collection);
}

void MetaReaderThread::slotJobFinished()
{
    ActionThreadBase::slotJobFinished();

    if (isEmpty())
        emit done();
}

// ----------------------------------------------------------------------------------------------------

QTEST_MAIN(MetaReaderThreadTest)

void MetaReaderThreadTest::testMetaReaderThread()
{
    QString path = m_originalImageFolder;
    qDebug() << "Images path : " << path;

    QString filter;
    supportedImageMimeTypes(QIODevice::ReadOnly, filter);
    QStringList mimeTypes = filter.split(QLatin1Char(' ')); 

    qDebug() << "Images filters : " << mimeTypes;

    QString direction = QLatin1String("READ");

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
        QFAIL("Files list to process is empty!");
    }
    else
    {
        qDebug() << list.count() << "files to process...";
    }

    MetaReaderThread* const thread = new MetaReaderThread(this);
    thread->readMetadata(list, direction);

    QElapsedTimer timer;
    timer.start();

    QSignalSpy spy(thread, SIGNAL(done()));
    thread->start();

    QVERIFY(spy.wait(30000));

    qDebug() << "Reading metadata from " << list.size()
             << " files took " << timer.elapsed()/1000.0 << " seconds";
}
