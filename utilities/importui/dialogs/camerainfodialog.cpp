/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-28
 * Description : a dialog to display camera information.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
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

#include "camerainfodialog.h"

// Qt includes

#include <QIcon>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTextEdit>
#include <QDialogButtonBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dxmlguiwindow.h"

namespace Digikam
{

CameraInfoDialog::CameraInfoDialog(QWidget* const parent,
                                   const QString& summary,
                                   const QString& manual,
                                   const QString& about)
    : QDialog(parent)
{
    setModal(true);
    setWindowTitle(i18nc("@title:window", "Device Information"));
    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok, this);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    resize(500, 400);

    QTabWidget* const tab = new QTabWidget(this);

    // ----------------------------------------------------------

    QTextEdit* const summaryView = new QTextEdit(summary);
    summaryView->setWordWrapMode(QTextOption::WordWrap);
    summaryView->setReadOnly(true);
    tab->insertTab(0, summaryView, QIcon::fromTheme(QLatin1String("dialog-information")), i18n("Device Summary"));

    // ----------------------------------------------------------

    QTextEdit* const manualView  = new QTextEdit(manual);
    manualView->setWordWrapMode(QTextOption::WordWrap);
    manualView->setReadOnly(true);
    tab->insertTab(1, manualView, QIcon::fromTheme(QLatin1String("help-contents")), i18n("Device Manual"));

    // ----------------------------------------------------------

    QTextEdit* const aboutView   = new QTextEdit(about);
    aboutView->setWordWrapMode(QTextOption::WordWrap);
    aboutView->setReadOnly(true);
    tab->insertTab(2, aboutView, QIcon::fromTheme(QLatin1String("camera-photo")), i18n("About Driver"));

    // ----------------------------------------------------------

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(tab);
    vbx->addWidget(buttons);
    setLayout(vbx);

    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));
}

CameraInfoDialog::~CameraInfoDialog()
{
}

void CameraInfoDialog::slotHelp()
{
    DXmlGuiWindow::openHandbook();
}

}  // namespace Digikam
