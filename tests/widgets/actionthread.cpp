/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-10-17
 * Description : test for implementation of dimagelist api
 *
 * Copyright (C)      2014 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * Copyright (C) 2014-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "actionthread.h"

// Qt includes

#include <QImage>
#include <QTransform>
#include <QDebug>

class Task : public ActionJob
{
public:

    Task()
        : ActionJob()
    {
    }

    QString errString;
    QUrl    fileUrl;

protected:

    void run()
    {
        emit signalStarted();

        QImage src;
        QImage dst;

        if (m_cancel)
            return;

        emit signalProgress(20);

        if (!src.load(fileUrl.toLocalFile()))
        {
            errString = QLatin1String("Loading image failed. Aborted...");
            return;
        }

        if (m_cancel)
            return;

        emit signalProgress(40);

        QTransform transform;
        transform.rotate(90);

        if (m_cancel)
            return;

        emit signalProgress(60);

        dst = src.transformed(transform);

        if (m_cancel)
            return;

        emit signalProgress(80);

        if (!dst.save(fileUrl.toLocalFile()))
        {
            errString = QLatin1String("Saving image failed. Aborted...");
            return;
        }

        emit signalDone();
    }
};

// ----------------------------------------------------------------------------------------------------

ActionThread::ActionThread(QObject* const parent)
    : ActionThreadBase(parent)
{
}

ActionThread::~ActionThread()
{
}

void ActionThread::rotate(const QList<QUrl>& list)
{
    ActionJobCollection collection;

    foreach (const QUrl& url, list)
    {
        Task* const job = new Task();
        job->fileUrl    = url;

        connect(job, &Task::signalStarted,
                this, &ActionThread::slotJobStarted);

        connect(job, &Task::signalProgress,
                this, &ActionThread::slotJobProgress);

        connect(job, &Task::signalDone,
                this, &ActionThread::slotJobDone);

        collection.insert(job, 0);

        qDebug() << "Appending file to process " << url;
    }

    appendJobs(collection);
}

void ActionThread::slotJobDone()
{
    Task* const task = dynamic_cast<Task*>(sender());
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

void ActionThread::slotJobProgress(int p)
{
    Task* const task = dynamic_cast<Task*>(sender());
    if (!task) return;

    emit progress(task->fileUrl, p);
}

void ActionThread::slotJobStarted()
{
    Task* const task = dynamic_cast<Task*>(sender());
    if (!task) return;

    emit starting(task->fileUrl);
}
