/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-28
 * Description : a dialog to display camera information.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QTextEdit>

// KDE includes

#include <kicon.h>
#include <klocale.h>
#include <ktextedit.h>

namespace Digikam
{

CameraInfoDialog::CameraInfoDialog(QWidget* const parent, const QString& summary, const QString& manual,
                                   const QString& about)
    : KPageDialog(parent)
{
    setCaption(i18nc("@title:window", "Device Information"));
    setButtons(KDialog::Help | KDialog::Ok);
    setDefaultButton(KDialog::Ok);
    setHelp("digitalstillcamera.anchor", "digikam");
    setFaceType(KPageDialog::List);
    setModal(true);
    resize(500, 400);

    // ----------------------------------------------------------

    KTextEdit* const summaryView = new KTextEdit(summary);
    summaryView->setWordWrapMode(QTextOption::WordWrap);
    summaryView->setReadOnly(true);

    KPageWidgetItem* const p1    = addPage(summaryView, i18nc("Device information summary", "Summary"));
    p1->setHeader(i18n("Device Summary"));
    p1->setIcon(KIcon("dialog-information"));

    // ----------------------------------------------------------

    KTextEdit* const manualView  = new KTextEdit(manual);
    manualView->setWordWrapMode(QTextOption::WordWrap);
    manualView->setReadOnly(true);

    KPageWidgetItem* const p2    = addPage(manualView, i18nc("Manual of the device", "Manual"));
    p2->setHeader(i18n("Device Manual"));
    p2->setIcon(KIcon("help-contents"));

    // ----------------------------------------------------------

    KTextEdit* const aboutView   = new KTextEdit(about);
    aboutView->setWordWrapMode(QTextOption::WordWrap);
    aboutView->setReadOnly(true);

    KPageWidgetItem* const p3    = addPage(aboutView, i18nc("About device driver", "About"));
    p3->setHeader(i18n("About Driver"));
    p3->setIcon(KIcon("camera-photo"));
}

CameraInfoDialog::~CameraInfoDialog()
{
}

}  // namespace Digikam
