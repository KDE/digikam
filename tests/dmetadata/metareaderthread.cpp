/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-08-14
 * Description : CLI tool to test to load metadata from images through multi-core threads.
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

#include "metareaderthread.h"

// Qt includes

#include <QDirIterator>
#include <QDebug>
#include <QApplication>
#include <QElapsedTimer>

// Local includes

#include "metaengine.h"

class Mytask : public ActionJob
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

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    if (argc < 3)
    {
        qDebug() << "metareaderthread - test to load/save metadata from images through multi-core threads";
        qDebug() << "Usage  : <direction: READ | WRITE> <images path> <image file filter> ... <image file filter>";
        qDebug() << "Example: READ /mnt/photos *.jpg *.png *.tif *.nef *.dng";
        qDebug() << "Warning: Write direction will touch file matadata contents!";
        return -1;
    }

    QString direction = QString::fromLocal8Bit(argv[1]);

    if (direction != QLatin1String("READ") && direction == QLatin1String("WRITE"))
    {
        qDebug() << "Wrong direction type: " << direction;
        return -1;
    }

    QString path = QString::fromLocal8Bit(argv[2]);
    qDebug() << "Images path : " << path;
    QStringList filters;

    for (int i = 3 ; i < argc ; i++)
    {
        filters << QString::fromLocal8Bit(argv[i]);
    }

    if (filters.isEmpty())
    {
        qDebug() << "Image filters list is empty!";
        return -1;
    }

    qDebug() << "Images filters : " << filters;

    QList<QUrl> list;
    QDirIterator it(path, filters,
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
        return -1;
    }

    MetaEngine::initializeExiv2();

    MetaReaderThread* const thread = new MetaReaderThread(&app);
    thread->readMetadata(list, direction);

    QElapsedTimer timer;
    timer.start();

    thread->start();

    QObject::connect(thread, SIGNAL(done()),
                     &app, SLOT(quit()));

    app.exec();

    qDebug() << "Reading metadata from " << list.size() << " files took " << timer.elapsed()/1000.0 << " seconds";

    MetaEngine::cleanupExiv2();

    return 0;
}
