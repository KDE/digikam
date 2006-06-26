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

// KDE includes.

#include <kdebug.h>
#include <klocale.h>

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
    setCaption(profilePath);
       
    ICCProfileWidget *profileWidget = new ICCProfileWidget(this, 0, 340, 256);
    
    if (profileData.isEmpty())
        profileWidget->loadFromURL(KURL(profilePath));
    else
        profileWidget->loadFromData(profilePath, profileData); 
                                     
    setMainWidget(profileWidget);
}

ICCProfileInfoDlg::~ICCProfileInfoDlg()
{
}

}  // NameSpace Digikam

