/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-12-28
 * Description : a class to manage Raw to Png conversion using threads
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2014 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "myactionthread.h"

// Qt includes

#include <QFileInfo>
#include <QImage>
#include <QDebug>

// Local includes

#include "drawdecoder.h"

class Mytask : public ActionJob
{
public:

    Mytask()
        : ActionJob()
    {
    }

    DRawDecoderSettings settings;
    QString             errString;
    QUrl                fileUrl;

protected:

    void run()
    {
        emit signalStarted();

        QImage image;

        if (m_cancel) return;

        emit signalProgress(20);

        DRawDecoder rawProcessor;

        if (m_cancel) return;

        emit signalProgress(30);

        QFileInfo input(fileUrl.toLocalFile());
        QString   fullFilePath(input.baseName() + QString::fromLatin1(".full.png"));
        QFileInfo fullOutput(fullFilePath);

        if (m_cancel) return;

        emit signalProgress(40);

        if (!rawProcessor.loadFullImage(image, fileUrl.toLocalFile(), settings))
        {
            errString = QString::fromLatin1("raw2png: Loading full RAW image failed. Aborted...");
            return;
        }

        if (m_cancel) return;

        emit signalProgress(60);

        qDebug() << "raw2png: Saving full RAW image to "
                 << fullOutput.fileName() << " size ("
                 << image.width() << "x" << image.height()
                 << ")";

        if (m_cancel) return;

        emit signalProgress(80);

        image.save(fullFilePath, "PNG");

        emit signalDone();
    }
};

// ----------------------------------------------------------------------------------------------------

MyActionThread::MyActionThread(QObject* const parent)
    : ActionThreadBase(parent)
{
}

MyActionThread::~MyActionThread()
{
}

void MyActionThread::convertRAWtoPNG(const QList<QUrl>& list, const DRawDecoderSettings& settings, int priority)
{
    ActionJobCollection collection;

    foreach (const QUrl& url, list)
    {
        Mytask* const job = new Mytask();
        job->fileUrl      = url;
        job->settings     = settings;

        connect(job, SIGNAL(signalStarted()),
                this, SLOT(slotJobStarted()));

        connect(job, SIGNAL(signalProgress(int)),
                this, SLOT(slotJobProgress(int)));

        connect(job, SIGNAL(signalDone()),
                this, SLOT(slotJobDone()));

        collection.insert(job, priority);

        qDebug() << "Appending file to process " << url;
    }

    appendJobs(collection);
}

void MyActionThread::slotJobDone()
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

void MyActionThread::slotJobProgress(int p)
{
    Mytask* const task = dynamic_cast<Mytask*>(sender());
    if (!task) return;

    emit progress(task->fileUrl, p);
}

void MyActionThread::slotJobStarted()
{
    Mytask* const task = dynamic_cast<Mytask*>(sender());
    if (!task) return;

    emit starting(task->fileUrl);
}
