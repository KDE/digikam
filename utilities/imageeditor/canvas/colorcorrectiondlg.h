/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-05-15
 * Description : a dialog to see preview ICC color correction 
 *               before to apply color profile.
 * 
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

#ifndef COLORCORRECTIONDLG_H
#define COLORCORRECTIONDLG_H

// Qt includes.

#include <qstring.h>

// KDE includes.

#include <kdialogbase.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class IccTransform;
class DImg;

class DIGIKAM_EXPORT ColorCorrectionDlg : public KDialogBase
{
    Q_OBJECT

public:

    ColorCorrectionDlg(QWidget *parent, DImg *preview, 
                       IccTransform *iccTrans, const QString& file);
    ~ColorCorrectionDlg();
    
private slots:
    
    void slotCurrentProfInfo();
    void slotEmbeddedProfInfo();
    void slotApplyClicked();
    
private: 

    QWidget      *m_parent;

    IccTransform *m_iccTrans;
};

}  // Namespace Digikam

#endif /* COLORCORRECTIONDLG_H */
