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
#include <q3frame.h>
#include <q3textedit.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

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
    
    Q3Frame *p1 = addPage( i18n("Summary"), i18n("Camera Summary"), BarIcon("contents2", KIcon::SizeMedium) );
    Q3VBoxLayout *p1layout = new Q3VBoxLayout( p1, 0, 6 );

    Q3TextEdit *summaryView = new Q3TextEdit(summary, QString(), p1);
    summaryView->setWordWrap(Q3TextEdit::WidgetWidth);
    summaryView->setReadOnly(true);
    p1layout->addWidget(summaryView);

    // ----------------------------------------------------------

    Q3Frame *p2 = addPage( i18n("Manual"), i18n("Camera Manual"), BarIcon("contents", KIcon::SizeMedium) );
    Q3VBoxLayout *p2layout = new Q3VBoxLayout( p2, 0, 6 );

    Q3TextEdit *manualView = new Q3TextEdit(manual, QString(), p2);
    manualView->setWordWrap(Q3TextEdit::WidgetWidth);
    manualView->setReadOnly(true);
    p2layout->addWidget(manualView);

    // ----------------------------------------------------------

    Q3Frame *p3 = addPage( i18n("About"), i18n("About Driver"), BarIcon("camera", KIcon::SizeMedium) );
    Q3VBoxLayout *p3layout = new Q3VBoxLayout( p3, 0, 6 );
    
    Q3TextEdit *aboutView = new Q3TextEdit(about, QString(), p3);
    aboutView->setWordWrap(Q3TextEdit::WidgetWidth);
    aboutView->setReadOnly(true);
    p3layout->addWidget(aboutView);
}

CameraInfoDialog::~CameraInfoDialog()
{
}

}  // namespace Digikam
