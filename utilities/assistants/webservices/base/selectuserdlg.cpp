/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-16-05
 * Description : a dialog to select user for export tools
 *
 * Copyright (C) 2015 by Shourya Singh Gupta <shouryasgupta at gmail dot com>
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

#include "selectuserdlg.h"

// Qt includes

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <kconfig.h>
#include <kconfiggroup.h>

namespace Digikam
{

SelectUserDlg::SelectUserDlg(QWidget* const parent, const QString& serviceName)
    : QDialog(parent)
{
    m_serviceName = serviceName;

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

    m_okButton = buttonBox->button(QDialogButtonBox::Ok);

    if (m_serviceName == QString::fromLatin1("23"))
    {
        setWindowIcon(QIcon::fromTheme(QString::fromLatin1("hq")));
    }
    else
    {
        setWindowIcon(QIcon::fromTheme(QString::fromLatin1("flickr")));
    }

    m_uname = QString();

    m_label = new QLabel(this);
    m_label->setText(i18n("Choose the %1 account to use for exporting images:", m_serviceName));

    m_userComboBox = new QComboBox(this);

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_label);
    mainLayout->addWidget(m_userComboBox);
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

SelectUserDlg::~SelectUserDlg()
{
    delete m_userComboBox;
    delete m_label;
}

void SelectUserDlg::reactivate()
{
    KConfig config;

    m_userComboBox->clear();

    foreach(const QString& group, config.groupList())
    {
        if (!(group.contains(m_serviceName)))
            continue;

        KConfigGroup grp = config.group(group);

        if (QString::compare(grp.readEntry(QString::fromLatin1("username")), QString(), Qt::CaseInsensitive) == 0)
            continue;

        m_userComboBox->addItem(grp.readEntry(QString::fromLatin1("username")));
    }

    m_okButton->setEnabled(m_userComboBox->count() > 0);

    exec();
}

void SelectUserDlg::slotOkClicked()
{
    m_uname = m_userComboBox->currentText();
}

void SelectUserDlg::slotNewAccountClicked()
{
    m_uname = QString();
}

QString SelectUserDlg::getUname() const
{
    return m_uname;
}

SelectUserDlg* SelectUserDlg::getDlg()
{
    return this;
}

} // namespace Digikam
