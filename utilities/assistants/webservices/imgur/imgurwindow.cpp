/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-06-06
 * Description : a tool to export images to Imgur web service
 *
 * Copyright (C) 2010-2012 by Marius Orcsik <marius at habarnam dot ro>
 * Copyright (C) 2016      by Fabian Vogt <fabian at ritter dash vogt dot de>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#include "imgurwindow.h"

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

static const constexpr char* IMGUR_CLIENT_ID("bd2572bce74b73d");
static const constexpr char* IMGUR_CLIENT_SECRET("300988683e99cb7b203a5889cf71de9ac891c1c1");

namespace Digikam
{

ImgurWindow::ImgurWindow(DInfoInterface* const iface, QWidget* const /*parent*/)
    : WSToolDialog(0)
{
    api = new ImgurTalker(QString::fromLatin1(IMGUR_CLIENT_ID),
                          QString::fromLatin1(IMGUR_CLIENT_SECRET), this);

    // Connect API signals
    connect(api, &ImgurTalker::signalAuthorized,
            this, &ImgurWindow::slotApiAuthorized);

    connect(api, &ImgurTalker::signalAuthError,
            this, &ImgurWindow::slotApiAuthError);

    connect(api, &ImgurTalker::progress,
            this, &ImgurWindow::slotApiProgress);

    connect(api, &ImgurTalker::requestPin,
            this, &ImgurWindow::slotApiRequestPin);

    connect(api, &ImgurTalker::success,
            this, &ImgurWindow::slotApiSuccess);

    connect(api, &ImgurTalker::error,
            this, &ImgurWindow::slotApiError);

    connect(api, &ImgurTalker::busy,
            this, &ImgurWindow::slotApiBusy);

    // | List | Auth |
    auto* mainLayout = new QHBoxLayout;
    auto* mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    this->setMainWidget(mainWidget);

    this->list       = new ImgurImagesList;
    list->setIface(iface);
    list->loadImagesFromCurrentSelection();
    mainLayout->addWidget(list);

    /* |  Logged in as:  |
     * | <Not logged in> |
     * |     Forget      |
     */
    auto* const userLabelLabel = new QLabel(i18n("Logged in as:"));
    userLabelLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    userLabelLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    // Label set in readSettings().
    this->userLabel            = new QLabel; 
    userLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    userLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    forgetButton           = new QPushButton(i18n("Forget"));

    auto* const authLayout = new QVBoxLayout;
    mainLayout->addLayout(authLayout);
    authLayout->addWidget(userLabelLabel);
    authLayout->addWidget(userLabel);
    authLayout->addWidget(forgetButton);
    authLayout->insertStretch(-1, 1);

    // Add anonymous upload button
    uploadAnonButton       = new QPushButton(i18n("Upload Anonymously"));
    addButton(uploadAnonButton, QDialogButtonBox::ApplyRole);

    // Connect UI signals
    connect(forgetButton, &QPushButton::clicked,
            this, &ImgurWindow::slotForgetButtonClicked);

    connect(startButton(), &QPushButton::clicked,
            this, &ImgurWindow::slotUpload);

    connect(uploadAnonButton, &QPushButton::clicked,
            this, &ImgurWindow::slotAnonUpload);

    connect(this, &ImgurWindow::finished,
            this, &ImgurWindow::slotFinished);

    connect(this, &ImgurWindow::cancelClicked,
            this, &ImgurWindow::slotCancel);

    setWindowTitle(i18n("Export to imgur.com"));
    setModal(false);

    startButton()->setText(i18n("Upload"));
    startButton()->setToolTip(i18n("Start upload to Imgur"));
    startButton()->setEnabled(true);

    // Only used if not overwritten by readSettings()
    resize(650, 320);
    readSettings();
}

ImgurWindow::~ImgurWindow()
{
    saveSettings();
}

void ImgurWindow::setItemsList(const QList<QUrl>& urls)
{
    this->list->slotAddImages(urls);
}

void ImgurWindow::reactivate()
{
    list->loadImagesFromCurrentSelection();
    show();
}

void ImgurWindow::slotForgetButtonClicked()
{
    api->getAuth().unlink();

    slotApiAuthorized(false, {});
}

void ImgurWindow::slotUpload()
{
    QList<const ImgurImageListViewItem*> pending = this->list->getPendingItems();

    for (auto item : pending)
    {
        ImgurTalkerAction action;
        action.type               = ImgurTalkerActionType::IMG_UPLOAD;
        action.upload.imgpath     = item->url().toLocalFile();
        action.upload.title       = item->Title();
        action.upload.description = item->Description();

        api->queueWork(action);
    }
}

void ImgurWindow::slotAnonUpload()
{
    QList<const ImgurImageListViewItem*> pending = this->list->getPendingItems();

    for (auto item : pending)
    {
        ImgurTalkerAction action;
        action.type               = ImgurTalkerActionType::ANON_IMG_UPLOAD;
        action.upload.imgpath     = item->url().toLocalFile();
        action.upload.title       = item->Title();
        action.upload.description = item->Description();

        api->queueWork(action);
    }
}

void ImgurWindow::slotFinished()
{
    saveSettings();
}

void ImgurWindow::slotCancel()
{
    api->cancelAllWork();
}

void ImgurWindow::slotApiAuthorized(bool success, const QString& username)
{
    if (success)
    {
        this->username = username;
        this->userLabel->setText(this->username);
        this->forgetButton->setEnabled(true);
        return;
    }

    this->username = QString();
    this->userLabel->setText(i18n("<Not logged in>"));
    this->forgetButton->setEnabled(false);
}

void ImgurWindow::slotApiAuthError(const QString& msg)
{
    QMessageBox::critical(this,
                          i18n("Authorization Failed"),
                          i18n("Failed to log into Imgur: %1\n", msg));
}

void ImgurWindow::slotApiProgress(unsigned int /*percent*/, const ImgurTalkerAction& action)
{
    list->processing(QUrl::fromLocalFile(action.upload.imgpath));
}

void ImgurWindow::slotApiRequestPin(const QUrl& url)
{
    QDesktopServices::openUrl(url);
}

void ImgurWindow::slotApiSuccess(const ImgurTalkerResult& result)
{
    list->slotSuccess(result);
}

void ImgurWindow::slotApiError(const QString& msg, const ImgurTalkerAction& action)
{
    list->processed(QUrl::fromLocalFile(action.upload.imgpath), false);

    /* 1 here because the current item is still in the queue. */
    if (api->workQueueLength() <= 1)
    {
        QMessageBox::critical(this,
                              i18n("Uploading Failed"),
                              i18n("Failed to upload photo to Imgur: %1\n", msg));
        return;
    }

    QMessageBox::StandardButton cont =
            QMessageBox::question(this,
                                  i18n("Uploading Failed"),
                                  i18n("Failed to upload photo to Imgur: %1\n"
                                       "Do you want to continue?", msg));

    if (cont != QMessageBox::Yes)
        api->cancelAllWork();
}

void ImgurWindow::slotApiBusy(bool busy)
{
    setCursor(busy ? Qt::WaitCursor : Qt::ArrowCursor);
    startButton()->setEnabled(!busy);
}

void ImgurWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
        return;

    slotFinished();
    e->accept();
}

void ImgurWindow::readSettings()
{
    KConfig config;
    KConfigGroup groupAuth = config.group("Imgur Auth");
    username               = groupAuth.readEntry("username", QString());
    slotApiAuthorized(!username.isEmpty(), username);

    winId();
    KConfigGroup groupDialog = config.group("Imgur Dialog");
    KWindowConfig::restoreWindowSize(windowHandle(), groupDialog);
    resize(windowHandle()->size());
}

void ImgurWindow::saveSettings()
{
    KConfig config;
    KConfigGroup groupAuth   = config.group("Imgur Auth");
    groupAuth.writeEntry("username", username);

    KConfigGroup groupDialog = config.group("Imgur Dialog");
    KWindowConfig::saveWindowSize(windowHandle(), groupDialog);
    config.sync();
}

} // namespace Digikam
