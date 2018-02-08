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

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// LibKvkontakte includes

#include <Vkontakte/userinfojob.h>
#include <Vkontakte/vkapi.h>

// TODO: share this code with `vkwindow.cpp`
#define SLOT_JOB_DONE_INIT(JobClass) \
    JobClass* const job = dynamic_cast<JobClass*>(kjob); \
    Q_ASSERT(job);                                       \
    if (job && job->error())                             \
    {                                                    \
        handleVkError(job);                              \
        return;                                          \
    }

namespace Digikam
{

VKAuthWidget::VKAuthWidget(QWidget* const parent,
                           Vkontakte::VkApi* const vkapi)
    : QGroupBox(i18n("Account"), parent)
{
    m_vkapi  = vkapi;
    m_userId = -1;

    setWhatsThis(i18n("This account is used for authentication."));

    QGridLayout* const accountBoxLayout = new QGridLayout(this);
    QLabel* const loginDescLabel        = new QLabel(this);
    loginDescLabel->setText(i18n("Name:"));
    loginDescLabel->setWhatsThis(i18n("Your VKontakte login"));

    m_loginLabel       = new QLabel(this);
    m_changeUserButton = new QPushButton(QIcon::fromTheme(QString::fromLatin1("system-switch-user")),
                                         i18n("Change Account"), this);
    m_changeUserButton->setToolTip(i18n("Change VKontakte account used to authenticate"));
    m_changeUserButton->hide(); // changing account does not work anyway

    accountBoxLayout->addWidget(loginDescLabel,     0, 0);
    accountBoxLayout->addWidget(m_loginLabel,       0, 1);
    accountBoxLayout->addWidget(m_changeUserButton, 1, 1);
//     accountBoxLayout->setSpacing(KDialog::spacingHint());
//     accountBoxLayout->setMargin(KDialog::spacingHint());

    connect(m_changeUserButton, SIGNAL(clicked()),
            this, SLOT(slotChangeUserClicked()));

    connect(m_vkapi, SIGNAL(authenticated()),
            this, SLOT(startGetUserInfo()));

    connect(this, SIGNAL(signalUpdateAuthInfo()),
            this, SLOT(updateAuthInfo()));
}

VKAuthWidget::~VKAuthWidget()
{
}

//---------------------------------------------------------------------------

void VKAuthWidget::startAuthentication(bool forceLogout)
{
    m_userFullName.clear();
    m_userId = -1;
    m_vkapi->startAuthentication(forceLogout);

    emit authCleared();
}

void VKAuthWidget::slotChangeUserClicked()
{
    // force authenticate window
    startAuthentication(true);
}

//---------------------------------------------------------------------------

void VKAuthWidget::startGetUserInfo()
{
    // Retrieve user info with UserInfoJob
    Vkontakte::UserInfoJob* const job = new Vkontakte::UserInfoJob(m_vkapi->accessToken());

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotGetUserInfoDone(KJob*)));

    job->start();
}

void VKAuthWidget::slotGetUserInfoDone(KJob* kjob)
{
    SLOT_JOB_DONE_INIT(Vkontakte::UserInfoJob)

    if (!job) return;

    QList<Vkontakte::UserInfo> res = job->userInfo();
    Vkontakte::UserInfo user = res.first();

    m_userId = user.userId();
    m_userFullName = i18nc("Concatenation of first name (%1) and last name (%2)", "%1 %2",
                           user.firstName(), user.lastName());
    emit signalUpdateAuthInfo();
}

//---------------------------------------------------------------------------

void VKAuthWidget::updateAuthInfo()
{
    QString loginText;

    if (m_vkapi->isAuthenticated())
    {
        loginText = m_userFullName;
    }
    else
    {
        loginText = i18n("Unauthorized");
    }

    m_loginLabel->setText(QString::fromLatin1("<b>%1</b>").arg(loginText));
}

// TODO: share this code with `vkwindow.cpp`
void VKAuthWidget::handleVkError(KJob* kjob)
{
    QMessageBox::critical(this, i18nc("@title:window", "Request to VKontakte failed"), kjob->errorText());
}

//---------------------------------------------------------------------------

QString VKAuthWidget::albumsURL() const
{
    if (m_vkapi->isAuthenticated() && m_userId != -1)
        return QString::fromLatin1("http://vk.com/albums%1").arg(m_userId);
    else
        return QString::fromLatin1("http://vk.com/");
}

} // namespace Digikam
