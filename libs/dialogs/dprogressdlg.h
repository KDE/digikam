/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-30-08
 * Description :a progress dialog for digiKam
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

#ifndef DPROGRESSDLG_H
#define DPROGRESSDLG_H

// KDE includes.

#include <kdialogbase.h>

// Local includes.

#include "digikam_export.h"

class KProgress;

namespace Digikam
{

class DProgressDlgPriv;

class DIGIKAM_EXPORT DProgressDlg : public KDialogBase
{
Q_OBJECT

 public:

    DProgressDlg(QWidget *parent=0, const QString &caption=QString::null);
    ~DProgressDlg();

    void setButtonText(const QString &text);
    void addedAction(const QPixmap& pix, const QString &text);
    void reset();
    void setTotalSteps(int total);
    void setValue(int value);
    void advance(int value);
    void setLabel(const QString &text);
    void setTitle(const QString &text);
    void setActionListVSBarVisible(bool visible);
    void showCancelButton(bool show);
    void setAllowCancel(bool allowCancel);
    bool wasCancelled() const;
    bool allowCancel() const;

    KProgress *progressBar() const;
 
 protected slots:

    void slotCancel();

 private:
 
    DProgressDlgPriv* d;
};

}  // NameSpace Digikam

#endif  // DPROGRESSDLG_H
