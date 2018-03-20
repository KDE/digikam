/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-16-05
 * Description : a dialog to select user for Web Service tools
 *
 * Copyright (C) 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * Copyright (C) 2016-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "wsselectuserdlg.h"

// Qt includes

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QIcon>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>
#include <kconfig.h>
#include <kconfiggroup.h>

namespace Digikam
{

class WSSelectUserDlg::Private
{
public:

    explicit Private()
    {
        userComboBox = 0;
        label        = 0;
        okButton     = 0;
    }

    QComboBox*   userComboBox;
    QLabel*      label;
    QPushButton* okButton;
    QString      userName;
    QString      serviceName;
};

WSSelectUserDlg::WSSelectUserDlg(QWidget* const parent, const QString& serviceName)
    : QDialog(parent),
      d(new Private)
{
    d->serviceName = serviceName;

    setWindowTitle(i18n("Account Selector"));
    setModal(true);

    QDialogButtonBox* const buttonBox   = new QDialogButtonBox();
    QPushButton* const buttonNewAccount = new QPushButton(buttonBox);
    buttonNewAccount->setText(i18n("Add another account"));
    buttonNewAccount->setIcon(QIcon::fromTheme(QString::fromLatin1("network-workgroup")));

    buttonBox->addButton(buttonNewAccount, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::Ok);
    buttonBox->addButton(QDialogButtonBox::Close);

    buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    d->okButton = buttonBox->button(QDialogButtonBox::Ok);

    if (d->serviceName == QString::fromLatin1("23"))
    {
        setWindowIcon(QIcon::fromTheme(QString::fromLatin1("hq")));
    }
    else
    {
        setWindowIcon(QIcon::fromTheme(QString::fromLatin1("flickr")));
    }

    d->userName     = QString();
    d->label        = new QLabel(this);
    d->label->setText(i18n("Choose the %1 account to use for exporting images:", d->serviceName));
    d->userComboBox = new QComboBox(this);

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(d->label);
    mainLayout->addWidget(d->userComboBox);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));

    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotOkClicked()));

    connect(buttonNewAccount, SIGNAL(clicked()),
            this, SLOT(slotNewAccountClicked()));
}

WSSelectUserDlg::~WSSelectUserDlg()
{
    delete d->userComboBox;
    delete d->label;
    delete d;
}

void WSSelectUserDlg::reactivate()
{
    KConfig config;

    d->userComboBox->clear();

    foreach(const QString& group, config.groupList())
    {
        if (!(group.contains(d->serviceName)))
            continue;

        KConfigGroup grp = config.group(group);

        if (QString::compare(grp.readEntry(QString::fromLatin1("username")), QString(), Qt::CaseInsensitive) == 0)
            continue;

        d->userComboBox->addItem(grp.readEntry(QString::fromLatin1("username")));
    }

    d->okButton->setEnabled(d->userComboBox->count() > 0);

    exec();
}

void WSSelectUserDlg::slotOkClicked()
{
    d->userName = d->userComboBox->currentText();
}

void WSSelectUserDlg::slotNewAccountClicked()
{
    d->userName = QString();
}

QString WSSelectUserDlg::getUserName() const
{
    return d->userName;
}

} // namespace Digikam
