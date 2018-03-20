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

    Private() :
        thumbLoadThread(0),
        utilities(0),
        cancel(false)
    {
    }

    ThumbnailLoadThread* thumbLoadThread;
    ImageViewUtilities*  utilities;

    NewNamesList         newNameList;
    QUrl                 currentUrl;
    bool                 cancel;
};

AdvancedRenameProcessDialog::AdvancedRenameProcessDialog(const NewNamesList& list)
    : DProgressDlg(0),
      d(new Private)
{
    d->newNameList     = list;
    d->utilities       = new ImageViewUtilities(this);
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotGotThumbnail(LoadingDescription,QPixmap)));

    connect(DIO::instance(), SIGNAL(signalRenameSucceeded(QUrl)),
            this, SLOT(slotRenameSuccessded(QUrl)));

    connect(DIO::instance(), SIGNAL(signalRenameFailed(QUrl)),
            this, SLOT(slotRenameFailed(QUrl)));

    setValue(0);
    setModal(true);
    setButtonText(i18n("&Abort"));
    setWindowTitle(i18n("Renaming images"));
    setLabel(i18n("<b>Renaming images. Please wait...</b>"));

    QTimer::singleShot(500, this, SLOT(slotRenameImages()));
}

AdvancedRenameProcessDialog::~AdvancedRenameProcessDialog()
{
    delete d->utilities;
    delete d;
}

void AdvancedRenameProcessDialog::slotRenameImages()
{
    setTitle(i18n("Processing..."));

    setMaximum(d->newNameList.count());

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

    d->currentUrl.clear();
    d->thumbLoadThread->find(ThumbnailIdentifier(d->newNameList.first().first.toLocalFile()));
}

void AdvancedRenameProcessDialog::complete()
{
    done(QDialogButtonBox::Cancel);
}

void AdvancedRenameProcessDialog::slotGotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    if (d->cancel || d->newNameList.isEmpty())
    {
        return;
    }

    if (d->newNameList.first().first.toLocalFile() != desc.filePath)
    {
        return;
    }

    if (d->currentUrl.toLocalFile() == desc.filePath)
    {
        return;
    }

    addedAction(pix, QDir::toNativeSeparators(desc.filePath));
    setLabel(i18n("<b>Renaming images. Please wait...</b>"));
    advance(1);

    NewNameInfo info = d->newNameList.takeFirst();
    d->currentUrl    = info.first;

    d->utilities->rename(info.first, info.second);
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

    if (d->newNameList.isEmpty())
    {
        complete();
    }
    else
    {
        processOne();
    }
}

void AdvancedRenameProcessDialog::slotRenameFailed(const QUrl& src)
{
    QPixmap pix = QIcon::fromTheme(QLatin1String("emblem-error")).pixmap(32, 32);
    addedAction(pix, QDir::toNativeSeparators(src.toLocalFile()));
    setLabel(i18n("<b>Renaming images has failed...</b>"));
    qApp->processEvents();

    QThread::sleep(2);

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
}

}  // namespace Digikam
