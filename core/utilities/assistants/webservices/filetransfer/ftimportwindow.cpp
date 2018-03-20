/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 04.10.2009
 * Description : A tool for importing images via KIO
 *
 * Copyright (C) 2009      by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "ftimportwindow.h"

// Qt includes

#include <QAction>
#include <QMenu>
#include <QMessageBox>

// KDE includes

#include <kio/job.h>
#include <kio/copyjob.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "ftimportwidget.h"
#include "dimageslist.h"

namespace Digikam
{

class FTImportWindow::Private
{
public:

    explicit Private()
    {
        importWidget = 0;
        iface        = 0;
    }

    FTImportWidget* importWidget;
    DInfoInterface* iface;
};

FTImportWindow::FTImportWindow(DInfoInterface* const iface, QWidget* const /*parent*/)
    : WSToolDialog(0),
      d(new Private)
{
    d->iface        = iface;
    d->importWidget = new FTImportWidget(this, d->iface);
    setMainWidget(d->importWidget);

    // window setup

    setWindowTitle(i18n("Import from Remote Storage"));
    setModal(false);
    startButton()->setEnabled(false);

    startButton()->setText(i18n("Start import"));
    startButton()->setToolTip(i18n("Start importing the specified images "
                                   "into the currently selected album."));

    // connections

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotImport()));

    connect(d->importWidget->imagesList(), SIGNAL(signalImageListChanged()),
            this, SLOT(slotSourceAndTargetUpdated()));

    connect(d->iface, SIGNAL(signalUploadUrlChanged()),
            this, SLOT(slotSourceAndTargetUpdated()));

    slotSourceAndTargetUpdated();
}

FTImportWindow::~FTImportWindow()
{
    delete d;
}

void FTImportWindow::slotImport()
{
    QUrl url = d->iface->uploadUrl();

    if (!url.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "starting to import urls: " << d->importWidget->sourceUrls();

        // start copying and react on signals
        setEnabled(false);

        KIO::CopyJob* const copyJob = KIO::copy(d->importWidget->imagesList()->imageUrls(), url);

        connect(copyJob, SIGNAL(copyingDone(KIO::Job*, QUrl, QUrl, QDateTime, bool, bool)),
                this, SLOT(slotCopyingDone(KIO::Job*, QUrl, QUrl, QDateTime, bool, bool)));

        connect(copyJob, SIGNAL(result(KJob*)),
                this, SLOT(slotCopyingFinished(KJob*)));
    }
}

void FTImportWindow::slotCopyingDone(KIO::Job* job, const QUrl& from, const QUrl& to,
                                     const QDateTime& mtime, bool directory, bool renamed)
{
    Q_UNUSED(job);
    Q_UNUSED(to);
    Q_UNUSED(mtime);
    Q_UNUSED(directory);
    Q_UNUSED(renamed);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "copied " << to.toDisplayString();

    d->importWidget->imagesList()->removeItemByUrl(from);
}

void FTImportWindow::slotCopyingFinished(KJob* job)
{
    Q_UNUSED(job);

    setEnabled(true);

    if (!d->importWidget->imagesList()->imageUrls().empty())
    {
        QMessageBox::information(this, i18n("Import not completed"),
                                 i18n("Some of the images have not been transferred "
                                      "and are still in the list. "
                                      "You can retry to import these images now."));
    }
}

void FTImportWindow::slotSourceAndTargetUpdated()
{
    bool hasUrlToImport = !d->importWidget->sourceUrls().empty();
    bool hasTarget      = !d->iface->uploadUrl().isEmpty();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "switching import button activity with: hasUrlToImport = "
                                 << hasUrlToImport << ", hasTarget = " << hasTarget;

    startButton()->setEnabled(hasUrlToImport && hasTarget);
}

} // namespace Digikam
