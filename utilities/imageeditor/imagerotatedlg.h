/* ============================================================
 * File  : imagerotatedlg.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-16
 * Description : a dialog for free resizing image operations.
 * 
 * Copyright 2004 by Gilles Caulier
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

#ifndef IMAGEROTATEDLG_H
#define IMAGEROTATEDLG_H

// KDE includes.

#include <kdialogbase.h>

class KDoubleNumInput;

class ImageRotateDlg : public KDialogBase
{
    Q_OBJECT

public:

    ImageRotateDlg(QWidget* parent, double *angle);
    ~ImageRotateDlg();

private:

    KDoubleNumInput *m_angleInput;

    double          *m_angle;
    
private slots:

    void slotOk();
};

#endif /* IMAGEROTATEDLG_H */
