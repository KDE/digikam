/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-09
 * Description : resize image threads manager.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Andi Clemens <andi dot clemens at googlemail dot com>
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

#include "imageresizethread.h"

// Qt includes

#include <QFileInfo>
#include <QTemporaryDir>

// Local includes

#include "digikam_debug.h"
#include "imageresizejob.h"

namespace Digikam
{

ImageResizeThread::ImageResizeThread(QObject* const parent)
    : ActionThreadBase(parent)
{
    m_count  = new int;
    *m_count = 0;
}

ImageResizeThread::~ImageResizeThread()
{
    delete m_count;
}

void ImageResizeThread::resize(MailSettings* const settings)
{
    ActionJobCollection collection;
    *m_count = 0;
    int i    = 1;

    for (QMap<QUrl, QUrl>::const_iterator it = settings->itemsList.constBegin();
         it != settings->itemsList.constEnd(); ++it)
    {
        ImageResizeJob* const t = new ImageResizeJob(m_count);
        t->m_orgUrl   = it.key();
        t->m_settings = settings;

        QTemporaryDir tmpDir(t->m_settings->tempPath);
        tmpDir.setAutoRemove(false);

        QFileInfo fi(t->m_orgUrl.fileName());
        t->m_destName = tmpDir.path() + QLatin1Char('/') +
                        QString::fromUtf8("%1.%2").arg(fi.baseName()).arg(t->m_settings->format().toLower());

        connect(t, SIGNAL(startingResize(QUrl)),
                this, SIGNAL(startingResize(QUrl)));

        connect(t, SIGNAL(finishedResize(QUrl,QUrl,int)),
                this, SIGNAL(finishedResize(QUrl,QUrl,int)));

        connect(t, SIGNAL(failedResize(QUrl,QString,int)),
                this, SIGNAL(failedResize(QUrl,QString,int)));

        collection.insert(t, 0);
        i++;
    }

    appendJobs(collection);
}

void ImageResizeThread::cancel()
{
    *m_count   = 0;
    ActionThreadBase::cancel();
}

}  // namespace Digikam
