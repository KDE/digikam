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

#include <QPair>
#include <QPointer>

// KDE includes

#include <kio/statjob.h>
#include <kjobwidgets.h>
#include <kio/job.h>
#include <kio/global.h>
#include <kio/copyjob.h>
#include <kio/mkdirjob.h>
#include <kio/deletejob.h>
#include <kio/previewjob.h>
#include <krun.h>
#include <kio_version.h>
#include <KIOWidgets/kio/renamedialog.h>

namespace Digikam
{

KIOWrapper::KIOWrapper()
{
}

QUrl KIOWrapper::mostLocalUrl(const QUrl& url)
{
    // If the given directory is not local, it can still be the URL of an
    // ioslave using UDS_LOCAL_PATH which to be converted first.
    KIO::StatJob* const job = KIO::mostLocalUrl(url, KIO::HideProgressInfo);
    job->exec();
    return job->mostLocalUrl();
}

QUrl KIOWrapper::upUrl(const QUrl& url)
{
    return KIO::upUrl(url);
}

bool KIOWrapper::fileCopy(const QUrl& src, const QUrl& dest, bool withKJobWidget, QWidget* const widget)
{
    KIO::FileCopyJob* const fileCopyJob = KIO::file_copy(src, dest, KIO::Overwrite);

    if (withKJobWidget)
    {
        KJobWidgets::setWindow(fileCopyJob, widget);
    }

    return fileCopyJob->exec();
}

bool KIOWrapper::fileMove(const QUrl& src, const QUrl& dest)
{
    KIO::Job* const job = KIO::file_move(src, dest, -1,
                                         KIO::Overwrite | KIO::HideProgressInfo);

    return job->exec();
}

bool KIOWrapper::fileDelete(const QUrl& url)
{
    KIO::SimpleJob* const job = KIO::file_delete(url);
    return job->exec();
}

bool KIOWrapper::mkdir(const QUrl& url, bool withKJobWidget, QWidget* const widget)
{
    KIO::Job* const job = KIO::mkdir(url);

    if (withKJobWidget)
    {
        KJobWidgets::setWindow(job, widget);
    }

    return job->exec();
}

bool KIOWrapper::rename(const QUrl& oldUrl, const QUrl& newUrl)
{
    KIO::Job* const job = KIO::rename(oldUrl, newUrl, KIO::HideProgressInfo);

    return job->exec();
}

void KIOWrapper::move(const QUrl& src, const QUrl& dest)
{
    KIO::Job* const job = KIO::move(src, dest);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(kioJobResult(KJob*)));
}

void KIOWrapper::del(const QUrl& url)
{
    KIO::Job* const job = KIO::del(url);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(kioJobResult(KJob*)));
}

void KIOWrapper::trash(const QUrl& url)
{
    KIO::Job* const job = KIO::trash(url);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(kioJobResult(KJob*)) );
}

QString KIOWrapper::convertSizeFromKiB(quint64 KbSize)
{
    return KIO::convertSizeFromKiB(KbSize);
}

QStringList KIOWrapper::previewJobAvailablePlugins()
{
    return KIO::PreviewJob::availablePlugins();
}

void KIOWrapper::filePreview(const QList<QUrl>& urlList, const QSize& size, const QStringList* const enabledPlugins)
{
    KFileItemList items;

    for (QList<QUrl>::ConstIterator it = urlList.constBegin() ; it != urlList.constEnd() ; ++it)
    {
        if ((*it).isValid())
            items.append(KFileItem(*it));
    }

    KIO::PreviewJob* const job = KIO::filePreview(items, size, enabledPlugins);

    connect(job, SIGNAL(gotPreview(KFileItem,QPixmap)),
            this, SLOT(gotPreview(KFileItem,QPixmap)));

    connect(job, SIGNAL(failed(KFileItem)),
            this, SLOT(previewJobFailed(KFileItem)));

    connect(job, SIGNAL(finished(KJob*)),
            this, SIGNAL(previewJobFinished()));
}

void KIOWrapper::kioJobResult(KJob* job)
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

void KIOWrapper::previewJobFailed(const KFileItem& item)
{
    emit previewJobFailed(item.url());
}

void KIOWrapper::gotPreview(const KFileItem& item, const QPixmap& pix)
{
    emit gotPreview(item.url(), pix);
}

QPair<int, QString> KIOWrapper::renameDlg(QWidget* widget, const QString& caption, const QUrl& src, const QUrl& dest)
{
    QPointer<KIO::RenameDialog> dlg = new KIO::RenameDialog(widget, caption,
                                                            src, dest,
                                                            KIO::RenameDialog_Mode(KIO::M_MULTI |
                                                            KIO::M_OVERWRITE |
                                                            KIO::M_SKIP));
    QPair<int, QString> pair;
    pair.first  = dlg->exec();
    pair.second = dlg->newDestUrl().toLocalFile();

    delete dlg;

    return pair;
}

bool KIOWrapper::run(const KService& service, const QList<QUrl>& urls, QWidget* const window)
{
#if KIO_VERSION < QT_VERSION_CHECK(5,6,0)
    return KRun::run(service, urls, window);
#else
    return KRun::runService(service, urls, window);
#endif
}

bool KIOWrapper::run(const QString& exec, const QList<QUrl>& urls, QWidget* const window)
{
    return KRun::run(exec, urls, window);
}

} // namespace Digikam
