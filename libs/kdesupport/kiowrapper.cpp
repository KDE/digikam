/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-07-10
 * Description : A wrapper to isolate KIO Jobs calls
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "kiowrapper.h"

// Qt includes

#include <QUrl>

// KDE includes

#include <kio/statjob.h>
#include <kjobwidgets.h>
#include <kio/job.h>
#include <kio/global.h>
#include <kio/copyjob.h>
#include <kio/mkdirjob.h>
#include <kio/deletejob.h>

namespace Digikam
{

class KIOWrapperCreator
{
public:

    KIOWrapper object;
};

Q_GLOBAL_STATIC(KIOWrapperCreator, creator)

// ----------------------------------------------

KIOWrapper::KIOWrapper()
{
}

KIOWrapper* KIOWrapper::instance()
{
    return& creator->object;
}

QUrl KIOWrapper::mostLocalUrl(const QUrl& url)
{
    // If the given directory is not local, it can still be the URL of an
    // ioslave using UDS_LOCAL_PATH which to be converted first.
    KIO::StatJob* const job = KIO::mostLocalUrl(url, KIO::HideProgressInfo);
    job->exec();
    return job->mostLocalUrl();
}

QUrl KIOWrapper::upUrl(const QUrl &url)
{
    return KIO::upUrl(url);
}

bool KIOWrapper::fileCopy(const QUrl &src, const QUrl &dest, bool withKJobWidget, QWidget* widget)
{
    KIO::FileCopyJob* const fileCopyJob = KIO::file_copy(src, dest, KIO::Overwrite);

    if (withKJobWidget)
    {
        KJobWidgets::setWindow(fileCopyJob, widget);
    }

    return fileCopyJob->exec();
}

bool KIOWrapper::fileMove(const QUrl &src, const QUrl &dest)
{
    KIO::Job* const job = KIO::file_move(src, dest, -1,
                                         KIO::Overwrite | KIO::HideProgressInfo);

    return job->exec();
}

bool KIOWrapper::mkdir(const QUrl &url, bool withKJobWidget, QWidget *widget)
{
    KIO::Job* const job = KIO::mkdir(url);

    if (withKJobWidget)
    {
        KJobWidgets::setWindow(job, widget);
    }

    return job->exec();
}

bool KIOWrapper::rename(const QUrl &oldUrl, const QUrl &newUrl)
{
    KIO::Job* const job = KIO::rename(oldUrl, newUrl, KIO::HideProgressInfo);

    return job->exec();
}

void KIOWrapper::del(const QUrl &url)
{
    KIO::Job* const job = KIO::del(url);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(kioJobResult(KJob*)));
}

void KIOWrapper::trash(const QUrl &url)
{
    KIO::Job* const job = KIO::trash(url);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(kioJobResult(KJob*)) );
}

QString KIOWrapper::convertSizeFromKiB(quint64 KbSize)
{
    return KIO::convertSizeFromKiB(KbSize);
}

void KIOWrapper::kioJobResult(KJob *job)
{
    if (job->error() != 0)
    {
        emit error(job->errorString());
    }
    else
    {
        emit error(QString());
    }
}

} // namespace Digikam
