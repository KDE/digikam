/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-16
 * Description : a dialog to display ICC profile informations.
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

#ifndef ICCPROFILEINFODLG_H
#define ICCPROFILEINFODLG_H

// Qt includes.

#include <qstring.h>

// KDE includes.

#include <kdialogbase.h>

// Local includes.

#include "digikam_export.h"

class QWidget;

namespace Digikam
{

class ICCProfileInfoDlgPriv;

class DIGIKAM_EXPORT ICCProfileInfoDlg : public KDialogBase
{

public:

    ICCProfileInfoDlg(QWidget *parent, const QString& profilePath, const QByteArray& profileData=QByteArray());
    ~ICCProfileInfoDlg();

};

}  // Namespace Digikam

#endif /* ICCPROFILEINFODLG_H */
