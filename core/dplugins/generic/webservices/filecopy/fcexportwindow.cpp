/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-03-27
 * Description : a tool to export items to a local storage
 *
 * Copyright (C) 2006-2009 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2019      by Maik Qualmann <metzpinguin at gmail dot com>
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

#include "fcexportwindow.h"

// Qt includes

#include <QWindow>
#include <QCloseEvent>
#include <QMessageBox>

// KDE includes

#include <kconfig.h>
#include <kwindowconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "fcexportwidget.h"
#include "ditemslist.h"
#include "fcthread.h"

namespace DigikamGenericFileCopyPlugin
{

class Q_DECL_HIDDEN FCExportWindow::Private
{
public:

    explicit Private()
    {
        exportWidget = 0;
        thread       = 0;
    }

    const static QString TARGET_URL_PROPERTY;
    const static QString TARGET_OVERWRITE;
    const static QString CONFIG_GROUP;

    FCExportWidget*      exportWidget;
    FCThread*            thread;
};

const QString FCExportWindow::Private::TARGET_URL_PROPERTY = QLatin1String("targetUrl");
const QString FCExportWindow::Private::TARGET_OVERWRITE    = QLatin1String("overwrite");
const QString FCExportWindow::Private::CONFIG_GROUP        = QLatin1String("FileCopyExport");

FCExportWindow::FCExportWindow(DInfoInterface* const iface, QWidget* const /*parent*/)
    : WSToolDialog(0, QLatin1String("FileCopy Export Dialog")),
      d(new Private)
{
    d->exportWidget = new FCExportWidget(iface, this);
    setMainWidget(d->exportWidget);

    // -- Window setup ------------------------------------------------------

    setWindowTitle(i18n("Export to Local Storage"));
    setModal(false);

    startButton()->setText(i18n("Start export"));
    startButton()->setToolTip(i18n("Start export to the specified target"));

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotCopy()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    connect(d->exportWidget->imagesList(), SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(d->exportWidget, SIGNAL(signalTargetUrlChanged(QUrl)),
            this, SLOT(slotTargetUrlChanged(QUrl)));

    // -- initial sync ------------------------------------------------------

    restoreSettings();
    updateUploadButton();
}

FCExportWindow::~FCExportWindow()
{
    delete d;
}

void FCExportWindow::slotFinished()
{
    saveSettings();
    d->exportWidget->imagesList()->listView()->clear();
}

void FCExportWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

void FCExportWindow::reactivate()
{
    d->exportWidget->imagesList()->loadImagesFromCurrentSelection();
    show();
}

void FCExportWindow::restoreSettings()
{
    KConfig config;
    KConfigGroup group  = config.group(d->CONFIG_GROUP);
    d->exportWidget->setTargetUrl(group.readEntry(d->TARGET_URL_PROPERTY,            QUrl()));
    d->exportWidget->overwriteBox()->setChecked(group.readEntry(d->TARGET_OVERWRITE, false));

    winId();
    KConfigGroup group2 = config.group(QLatin1String("FileCopy Export Dialog"));
    KWindowConfig::restoreWindowSize(windowHandle(), group2);
    resize(windowHandle()->size());
}

void FCExportWindow::saveSettings()
{
    KConfig config;
    KConfigGroup group = config.group(d->CONFIG_GROUP);
    group.writeEntry(d->TARGET_URL_PROPERTY, d->exportWidget->targetUrl().url());
    group.writeEntry(d->TARGET_OVERWRITE,    d->exportWidget->overwriteBox()->isChecked());

    KConfigGroup group2 = config.group(QLatin1String("FileCopy Export Dialog"));
    KWindowConfig::saveWindowSize(windowHandle(), group2);
    config.sync();
}

void FCExportWindow::slotImageListChanged()
{
    updateUploadButton();
}

void FCExportWindow::slotTargetUrlChanged(const QUrl & target)
{
    Q_UNUSED(target);
    updateUploadButton();
}

void FCExportWindow::updateUploadButton()
{
    bool listNotEmpty = !d->exportWidget->imagesList()->imageUrls().isEmpty();
    startButton()->setEnabled(listNotEmpty && d->exportWidget->targetUrl().isValid());

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Updated upload button with listNotEmpty = "
                                     << listNotEmpty << ", targetUrl().isValid() = "
                                     << d->exportWidget->targetUrl().isValid();
}

void FCExportWindow::slotCopyingDone(const QUrl& from, const QUrl& to)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Copied to:" << to.toLocalFile();

    d->exportWidget->imagesList()->removeItemByUrl(from);
}

void FCExportWindow::slotCopyingFinished()
{
    setEnabled(true);

    if (!d->exportWidget->imagesList()->imageUrls().isEmpty())
    {
        QMessageBox::information(this, i18n("Copy not completed"),
                                 i18n("Some of the items have not been copied "
                                      "and are still in the list. "
                                      "You can retry to copy these items now."));
    }
}

void FCExportWindow::slotCopy()
{
    saveSettings();

    // start copying and react on signals
    setEnabled(false);

    if (d->thread)
    {
        d->thread->cancel();
    }
    else
    {
        d->thread = new FCThread(this);

        connect(d->thread, SIGNAL(finished()),
                this, SLOT(slotCopyingFinished()));

        connect(d->thread, SIGNAL(signalUrlProcessed(QUrl,QUrl)),
                this, SLOT(slotCopyingDone(QUrl,QUrl)));
    }

    d->thread->setItemsUrlList(d->exportWidget->imagesList()->imageUrls(),
                               d->exportWidget->targetUrl(),
                               d->exportWidget->overwriteBox()->isChecked());

    d->thread->start();
}

} // namespace DigikamGenericFileCopyPlugin
