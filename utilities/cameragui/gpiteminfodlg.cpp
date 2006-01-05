/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-30
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qlabel.h>
#include <qvgroupbox.h>

// KDE includes.

#include <kglobal.h>
#include <klocale.h>
#include <kio/global.h>

// Local includes.

#include "gpiteminfo.h"
#include "gpiteminfodlg.h"

namespace Digikam
{

GPItemInfoDlg::GPItemInfoDlg(QWidget* parent, const GPItemInfo* itemInfo)
    : KDialogBase(parent, 0, true, itemInfo->name, Close, Close)
{
    QVGroupBox *page = new QVGroupBox(i18n("Camera File Properties"),
                                      this);
    setMainWidget(page);
    
    QLabel *label = new QLabel( page );

    QString cellBeg("<tr><td><nobr>");
    QString cellMid("</nobr></td><td>");
    QString cellEnd("</td></tr>");
    QString str;

    QString text("<table cellspacing=2 cellpadding=2>");

    text += cellBeg + i18n("Name:") + cellMid +
            itemInfo->name + cellEnd;
    text += cellBeg + i18n("Folder:") + cellMid +
            itemInfo->folder + cellEnd;

    if (itemInfo->readPermissions < 0)
        str = i18n("Unknown");
    else if (itemInfo->readPermissions == 0)
        str = i18n("No");
    else
        str = i18n("Yes");
    
    text += cellBeg + i18n("Read Permissions:") + cellMid +
            str + cellEnd;

    if (itemInfo->writePermissions < 0)
        str = i18n("Unknown");
    else if (itemInfo->writePermissions == 0)
        str = i18n("No");
    else
        str = i18n("Yes");
    
    text += cellBeg + i18n("Write Permissions:") + cellMid +
            str + cellEnd;
    
    QDateTime date;
    date.setTime_t(itemInfo->mtime);
    text += cellBeg + i18n("Date:") + cellMid +
            KGlobal::locale()->formatDateTime(date, true, true)
            + cellEnd;
           
    text += cellBeg + i18n("Size:") + cellMid;
    text += i18n("%1 (%2)")
            .arg(KIO::convertSize(itemInfo->size))
            .arg(KGlobal::locale()->formatNumber(itemInfo->size, 0))
            + cellEnd;

    text += cellBeg + i18n("Mime:") + cellMid +
            itemInfo->mime + cellEnd;

    text += cellBeg + i18n("Width:") + cellMid +
            ((itemInfo->width <= 0) ? i18n("Unknown")
             : QString::number(itemInfo->width)) + cellEnd;

    text += cellBeg + i18n("Height:") + cellMid +
            ((itemInfo->height <= 0) ? i18n("Unknown")
             : QString::number(itemInfo->height)) + cellEnd;

    if (itemInfo->downloaded < 0)
        str = i18n("Unknown");
    else if (itemInfo->downloaded == 0)
        str = i18n("No");
    else
        str = i18n("Yes");
    
    text += cellBeg + i18n("Already Downloaded:") + cellMid +
            str + cellEnd;

    text += "</table>";

    
    label->setText(text);
}

GPItemInfoDlg::~GPItemInfoDlg()
{
}

}  // namespace Digikam
