/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-28
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju
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

#include <qstring.h>
#include <qlayout.h>
#include <qframe.h>
#include <qtextedit.h>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "camerainfodialog.h"

namespace Digikam
{

CameraInfoDialog::CameraInfoDialog(const QString& summary,
                                   const QString& manual,
                                   const QString& about)
    : KDialogBase(KJanusWidget::Tabbed, i18n("Camera Information"),
                  Ok, Ok)
{
    resize(500, 400);

    // ----------------------------------------------------------
    
    QFrame *p1 = addPage( i18n("Summary") );
    QVBoxLayout *p1layout = new QVBoxLayout( p1, 0, 6 );

    QTextEdit *summaryView = new QTextEdit(summary, QString::null,
                                           p1);
    summaryView->setWordWrap(QTextEdit::WidgetWidth);
    summaryView->setReadOnly(true);
    p1layout->addWidget(summaryView);

    // ----------------------------------------------------------

    QFrame *p2 = addPage( i18n("Manual") );
    QVBoxLayout *p2layout = new QVBoxLayout( p2, 0, 6 );

    QTextEdit *manualView = new QTextEdit(manual, QString::null,
                                           p2);
    manualView->setWordWrap(QTextEdit::WidgetWidth);
    manualView->setReadOnly(true);
    p2layout->addWidget(manualView);

    // ----------------------------------------------------------

    QFrame *p3 = addPage( i18n("About") );
    QVBoxLayout *p3layout = new QVBoxLayout( p3, 0, 6 );
    

    QTextEdit *aboutView = new QTextEdit(about, QString::null,
                                         p3);
    aboutView->setWordWrap(QTextEdit::WidgetWidth);
    aboutView->setReadOnly(true);
    p3layout->addWidget(aboutView);

}

CameraInfoDialog::~CameraInfoDialog()
{
}

}  // namespace Digikam
