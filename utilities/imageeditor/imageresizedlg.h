/* ============================================================
 * File  : imageresizedlg.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-05
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#ifndef IMAGERESIZEDLG_H
#define IMAGERESIZEDLG_H

#include <kdialogbase.h>

class KIntSpinBox;
class KDoubleSpinBox;
class QCheckBox;

class ImageResizeDlg : public KDialogBase
{
    Q_OBJECT

public:

    ImageResizeDlg(int* width, int* height);
    ~ImageResizeDlg();

private:

    KIntSpinBox     *m_wInput;
    KIntSpinBox     *m_hInput;
    KDoubleSpinBox  *m_wpInput;
    KDoubleSpinBox  *m_hpInput;
    QCheckBox       *m_constrainCheck;

    int      *m_width;
    int      *m_height;
    
private slots:

    void slotOk();
    void slotWChanged(int val);
    void slotHChanged(int val);
    void slotWPChanged(double val);
    void slotHPChanged(double val);
};

#endif /* IMAGERESIZEDLG_H */
