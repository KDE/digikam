/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-16
 * Description : a dialog to display ICC profile information.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "iccprofileinfodlg.h"

// Qt includes

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "iccprofilewidget.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

ICCProfileInfoDlg::ICCProfileInfoDlg(QWidget* const parent, const QString& profilePath, const IccProfile& profile)
    : QDialog(parent)
{
    setModal(true);
    setWindowTitle(i18n("Color Profile Info - %1", profilePath));

    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok, this);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    ICCProfileWidget* const profileWidget = new ICCProfileWidget(this, 340, 256);

    if (profile.isNull())
    {
        profileWidget->loadFromURL(QUrl::fromLocalFile(profilePath));
    }
    else
    {
        profileWidget->loadProfile(profilePath, profile);
    }

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(profileWidget);
    vbx->addWidget(buttons);
    setLayout(vbx);

    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));
}

ICCProfileInfoDlg::~ICCProfileInfoDlg()
{
}

void ICCProfileInfoDlg::slotHelp()
{
    DXmlGuiWindow::openHandbook();
}

}  // namespace Digikam
