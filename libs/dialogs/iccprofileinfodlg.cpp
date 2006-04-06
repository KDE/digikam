/* ============================================================
 * File  : iccprofileinfodlg.cpp
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

#include <config.h>

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

// Lcms includes.

#include LCMS_HEADER
#if LCMS_VERSION < 114
#define cmsTakeCopyright(profile) "Unknown"
#endif // LCMS_VERSION < 114

// Local includes.

#include "cietonguewidget.h"
#include "iccprofileinfodlg.h"

namespace Digikam
{

ICCProfileInfoDlg::ICCProfileInfoDlg(QWidget* parent, const QString& profilePath)
                 : KDialogBase(Plain, i18n("Color Profile Info"), Help|Ok, Ok,
                               parent, 0, true, true)
{
    setHelp("iccprofile.anchor", "digikam");

    QString intent;

    cmsHPROFILE hProfile        = cmsOpenProfileFromFile(QFile::encodeName(profilePath), "r");
    QString profileName         = QString((cmsTakeProductName(hProfile)));
    QString profileDescription  = QString((cmsTakeProductDesc(hProfile)));
    QString profileManufacturer = QString(cmsTakeCopyright(hProfile));
    int profileIntent           = cmsTakeRenderingIntent(hProfile);
    
    // "Decode" profile rendering intent.
    
    switch (profileIntent)
    {
        case 0:
            intent = i18n("Perceptual");
            break;
        case 1:
            intent = i18n("Relative Colorimetric");
            break;
        case 2:
            intent = i18n("Saturation");
            break;
        case 3:
            intent = i18n("Absolute Colorimetric");
            break;
    }

    QGridLayout* grid = new QGridLayout( plainPage(), 6, 1);
        
    QLabel *label1 = new QLabel(i18n("<p><b>Name:</b>"), plainPage());
    KSqueezedTextLabel *label2 = new KSqueezedTextLabel(profileName, plainPage());
    grid->addMultiCellWidget(label1, 0, 0, 0, 0);
    grid->addMultiCellWidget(label2, 0, 0, 1, 1);
                                     
    QLabel *label3 = new QLabel(i18n("<p><b>Description:</b>"), plainPage());
    KSqueezedTextLabel *label4 = new KSqueezedTextLabel(profileDescription, plainPage());
    grid->addMultiCellWidget(label3, 1, 1, 0, 0);
    grid->addMultiCellWidget(label4, 1, 1, 1, 1);
                                     
    QLabel *label5 = new QLabel(i18n("<p><b>Copyright:</b>"), plainPage());
    KSqueezedTextLabel *label6 = new KSqueezedTextLabel(profileManufacturer, plainPage());
    grid->addMultiCellWidget(label5, 2, 2, 0, 0);
    grid->addMultiCellWidget(label6, 2, 2, 1, 1);

    QLabel *label7 = new QLabel(i18n("<p><b>Rendering Intent:</b>"), plainPage());
    KSqueezedTextLabel *label8 = new KSqueezedTextLabel(intent, plainPage());
    grid->addMultiCellWidget(label7, 3, 3, 0, 0);
    grid->addMultiCellWidget(label8, 3, 3, 1, 1);
                                     
    QLabel *label9  = new QLabel(i18n("<p><b>Path:</b>"), plainPage());
    KSqueezedTextLabel *label10 = new KSqueezedTextLabel(profilePath, plainPage());
    grid->addMultiCellWidget(label9, 4, 4, 0, 0);
    grid->addMultiCellWidget(label10, 4, 4, 1, 1);
                                     
    QLabel *label11  = new QLabel(i18n("<p><b>CIE diagram:</b>"), plainPage());
    CIETongueWidget *cieTongue = new CIETongueWidget(256, 256, plainPage());
    cieTongue->setProfileHandler(hProfile);
    grid->addMultiCellWidget(label11, 5, 5, 0, 1);
    grid->addMultiCellWidget(cieTongue, 6, 6, 1, 1);

    cmsCloseProfile(hProfile);
}

ICCProfileInfoDlg::~ICCProfileInfoDlg()
{
}

}  // NameSpace Digikam

