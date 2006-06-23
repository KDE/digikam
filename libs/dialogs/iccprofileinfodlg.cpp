/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-16
 * Description : a dialog to display icc profile informations.
 * 
 * Copyright 2006 by Gilles Caulier
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

#include <qgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qframe.h>
#include <qfile.h>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kapplication.h>
#include <ksqueezedtextlabel.h>

// Local includes.

#include "iccprofilewidget.h"
#include "iccprofileinfodlg.h"

namespace Digikam
{

ICCProfileInfoDlg::ICCProfileInfoDlg(QWidget* parent, const QString& profilePath,
                                     const QByteArray& profileData)
                 : KDialogBase(parent, 0, true, i18n("Color Profile Info"),
                               Help|Ok, Ok, true)
{
    setHelp("iccprofile.anchor", "digikam");
       
    QWidget *page     = new QWidget(this);
    QGridLayout* grid = new QGridLayout(page, 1, 1);

    QLabel *label9  = new QLabel(i18n("<p><b>File:</b>"), page);
    KSqueezedTextLabel *label10 = new KSqueezedTextLabel(profilePath, page);
    grid->addMultiCellWidget(label9, 0, 0, 0, 0);
    grid->addMultiCellWidget(label10, 0, 0, 1, 1);
            
    ICCProfileWidget *profileWidget = new ICCProfileWidget(page);
    grid->addMultiCellWidget(profileWidget, 1, 1, 0, 1);
    
    if (profileData.isEmpty())
        profileWidget->loadFromURL(KURL(profilePath));
    else
        profileWidget->loadFromData(profilePath, profileData); 
                                     
    setMainWidget(page);
}

ICCProfileInfoDlg::~ICCProfileInfoDlg()
{
}

}  // NameSpace Digikam

