/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-06
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

#ifndef IMAGEEFFECT_REDEYE_H
#define IMAGEEFFECT_REDEYE_H

#include <kdialogbase.h>

class QRadioButton;

class ImageEffect_RedEye
{
public:

    static void removeRedEye(QWidget *parent);
};

class ImageEffect_RedEyeDlg : public KDialogBase
{
    Q_OBJECT
    
public:

    enum Result
    {
        Mild = 0,
        Aggressive = 1
    };

    ImageEffect_RedEyeDlg(QWidget* parent);
    Result result() const;

private slots:

    void slotClicked(int id);
    
private:

    int      m_selectedId;
};

#endif /* IMAGEEFFECT_REDEYE_H */
