/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date  : 2016-04-21
 * @brief : a class to manage video thumbnails extraction using threads
 *
 * @author Copyright (C) 2016 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
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

#include "loadvideothumbthread.h"

// Qt includes

#include <QFileInfo>
#include <QImage>
#include <QDebug>
#include <QEventLoop>

// Local includes

#include "loadvideothumb.h"

class Mytask : public ActionJob
{
public:

    Mytask()
        : ActionJob()
    {
    }

    QString           errString;
    QUrl              fileUrl;

protected:

    void run()
    {
        emit signalStarted();

        QImage image;

        if (m_cancel) return;

        emit signalProgress(20);

        VideoThumbnailer processor;

        if (m_cancel) return;

        emit signalProgress(40);

        processor.setThumbnailSize(256);
        processor.setCreateStrip(true);

        if (m_cancel) return;

        emit signalProgress(60);

        QEventLoop eventLoop;

        connect(&processor, &VideoThumbnailer::signalVideoThumbDone,
                &eventLoop, &QEventLoop::quit);

        if (m_cancel) return;

        emit signalProgress(80);

        processor.getThumbnail(fileUrl.toLocalFile());
        eventLoop.exec();

        emit signalDone();
    }
};

// ----------------------------------------------------------------------------------------------------

LoadVideoThumbThread::LoadVideoThumbThread(QObject* const parent)
    : ActionThreadBase(parent)
{
}

LoadVideoThumbThread::~LoadVideoThumbThread()
{
}

void LoadVideoThumbThread::extractVideoThumb(const QList<QUrl>& list)
{
    ActionJobCollection collection;

    foreach (const QUrl& url, list)
    {
        Mytask* const job = new Mytask();
        job->fileUrl      = url;

        connect(job, SIGNAL(signalStarted()),
                this, SLOT(slotJobStarted()));

        connect(job, SIGNAL(signalProgress(int)),
                this, SLOT(slotJobProgress(int)));

        connect(job, SIGNAL(signalDone()),
                this, SLOT(slotJobDone()));

        collection.insert(job, 0);

        qDebug() << "Appending file to process " << url;
    }

    appendJobs(collection);
}

void LoadVideoThumbThread::slotJobDone()
{
    Mytask* const task = dynamic_cast<Mytask*>(sender());
    if (!task) return;

    if (task->errString.isEmpty())
    {
        emit finished(task->fileUrl);
    }
    else
    {
        emit failed(task->fileUrl, task->errString);
    }
}

void LoadVideoThumbThread::slotJobProgress(int p)
{
    Mytask* const task = dynamic_cast<Mytask*>(sender());
    if (!task) return;

    emit progress(task->fileUrl, p);
}

void LoadVideoThumbThread::slotJobStarted()
{
    Mytask* const task = dynamic_cast<Mytask*>(sender());
    if (!task) return;

    emit starting(task->fileUrl);
}
