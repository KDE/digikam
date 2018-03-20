/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-11-12
 * Description : a common login dialog for Web Service tools
 *
 * Copyright (C) 2007-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2011      by Roman Tsisyk <roman at tsisyk dot com>
 * Copyright (C) 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "wslogindialog.h"

// Qt includes

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class WSLoginDialog::Private
{
public:

    explicit Private()
        : headerLabel(0),
          loginEdit(0),
          passwordEdit(0)
    {
    }

    QLabel*    headerLabel;
    QLineEdit* loginEdit;
    QLineEdit* passwordEdit;
};
    
WSLoginDialog::WSLoginDialog(QWidget* const parent,
                             const QString& prompt,
                             const QString& login,
                             const QString& password)
    : QDialog(parent),
      d(new Private)
{
    setSizeGripEnabled(false);

    QVBoxLayout* const vbox = new QVBoxLayout(this);
    d->headerLabel          = new QLabel(this);
    d->headerLabel->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    d->headerLabel->setTextFormat(Qt::RichText);
    d->headerLabel->setText(prompt);

    QFrame* const hline = new QFrame(this);
    hline->setObjectName(QString::fromLatin1("hline"));
    hline->setFrameShape( QFrame::HLine );
    hline->setFrameShadow( QFrame::Sunken );
    hline->setFrameShape( QFrame::HLine );

    QGridLayout* const centerLayout = new QGridLayout();

    d->loginEdit    = new QLineEdit(this);
    d->passwordEdit = new QLineEdit(this);
    d->passwordEdit->setEchoMode(QLineEdit::Password);

    QLabel* const loginLabel    = new QLabel(this);
    loginLabel->setText(i18n( "Login:" ));

    QLabel* const passwordLabel = new QLabel(this);
    passwordLabel->setText(i18n("Password:"));

    centerLayout->addWidget(d->loginEdit,    0, 1);
    centerLayout->addWidget(d->passwordEdit, 1, 1);
    centerLayout->addWidget(loginLabel,     0, 0);
    centerLayout->addWidget(passwordLabel,  1, 0);

    QHBoxLayout* const btnLayout = new QHBoxLayout();
    QPushButton* const okBtn     = new QPushButton(this);
    okBtn->setAutoDefault(true);
    okBtn->setDefault(true);
    okBtn->setText(i18n("&Login"));

    QPushButton* const cancelBtn = new QPushButton(this);
    cancelBtn->setText(i18n("&Skip"));

    btnLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    btnLayout->setContentsMargins(QMargins());
    btnLayout->setSpacing(5);

    vbox->setContentsMargins(5, 5, 5, 5);
    vbox->setSpacing(5);
    vbox->setObjectName(QString::fromLatin1("vbox"));
    vbox->addWidget(d->headerLabel);
    vbox->addWidget(hline);
    vbox->addLayout(centerLayout);
    vbox->addLayout(btnLayout);

    resize(QSize(300, 150).expandedTo(minimumSizeHint()));

    setLogin(login);
    setPassword(password);

    connect(okBtn, SIGNAL(clicked()),
            this, SLOT(slotAccept()));

    connect(cancelBtn, SIGNAL(clicked()),
            this, SLOT(reject()));
}

WSLoginDialog::~WSLoginDialog()
{
    delete d;
}

QString WSLoginDialog::login() const
{
    return d->loginEdit->text();
}

QString WSLoginDialog::password() const
{
    return d->passwordEdit->text();
}

void WSLoginDialog::setLogin(const QString& login)
{
    d->loginEdit->setText(login);
}

void WSLoginDialog::setPassword(const QString& password)
{
    d->passwordEdit->setText(password);
}

void WSLoginDialog::slotAccept()
{
    if (!d->passwordEdit->text().isEmpty())
    {
        accept();
    }
    else
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error"),
                              i18n("Password cannot be empty."));
    }
}

} // namespace Digikam
