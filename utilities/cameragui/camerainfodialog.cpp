/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-28
 * Description : a dialog to display camera information.
 * 
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qlayout.h>
#include <qframe.h>
#include <qtextedit.h>

// KDE includes.

#include <klocale.h>
#include <kiconloader.h>

// Local includes.

#include "camerainfodialog.h"

namespace Digikam
{

CameraInfoDialog::CameraInfoDialog(QWidget *parent, const QString& summary, const QString& manual,
                                   const QString& about)
                : KDialogBase(IconList, i18n("Camera Information"), Help|Ok, Ok, parent, 0, true, true)
{
    setHelp("digitalstillcamera.anchor", "digikam");
    resize(500, 400);

    // ----------------------------------------------------------
    
    QFrame *p1 = addPage( i18n("Summary"), i18n("Camera Summary"), BarIcon("contents2", KIcon::SizeMedium) );
    QVBoxLayout *p1layout = new QVBoxLayout( p1, 0, 6 );

    QTextEdit *summaryView = new QTextEdit(summary, QString(), p1);
    summaryView->setWordWrap(QTextEdit::WidgetWidth);
    summaryView->setReadOnly(true);
    p1layout->addWidget(summaryView);

    // ----------------------------------------------------------

    QFrame *p2 = addPage( i18n("Manual"), i18n("Camera Manual"), BarIcon("contents", KIcon::SizeMedium) );
    QVBoxLayout *p2layout = new QVBoxLayout( p2, 0, 6 );

    QTextEdit *manualView = new QTextEdit(manual, QString(), p2);
    manualView->setWordWrap(QTextEdit::WidgetWidth);
    manualView->setReadOnly(true);
    p2layout->addWidget(manualView);

    // ----------------------------------------------------------

    QFrame *p3 = addPage( i18n("About"), i18n("About Driver"), BarIcon("camera", KIcon::SizeMedium) );
    QVBoxLayout *p3layout = new QVBoxLayout( p3, 0, 6 );
    
    QTextEdit *aboutView = new QTextEdit(about, QString(), p3);
    aboutView->setWordWrap(QTextEdit::WidgetWidth);
    aboutView->setReadOnly(true);
    p3layout->addWidget(aboutView);
}

CameraInfoDialog::~CameraInfoDialog()
{
}

}  // namespace Digikam
