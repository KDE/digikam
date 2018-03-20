/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-19
 * Description : a tool to export images to VKontakte web service
 *
 * Copyright (C) 2011-2015 by Alexander Potashev <aspotashev at gmail dot com>
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

#include "vkauthwidget.h"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// LibKvkontakte includes

#include <Vkontakte/userinfojob.h>
#include <Vkontakte/vkapi.h>

namespace Digikam
{

class VKAuthWidget::Private
{
public:

    explicit Private()
    {
        vkapi            = 0;
        userId           = -1;
        loginLabel       = 0;
        changeUserButton = 0;
    }

    // VK.com interface
    Vkontakte::VkApi* vkapi;

    // Data
    int               userId;
    QString           userFullName;

    // GUI
    QLabel*           loginLabel;
    QPushButton*      changeUserButton;
};

VKAuthWidget::VKAuthWidget(QWidget* const parent,
                           Vkontakte::VkApi* const vkapi)
    : QGroupBox(i18n("Account"), parent),
      d(new Private)
{
    d->vkapi = vkapi;

    setWhatsThis(i18n("This account is used for authentication."));

    QGridLayout* const accountBoxLayout = new QGridLayout(this);
    QLabel* const loginDescLabel        = new QLabel(this);
    loginDescLabel->setText(i18n("Name:"));
    loginDescLabel->setWhatsThis(i18n("Your VKontakte login"));

    d->loginLabel       = new QLabel(this);
    d->changeUserButton = new QPushButton(QIcon::fromTheme(QString::fromLatin1("system-switch-user")),
                                         i18n("Change Account"), this);
    d->changeUserButton->setToolTip(i18n("Change VKontakte account used to authenticate"));
    d->changeUserButton->hide(); // changing account does not work anyway

    accountBoxLayout->addWidget(loginDescLabel,      0, 0);
    accountBoxLayout->addWidget(d->loginLabel,       0, 1);
    accountBoxLayout->addWidget(d->changeUserButton, 1, 1);

    connect(d->changeUserButton, SIGNAL(clicked()),
            this, SLOT(slotChangeUserClicked()));

    connect(d->vkapi, SIGNAL(authenticated()),
            this, SLOT(slotStartGetUserInfo()));

    connect(this, SIGNAL(signalUpdateAuthInfo()),
            this, SLOT(slotUpdateAuthInfo()));
}

VKAuthWidget::~VKAuthWidget()
{
    delete d;
}

//---------------------------------------------------------------------------

void VKAuthWidget::slotStartAuthentication(bool forceLogout)
{
    d->userFullName.clear();
    d->userId = -1;
    d->vkapi->startAuthentication(forceLogout);

    emit signalAuthCleared();
}

void VKAuthWidget::slotChangeUserClicked()
{
    // force authenticate window
    slotStartAuthentication(true);
}

//---------------------------------------------------------------------------

void VKAuthWidget::slotStartGetUserInfo()
{
    // Retrieve user info with UserInfoJob
    Vkontakte::UserInfoJob* const job = new Vkontakte::UserInfoJob(d->vkapi->accessToken());

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotGetUserInfoDone(KJob*)));

    job->start();
}

void VKAuthWidget::slotGetUserInfoDone(KJob* kjob)
{
    Vkontakte::UserInfoJob* const job = dynamic_cast<Vkontakte::UserInfoJob*>(kjob);
    Q_ASSERT(job);

    if (job && job->error())
    {
        handleVkError(job);
        return;
    }

    if (!job)
        return;

    QList<Vkontakte::UserInfo> res = job->userInfo();
    Vkontakte::UserInfo user       = res.first();

    d->userId       = user.userId();
    d->userFullName = i18nc("Concatenation of first name (%1) and last name (%2)", "%1 %2",
                           user.firstName(), user.lastName());
    emit signalUpdateAuthInfo();
}

//---------------------------------------------------------------------------

void VKAuthWidget::slotUpdateAuthInfo()
{
    QString loginText;

    if (d->vkapi->isAuthenticated())
    {
        loginText = d->userFullName;
    }
    else
    {
        loginText = i18n("Unauthorized");
    }

    d->loginLabel->setText(QString::fromLatin1("<b>%1</b>").arg(loginText));
}

// TODO: share this code with `vkwindow.cpp`
void VKAuthWidget::handleVkError(KJob* kjob)
{
    QMessageBox::critical(this, i18nc("@title:window", "Request to VKontakte failed"), kjob->errorText());
}

//---------------------------------------------------------------------------

QString VKAuthWidget::albumsURL() const
{
    if (d->vkapi->isAuthenticated() && d->userId != -1)
        return QString::fromLatin1("http://vk.com/albums%1").arg(d->userId);
    else
        return QString::fromLatin1("http://vk.com/");
}

} // namespace Digikam
