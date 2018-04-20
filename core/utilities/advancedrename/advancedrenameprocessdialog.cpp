/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-14
 * Description : process dialog for renaming files
 *
 * Copyright (C) 2010-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "advancedrenameprocessdialog.h"

// Qt includes

#include <QDialogButtonBox>
#include <QApplication>
#include <QCloseEvent>
#include <QMessageBox>
#include <QPixmap>
#include <QTimer>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dio.h"
#include "imageviewutilities.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class AdvancedRenameProcessDialog::Private
{
public:

    explicit Private() :
        thumbLoadThread(0),
        utilities(0),
        cancel(false)
    {
    }

    ThumbnailLoadThread* thumbLoadThread;
    ImageViewUtilities*  utilities;

    NewNamesList         newNameList;
    QList<QUrl>          failedUrls;
    QUrl                 currentUrl;
    bool                 cancel;

    QPixmap              thumbPixmap;
};

AdvancedRenameProcessDialog::AdvancedRenameProcessDialog(const NewNamesList& list, QWidget* const parent)
    : DProgressDlg(parent),
      d(new Private)
{
    d->newNameList     = list;
    d->utilities       = new ImageViewUtilities(this);
    d->thumbLoadThread = new ThumbnailLoadThread;

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotGotThumbnail(LoadingDescription,QPixmap)));

    connect(DIO::instance(), SIGNAL(signalRenameSucceeded(QUrl)),
            this, SLOT(slotRenameSuccessded(QUrl)));

    connect(DIO::instance(), SIGNAL(signalRenameFailed(QUrl)),
            this, SLOT(slotRenameFailed(QUrl)));

    setValue(0);
    setModal(true);
    setButtonText(i18n("&Abort"));
    setTitle(i18n("Processing..."));
    setWindowTitle(i18n("Renaming images"));
    setLabel(i18n("<b>Renaming images. Please wait...</b>"));

    getNextThumbnail();
    setMaximum(d->newNameList.count());
    QTimer::singleShot(500, this, SLOT(slotRenameImages()));
}

AdvancedRenameProcessDialog::~AdvancedRenameProcessDialog()
{
    delete d->thumbLoadThread;
    delete d->utilities;
    delete d;
}

void AdvancedRenameProcessDialog::slotRenameImages()
{
    if (d->newNameList.isEmpty())
    {
        slotCancel();
        return;
    }

    processOne();
}

void AdvancedRenameProcessDialog::processOne()
{
    if (d->cancel || d->newNameList.isEmpty())
    {
        return;
    }

    NewNameInfo info = d->newNameList.takeFirst();
    d->currentUrl    = info.first;

    addedAction(d->thumbPixmap, QDir::toNativeSeparators(info.first.toLocalFile()));
    setLabel(i18n("<b>Renaming images. Please wait...</b>"));
    d->utilities->rename(info.first, info.second);
    getNextThumbnail();
}

void AdvancedRenameProcessDialog::complete()
{
    done(QDialogButtonBox::Cancel);
}

void AdvancedRenameProcessDialog::slotGotThumbnail(const LoadingDescription& /*desc*/, const QPixmap& pix)
{
    d->thumbPixmap = pix;
}

void AdvancedRenameProcessDialog::slotCancel()
{
    abort();
    done(QDialogButtonBox::Cancel);
}

void AdvancedRenameProcessDialog::slotRenameSuccessded(const QUrl& src)
{
    if (d->cancel || d->currentUrl != src)
    {
        return;
    }

    advance(1);

    if (d->newNameList.isEmpty())
    {
        if (!d->failedUrls.isEmpty())
        {
            QMessageBox msgBox(QMessageBox::Warning,
                               i18n("Renaming images"),
                               i18np("An error occurred while renaming %1 image.\n"
                                    "Do you want to rename this image again?",
                                    "An error occurred while renaming %1 images.\n"
                                    "Do you want to rename these images again?",
                                    d->failedUrls.count()),
                               QMessageBox::Yes | QMessageBox::No, this);

            if (msgBox.exec() != QMessageBox::Yes)
            {
                d->failedUrls.clear();
            }
        }

        complete();
    }
    else
    {
        processOne();
    }
}

void AdvancedRenameProcessDialog::slotRenameFailed(const QUrl& src)
{
    if (d->cancel || d->currentUrl != src)
    {
        return;
    }

    QPixmap pix = QIcon::fromTheme(QLatin1String("emblem-error")).pixmap(32, 32);
    addedAction(pix, QDir::toNativeSeparators(src.toLocalFile()));
    setLabel(i18n("<b>Renaming images has failed...</b>"));
    qApp->processEvents();

    d->failedUrls << src;

    QThread::msleep(500);

    slotRenameSuccessded(src);
}

void AdvancedRenameProcessDialog::closeEvent(QCloseEvent* e)
{
    abort();
    e->accept();
}

void AdvancedRenameProcessDialog::abort()
{
    d->cancel = true;
    d->failedUrls.clear();
}

QList<QUrl> AdvancedRenameProcessDialog::failedUrls() const
{
    return d->failedUrls;
}

void AdvancedRenameProcessDialog::getNextThumbnail()
{
    if (d->newNameList.count() > 0)
    {
        QString path = d->newNameList.first().first.toLocalFile();
        d->thumbLoadThread->find(ThumbnailIdentifier(path));
    }
}

} // namespace Digikam
