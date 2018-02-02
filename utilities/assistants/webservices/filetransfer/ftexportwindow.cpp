/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-28
 * Description : a tool to export image to a KIO accessible
 *               location
 *
 * Copyright (C) 2006-2009 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "ftexportwindow.h"

// Qt includes

#include <QWindow>
#include <QCloseEvent>
#include <QMenu>
#include <QMessageBox>

// KDE includes

#include <kconfig.h>
#include <klocalizedstring.h>
#include <kwindowconfig.h>
#include <kio/job.h>
#include <kio/copyjob.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "dimageslist.h"
#include "ftexportwidget.h"

namespace Digikam
{

const QString FTExportWindow::TARGET_URL_PROPERTY  = QString::fromLatin1("targetUrl");
const QString FTExportWindow::HISTORY_URL_PROPERTY = QString::fromLatin1("historyUrls");
const QString FTExportWindow::CONFIG_GROUP         = QString::fromLatin1("KioExport");

FTExportWindow::FTExportWindow(DInfoInterface* const iface, QWidget* const /*parent*/)
    : WSToolDialog(0)
{
    m_exportWidget = new FTExportWidget(iface, this);
    setMainWidget(m_exportWidget);

    // -- Window setup ------------------------------------------------------

    setWindowTitle(i18n("Export to Remote Storage"));
    setModal(false);

    startButton()->setText(i18n("Start export"));
    startButton()->setToolTip(i18n("Start export to the specified target"));

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotUpload()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    connect(m_exportWidget->imagesList(), SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(m_exportWidget, SIGNAL(signalTargetUrlChanged(QUrl)),
            this, SLOT(slotTargetUrlChanged(QUrl)));

    // -- initial sync ------------------------------------------------------

    restoreSettings();
    updateUploadButton();
}

FTExportWindow::~FTExportWindow()
{
}

void FTExportWindow::slotFinished()
{
    saveSettings();
    m_exportWidget->imagesList()->listView()->clear();
}

void FTExportWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

void FTExportWindow::reactivate()
{
    m_exportWidget->imagesList()->loadImagesFromCurrentSelection();
    show();
}

void FTExportWindow::restoreSettings()
{
    KConfig config;
    KConfigGroup group  = config.group(CONFIG_GROUP);
    m_exportWidget->setHistory(group.readEntry(HISTORY_URL_PROPERTY, QList<QUrl>()));
    m_exportWidget->setTargetUrl(group.readEntry(TARGET_URL_PROPERTY, QUrl()));

    winId();
    KConfigGroup group2 = config.group(QString::fromLatin1("Kio Export Dialog"));
    KWindowConfig::restoreWindowSize(windowHandle(), group2);
    resize(windowHandle()->size());
}

void FTExportWindow::saveSettings()
{
    KConfig config;
    KConfigGroup group = config.group(CONFIG_GROUP);
    group.writeEntry(HISTORY_URL_PROPERTY, m_exportWidget->history());
    group.writeEntry(TARGET_URL_PROPERTY,  m_exportWidget->targetUrl().url());

    KConfigGroup group2 = config.group(QString::fromLatin1("Kio Export Dialog"));
    KWindowConfig::saveWindowSize(windowHandle(), group2);
    config.sync();
}

void FTExportWindow::slotImageListChanged()
{
    updateUploadButton();
}

void FTExportWindow::slotTargetUrlChanged(const QUrl & target)
{
    Q_UNUSED(target);
    updateUploadButton();
}

void FTExportWindow::updateUploadButton()
{
    bool listNotEmpty = !m_exportWidget->imagesList()->imageUrls().empty();
    startButton()->setEnabled(listNotEmpty && m_exportWidget->targetUrl().isValid());

    qCDebug(DIGIKAM_GENERAL_LOG) << "Updated upload button with listNotEmpty = "
                                 << listNotEmpty << ", targetUrl().isValid() = "
                                 << m_exportWidget->targetUrl().isValid();
}

void FTExportWindow::slotCopyingDone(KIO::Job* job, const QUrl& from, const QUrl& to,
                                      const QDateTime& mtime, bool directory, bool renamed)
{
    Q_UNUSED(job);
    Q_UNUSED(to);
    Q_UNUSED(mtime);
    Q_UNUSED(directory);
    Q_UNUSED(renamed);

    qCDebug(DIGIKAM_GENERAL_LOG) << "copied " << to.toDisplayString();

    m_exportWidget->imagesList()->removeItemByUrl(from);
}

void FTExportWindow::slotCopyingFinished(KJob* job)
{
    Q_UNUSED(job);

    setEnabled(true);

    if (!m_exportWidget->imagesList()->imageUrls().empty())
    {
        QMessageBox::information(this, i18n("Upload not completed"),
                                 i18n("Some of the images have not been transferred "
                                      "and are still in the list. "
                                      "You can retry to export these images now."));
    }
}

void FTExportWindow::slotUpload()
{
    saveSettings();

    // start copying and react on signals
    setEnabled(false);
    KIO::CopyJob* const copyJob = KIO::copy(m_exportWidget->imagesList()->imageUrls(),
                                            m_exportWidget->targetUrl());

    connect(copyJob, SIGNAL(copyingDone(KIO::Job*, QUrl, QUrl, QDateTime, bool, bool)),
            this, SLOT(slotCopyingDone(KIO::Job*, QUrl, QUrl, QDateTime, bool, bool)));

    connect(copyJob, SIGNAL(result(KJob*)),
            this, SLOT(slotCopyingFinished(KJob*)));
}

} // namespace Digikam
