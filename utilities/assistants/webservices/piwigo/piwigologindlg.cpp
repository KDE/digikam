/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-30
 * Description : a tool to export items to Piwigo web service
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006      by Colin Guthrie <kde at colin dot guthr dot ie>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008      by Andrea Diamantini <adjam7 at gmail dot com>
 * Copyright (C) 2010-2014 by Frederic Coiffier <frederic dot coiffier at free dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the id->plied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "piwigologindlg.h"

// Qt includes

#include <QLabel>
#include <QFrame>
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QDialogButtonBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_version.h"
#include "digikam_debug.h"
#include "piwigosession.h"

namespace Digikam
{

class PiwigoLoginDlg::Private
{
public:

    explicit Private()
    {
        pUrlEdit      = 0;
        pUsernameEdit = 0;
        pPasswordEdit = 0;
        pPiwigo       = 0;
    }

    QLineEdit*     pUrlEdit;
    QLineEdit*     pUsernameEdit;
    QLineEdit*     pPasswordEdit;

    PiwigoSession* pPiwigo;
};

PiwigoLoginDlg::PiwigoLoginDlg(QWidget* const pParent,
                               PiwigoSession* const pPiwigo,
                               const QString& title)
    : QDialog(pParent, Qt::Dialog),
      d(new Private)
{
    d->pPiwigo = pPiwigo;

    setWindowTitle(title);

    QFrame* const  page             = new QFrame(this);
    QGridLayout* const centerLayout = new QGridLayout();
    page->setMinimumSize(500, 128);

    d->pUrlEdit      = new QLineEdit(this);
    centerLayout->addWidget(d->pUrlEdit, 1, 1);

    d->pUsernameEdit = new QLineEdit(this);
    centerLayout->addWidget(d->pUsernameEdit, 2, 1);

    d->pPasswordEdit = new QLineEdit(this);
    d->pPasswordEdit->setEchoMode(QLineEdit::Password);
    centerLayout->addWidget(d->pPasswordEdit, 3, 1);

    QLabel* const urlLabel = new QLabel(this);
    urlLabel->setText(i18nc("piwigo login settings", "URL:"));
    centerLayout->addWidget(urlLabel, 1, 0);

    QLabel* const usernameLabel = new QLabel(this);
    usernameLabel->setText(i18nc("piwigo login settings", "Username:"));
    centerLayout->addWidget(usernameLabel, 2, 0);

    QLabel* const passwdLabel = new QLabel(this);
    passwdLabel->setText(i18nc("piwigo login settings", "Password:"));
    centerLayout->addWidget(passwdLabel, 3, 0);

    //---------------------------------------------

    page->setLayout(centerLayout);

    resize(QSize(300, 150).expandedTo(minimumSizeHint()));

    // setting initial data
    d->pUrlEdit->setText(pPiwigo->url());
    d->pUsernameEdit->setText(pPiwigo->username());
    d->pPasswordEdit->setText(pPiwigo->password());

    //---------------------------------------------

    QDialogButtonBox* const buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                             QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

    QVBoxLayout* const dialogLayout   = new QVBoxLayout(this);
    dialogLayout->addWidget(page);
    dialogLayout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(slotOk()));

    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));
}

PiwigoLoginDlg::~PiwigoLoginDlg()
{
    delete d;
}

void PiwigoLoginDlg::slotOk()
{
    if (d->pUrlEdit->isModified())
        d->pPiwigo->setUrl(d->pUrlEdit->text());

    if (d->pUsernameEdit->isModified())
        d->pPiwigo->setUsername(d->pUsernameEdit->text());

    if (d->pPasswordEdit->isModified())
        d->pPiwigo->setPassword(d->pPasswordEdit->text());

    d->pPiwigo->save();
    accept();
}

} // namespace Digikam
