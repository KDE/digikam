/* ============================================================
 * File  : gpfileiteminfodlg.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-19
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <qpixmap.h>
#include <qstring.h>
#include <qlayout.h>
#include <qlabel.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kseparator.h>

#include "gpfileiteminfo.h"
#include "gpfileiteminfodlg.h"


GPFileItemInfoDlg::GPFileItemInfoDlg(const GPFileItemInfo& info,
                                     QPixmap *pixmap )
    : KDialogBase(0, QString::null, true,
                  info.name,
                  Ok, Ok, true)
{
    QWidget *page = new QWidget( this );
    setMainWidget( page );

    QGridLayout *grid = new QGridLayout( page, 1, 1, 5, 5);

    // -----------------------------------------------------

    QLabel *thumbLabel = new QLabel( page );
    thumbLabel->setFrameShape(QFrame::Box);
    thumbLabel->setMargin(2);
    thumbLabel->setPaletteBackgroundColor(colorGroup().base());
    if (!pixmap) {
        if (info.mime.contains("image"))
            thumbLabel->setPixmap(DesktopIcon("image"));
        else if (info.mime.contains("audio"))
            thumbLabel->setPixmap(DesktopIcon("audio"));
        else if (info.mime.contains("video"))
            thumbLabel->setPixmap(DesktopIcon("video"));
        else
            thumbLabel->setPixmap(DesktopIcon("document"));
    }
    else
        thumbLabel->setPixmap(*pixmap);

    grid->addWidget( thumbLabel, 0, 0);
            
    // ----------------------------------------------------

    QLabel *nameLabel = new QLabel( page );
    nameLabel->setText(info.name);

    grid->addWidget( nameLabel, 0, 2);

    // ----------------------------------------------------

    KSeparator *sep = new KSeparator( KSeparator::HLine, page );
    grid->addMultiCellWidget( sep, 1, 1, 0, 2);

    // ----------------------------------------------------

    QLabel *l = 0;
    int currRow = 2;
    QString value;

    l = new QLabel(i18n("MimeType:"), page);
    grid->addWidget(l, currRow,  0);
    value = info.mime.isNull() ? i18n("Unknown") : info.mime;
    l = new QLabel(value, page);
    grid->addWidget(l, currRow++, 2);

    l = new QLabel(i18n("Date:"), page);
    grid->addWidget(l, currRow,  0);
    value = info.time.isNull() ? i18n("Unknown") : info.time;
    l = new QLabel(value, page);
    grid->addWidget(l, currRow++, 2);

    l = new QLabel(i18n("Size:"), page);
    grid->addWidget(l, currRow,  0);
    value = info.size <= 0 ? i18n("Unknown") : QString::number(info.size);
    value += " bytes";
    l = new QLabel(value, page);
    grid->addWidget(l, currRow++, 2);

    l = new QLabel(i18n("Width:"), page);
    grid->addWidget(l, currRow,  0);
    value = info.width <= 0 ? i18n("Unknown") : QString::number(info.width);
    l = new QLabel(value, page);
    grid->addWidget(l, currRow++, 2);


    l = new QLabel(i18n("Height:"), page);
    grid->addWidget(l, currRow,  0);
    value = info.height <= 0 ? i18n("Unknown") : QString::number(info.height);
    l = new QLabel(value, page);
    grid->addWidget(l, currRow++, 2);

    l = new QLabel(i18n("Read Permissions:"), page);
    grid->addWidget(l, currRow,  0);
    if (info.readPermissions == 0)
        value = i18n("No");
    else if (info.readPermissions == 1)
        value = i18n("Yes");
    else
        value = i18n("Unknown");
    l = new QLabel(value, page);
    grid->addWidget(l, currRow++, 2);

    l = new QLabel(i18n("Write Permissions:"), page);
    grid->addWidget(l, currRow,  0);
    if (info.writePermissions == 0)
        value = i18n("No");
    else if (info.writePermissions == 1)
        value = i18n("Yes");
    else
        value = i18n("Unknown");
    l = new QLabel(value, page);
    grid->addWidget(l, currRow++, 2);

    l = new QLabel(i18n("Downloaded:"), page);
    grid->addWidget(l, currRow,  0);
    if (info.downloaded == 0)
        value = i18n("No");
    else if (info.downloaded == 1)
        value = i18n("Yes");
    else
        value = i18n("Unknown");
    l = new QLabel(value, page);
    grid->addWidget(l, currRow++, 2);


}

GPFileItemInfoDlg::~GPFileItemInfoDlg()
{

}
