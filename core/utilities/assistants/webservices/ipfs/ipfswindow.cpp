/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-12
 * Description : a tool to export images to IPFS web service
 *
 * Copyright (C) 2018 by Amar Lakshya <amar dot lakshya  at xaviers dot edu dot in>
 * Copyright (C) 2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#include "ipfswindow.h"

// Qt includes

#include <QDesktopServices>
#include <QInputDialog>
#include <QCloseEvent>
#include <QMessageBox>
#include <QBoxLayout>
#include <QWindow>

// KDE includes

#include <klocalizedstring.h>
#include <kwindowconfig.h>
#include <kconfig.h>

// Local includes

#include "digikam_debug.h"
#include "dinfointerface.h"
#include "digikam_version.h"

namespace Digikam
{

IpfsWindow::IpfsWindow(DInfoInterface* const iface, QWidget* const /*parent*/)
    : WSToolDialog(0)
{
    api = new IpfsTalker(this);

    /* Connect API signals */

    connect(api, &IpfsTalker::progress,
            this, &IpfsWindow::apiProgress);

    connect(api, &IpfsTalker::success,
            this, &IpfsWindow::apiSuccess);

    connect(api, &IpfsTalker::error,
            this, &IpfsWindow::apiError);

    connect(api, &IpfsTalker::busy,
            this, &IpfsWindow::apiBusy);

    /* | List | Auth | */
    auto* mainLayout = new QHBoxLayout;
    auto* mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    this->setMainWidget(mainWidget);

    this->list = new IpfsImagesList;
    this->list->setIface(iface);
    mainLayout->addWidget(list);

    /* |  Logged in as:  |
     * | <Not logged in> |
     * |     Forget      | */

    auto* authLayout = new QVBoxLayout;
    mainLayout->addLayout(authLayout);

    authLayout->insertStretch(-1, 1);

    /* Add anonymous upload button */
    /* Connect UI signals */
    connect(startButton(), &QPushButton::clicked,
            this, &IpfsWindow::slotUpload);

    connect(this, &IpfsWindow::finished,
            this, &IpfsWindow::slotFinished);

    connect(this, &IpfsWindow::cancelClicked,
            this, &IpfsWindow::slotCancel);

    setWindowIcon(QIcon::fromTheme(QString::fromLatin1("ipfs")));
    setWindowTitle(i18n("Export to IPFS"));
    setModal(false);

    startButton()->setText(i18n("Upload"));
    startButton()->setToolTip(i18n("Start upload to IPFS"));
    startButton()->setEnabled(true);

    /* Only used if not overwritten by readSettings() */
    resize(650, 320);
    readSettings();
}

IpfsWindow::~IpfsWindow()
{
    saveSettings();
}

void IpfsWindow::reactivate()
{
    list->loadImagesFromCurrentSelection();
    show();
}

void IpfsWindow::slotUpload()
{
    QList<const IpfsImagesListViewItem*> pending = this->list->getPendingItems();

    for (auto item : pending)
    {
        IpfsTalkerAction action;
        action.type               = IpfsTalkerActionType::IMG_UPLOAD;
        action.upload.imgpath     = item->url().toLocalFile();
        action.upload.title       = item->Title();
        action.upload.description = item->Description();

        api->queueWork(action);
    }
}

void IpfsWindow::slotFinished()
{
    saveSettings();
}

void IpfsWindow::slotCancel()
{
    api->cancelAllWork();
}

/* void IpfsWindow::apiAuthorized(bool success, const QString& username) */
// {
    // if (success)
    // {
        // this->username = username;
        // this->userLabel->setText(this->username);
        // this->forgetButton->setEnabled(true);
        // return;
    // }
//
    // this->username = QString();
    // this->userLabel->setText(i18n("<Not logged in>"));
    // this->forgetButton->setEnabled(false);
/* } */

/* void IpfsWindow::apiAuthError(const QString& msg) */
// {
    // QMessageBox::critical(this,
                          // i18n("Authorization Failed"),
                          // i18n("Failed to log into IPFS: %1\n", msg));
// }
/*  */

void IpfsWindow::apiProgress(unsigned int /*percent*/, const IpfsTalkerAction& action)
{
    list->processing(QUrl::fromLocalFile(action.upload.imgpath));
}

void IpfsWindow::apiRequestPin(const QUrl& url)
{
    QDesktopServices::openUrl(url);
}

void IpfsWindow::apiSuccess(const IpfsTalkerResult& result)
{
    list->slotSuccess(result);
}

void IpfsWindow::apiError(const QString& msg, const IpfsTalkerAction& action)
{
    list->processed(QUrl::fromLocalFile(action.upload.imgpath), false);

    /* 1 here because the current item is still in the queue. */
    if (api->workQueueLength() <= 1)
    {
        QMessageBox::critical(this,
                              i18n("Uploading Failed"),
                              i18n("Failed to upload photo to IPFS: %1\n", msg));
        return;
    }

    QMessageBox::StandardButton cont =
            QMessageBox::question(this,
                                  i18n("Uploading Failed"),
                                  i18n("Failed to upload photo to IPFS: %1\n"
                                       "Do you want to continue?", msg));

    if (cont != QMessageBox::Yes)
        api->cancelAllWork();
}

void IpfsWindow::apiBusy(bool busy)
{
    setCursor(busy ? Qt::WaitCursor : Qt::ArrowCursor);
    startButton()->setEnabled(!busy);
}

void IpfsWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
        return;

    slotFinished();
    e->accept();
}

void IpfsWindow::readSettings()
{
    KConfig config;
    KConfigGroup groupAuth = config.group("IPFS Auth");
    username = groupAuth.readEntry("username", QString());
    // apiAuthorized(!username.isEmpty(), username);

    winId();
    KConfigGroup groupDialog = config.group("IPFS Dialog");
    KWindowConfig::restoreWindowSize(windowHandle(), groupDialog);
    resize(windowHandle()->size());
}

void IpfsWindow::saveSettings()
{
    KConfig config;
    KConfigGroup groupAuth = config.group("IPFS Auth");
    groupAuth.writeEntry("username", username);

    KConfigGroup groupDialog = config.group("IPFS Dialog");
    KWindowConfig::saveWindowSize(windowHandle(), groupDialog);
    config.sync();
}

} // namespace Digikam
